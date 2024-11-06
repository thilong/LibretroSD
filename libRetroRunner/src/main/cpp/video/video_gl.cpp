//
// Created by aidoo on 2024/11/5.
//

#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl32.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "video_gl.h"
#include "../rr_log.h"
#include "../app.h"
#include "../environment.h"


#define GLVIDEOLOGD(...) LOGD("[VIDEO] " __VA_ARGS__)
#define GLVIDEOLOGW(...) LOGW("[VIDEO] " __VA_ARGS__)
#define GLVIDEOLOGE(...) LOGE("[VIDEO] " __VA_ARGS__)
#define GLVIDEOLOGI(...) LOGI("[VIDEO] " __VA_ARGS__)
#define ENABLE_GL_DEBUG 0
namespace libRetroRunner {

    extern "C" JavaVM *gVm;

    static void MessageCallback(
            GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar *message,
            const void *userParam
    ) {
        if (type == GL_DEBUG_TYPE_ERROR) {
            LOGE("GL CALLBACK: \"** GL ERROR **\" type = 0x%x, severity = 0x%x, message = %s\n",
                 type,
                 severity,
                 message);
        }
    }

    __unused
    static bool initializeGLESLogCallbackIfNeeded() {
        auto debugCallback = (void (*)(void *, void *)) eglGetProcAddress("glDebugMessageCallback");
        if (debugCallback) {
            glEnable(GL_DEBUG_OUTPUT);
            debugCallback((void *) MessageCallback, nullptr);
            LOGW("GL debug callback enabled.");
        }
        return true;
    }
}
namespace libRetroRunner {
    GLVideoContext::GLVideoContext() : VideoContext() {
        is_ready = false;
        eglContext = nullptr;
        eglDisplay = nullptr;
        eglSurface = nullptr;
        frame_count = 0;
        current_height = 0;
        current_height = 0;
    }

    GLVideoContext::~GLVideoContext() {
        Destroy();
    }

    void GLVideoContext::SetSurface(JNIEnv *env, jobject surface) {
        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            GLVIDEOLOGD("egl have not got display.");
            return;
        }
        if (eglInitialize(display, 0, 0) != EGL_TRUE) {
            GLVIDEOLOGD("egl Initialize failed.%d", eglGetError());
            return;
        }
        eglDisplay = display;
        const EGLint atrrs[] = {
                EGL_ALPHA_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_DEPTH_SIZE, 16,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_NONE
        };
        //opengl es2: EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGLConfig eglConfig;
        EGLint numOfEglConfig;
        if (eglChooseConfig(display, atrrs, &eglConfig, 1, &numOfEglConfig) != EGL_TRUE) {
            GLVIDEOLOGE("egl choose config failed.%d,", eglGetError());
            return;
        }

        EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        eglContext = eglCreateContext(display, eglConfig, nullptr, attributes);
        if (!eglContext) {
            GLVIDEOLOGE("eglCreateContext failed.");
            return;
        }

        EGLint format;
        if (!eglGetConfigAttrib(display, eglConfig, EGL_NATIVE_VISUAL_ID, &format)) {
            GLVIDEOLOGE("egl get config attrib failed.");
            return;
        }

        ANativeWindow_acquire(window);
        ANativeWindow_setBuffersGeometry(window, 0, 0, format);
        eglSurface = eglCreateWindowSurface(display, eglConfig, window, 0);
        if (!eglSurface) {
            GLVIDEOLOGE("eglCreateWindowSurface failed.");
            return;
        }

#if(ENABLE_GL_DEBUG)
        initializeGLESLogCallbackIfNeeded();
#endif

    }

    void GLVideoContext::OnFrameArrive(const void *data, unsigned int width, unsigned int height, size_t pitch) {
        GL_CHECK2("GLVideoContext::OnFrameArrive", "frame: %llu", frame_count);

        if (is_ready && data != nullptr) {
            if (data != RETRO_HW_FRAME_BUFFER_VALID) {

                auto appContext = AppContext::Instance();

                //如果没有使用硬件加速，则需要创建一个软件纹理
                if (gameTexture == nullptr || gameTexture->GetWidth() != width || gameTexture->GetHeight() != height) {
                    gameTexture = std::make_unique<SoftwareTextureBuffer>();
                    gameTexture->Create(width, height);
                    GL_CHECK2("gameTexture->Create", "frame: %llu", frame_count);
                }

                //把核心渲染的数据写入到gameTexture上
                gameTexture->WriteTextureData(data, width, height, appContext->GetEnvironment()->pixelFormat);
                GL_CHECK2("gameTexture->WriteTextureData", "frame: %llu", frame_count);

                /*
                if (gameRender == nullptr) {
                    gameRender = std::make_unique<SoftwareRender>();
                    GL_CHECK2("gameRender->Create", "frame: %llu", frame_count);
                }

                //渲染(Test)
                gameRender->Render(current_width, current_height, gameTexture->GetTexture());
                GL_CHECK2("gameRender->Render", "frame: %llu", frame_count);
                */

                gamePass->DrawTexture(gameTexture->GetTexture());

            }
            DrawFrame();
        }

        frame_count++;
    }

    void GLVideoContext::DrawFrame() {

        if (current_width == 0 || current_height == 0) return;
        do {
            /*
            glViewport(0, 0, current_width, current_height);
            if (frame_count % 60 == 0) {
                glClearColor(1.0, 0, 0, 1.0);
            } else {
                glClearColor(0, 1.0, 1.0, 1);
            }
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
             */
            gamePass->DrawTexture(0, current_width, current_height);
            eglSwapBuffers(eglDisplay, eglSurface);

        } while ((false));

    }

    void GLVideoContext::Destroy() {
        if (gameTexture) {
            gameTexture->Destroy();
            gameTexture = nullptr;
        }
        if (gamePass != nullptr) {
            gamePass->Destroy();
            gamePass = nullptr;
        }
        shaderPasses.erase(shaderPasses.begin(), shaderPasses.end());

        if (eglDisplay != EGL_NO_DISPLAY) {
            eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (eglSurface != EGL_NO_SURFACE) {
                eglDestroySurface(eglDisplay, eglSurface);
                eglSurface = EGL_NO_SURFACE;
            }
            if (eglContext != EGL_NO_CONTEXT) {
                eglDestroyContext(eglDisplay, eglContext);
                eglContext = EGL_NO_CONTEXT;
            }
            eglTerminate(eglDisplay);
            eglDisplay = EGL_NO_DISPLAY;
        }
    }

    void GLVideoContext::SetSurfaceSize(unsigned int width, unsigned int height) {
        current_width = width;
        current_height = height;
    }

    unsigned int GLVideoContext::GetCurrentFramebuffer() {
        GLVIDEOLOGW("GLVideoContext::GetCurrentFramebuffer");
        if (gamePass != nullptr) {
            return gamePass->GetFrameBuffer();
        }
        return 0;
    }

    void GLVideoContext::Init() {
        if (!eglContextMakeCurrent()) {
            LOGE("video init failed, this may cause render error.");
            return;
        }
        createFrameBufferForCore();
        is_ready = true;
    }

    void GLVideoContext::Prepare() {
        auto env = AppContext::Instance()->GetEnvironment();
        if (env->gameGeometryUpdated && is_ready) {
            createFrameBufferForCore(1);
        }
        GL_CHECK2("GLVideoContext::Prepare", "frame: %llu", frame_count);
    }

    void GLVideoContext::createFrameBufferForCore(int useCase) {
        auto env = AppContext::Instance()->GetEnvironment();
        gamePass = std::make_unique<GLShaderPass>(nullptr, nullptr);
        gamePass->CreateFrameBuffer(env->gameGeometryWidth, env->gameGeometryHeight, true, env->renderUseDepth, env->renderUseStencil);
        env->gameGeometryUpdated = false;
    }

    bool GLVideoContext::eglContextMakeCurrent() {
        if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) != EGL_TRUE) {
            LOGE("eglMakeCurrent failed.");
            return false;
        }
        return true;
    }
}
