//
// Created by aidoo on 2024/11/5.
//

#include <unistd.h>

#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "video_gl.h"
#include "../rr_log.h"
#include "../app.h"
#include "../environment.h"


#define LOGD_GLVIDEO(...) LOGD("[VIDEO] " __VA_ARGS__)
#define LOGW_GLVIDEO(...) LOGW("[VIDEO] " __VA_ARGS__)
#define LOGE_GLVIDEO(...) LOGE("[VIDEO] " __VA_ARGS__)
#define LOGI_GLVIDEO(...) LOGI("[VIDEO] " __VA_ARGS__)

#define ENABLE_GL_DEBUG 1

namespace libRetroRunner {

    extern "C" JavaVM *gVm;

#ifdef HAVE_GLES3

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
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            debugCallback((void *) MessageCallback, nullptr);
            LOGW("GL debug callback enabled.");
        } else {
            LOGW("cant find symbol of glDebugMessageCallback, log wont show up.");
        }
        return true;
    }

#endif
}
namespace libRetroRunner {
    static std::unique_ptr<SoftwareRender> softwareRender = nullptr;

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

    void GLVideoContext::Init() {
        if (!eglContextMakeCurrent()) {
            LOGE_GLVIDEO("video init failed, this may cause render error.");
            return;
        }

#if defined(HAVE_GLES3) && (ENABLE_GL_DEBUG)
        initializeGLESLogCallbackIfNeeded();
#endif
        makeBackBuffer();
        auto appContext = AppContext::Instance();
        auto env = appContext->GetEnvironment();
        if (env->renderUseHWAcceleration) {

            env->renderContextReset();
        }
        is_ready = true;
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

    void GLVideoContext::SetSurface(void *envObj, void *surfaceObj) {
        JNIEnv *env = (JNIEnv *) envObj;
        jobject surface = (jobject) surfaceObj;

        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            LOGE_GLVIDEO("egl have not got display.");
            return;
        }
        if (eglInitialize(display, 0, 0) != EGL_TRUE) {
            LOGE_GLVIDEO("egl Initialize failed.%d", eglGetError());
            return;
        }
        eglDisplay = display;
        //2:EGL_OPENGL_ES2_BIT   3:EGL_OPENGL_ES3_BIT_KHR
        const EGLint atrrs[] = {
                EGL_ALPHA_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_DEPTH_SIZE, 16,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_NONE
        };
        //opengl es2: EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGLConfig eglConfig;
        EGLint numOfEglConfig;
        if (eglChooseConfig(display, atrrs, &eglConfig, 1, &numOfEglConfig) != EGL_TRUE) {
            LOGE_GLVIDEO("egl choose config failed.%d,", eglGetError());
            return;
        }

        EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        eglContext = eglCreateContext(display, eglConfig, nullptr, attributes);
        if (!eglContext) {
            LOGE_GLVIDEO("eglCreateContext failed.");
            return;
        }

        EGLint format;
        if (!eglGetConfigAttrib(display, eglConfig, EGL_NATIVE_VISUAL_ID, &format)) {
            LOGE_GLVIDEO("egl get config attrib failed.");
            return;
        }

        ANativeWindow_acquire(window);
        ANativeWindow_setBuffersGeometry(window, 0, 0, format);
        EGLint window_attribs[] = {
                EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
                EGL_NONE,
        };
        eglSurface = eglCreateWindowSurface(display, eglConfig, window, window_attribs);
        if (!eglSurface) {
            LOGE_GLVIDEO("eglCreateWindowSurface failed.");
            return;
        }


    }

    void GLVideoContext::OnFrameArrive(const void *data, unsigned int width, unsigned int height, size_t pitch) {
        if (is_ready && data != nullptr) {
            if (data != RETRO_HW_FRAME_BUFFER_VALID) {
                //LOGD_GLVIDEO("frame: %llu, width: %d, height: %d, pitch: %d", frame_count, width, height, pitch);
                auto appContext = AppContext::Instance();
                //如果没有使用硬件加速，则需要创建一个软件纹理
                if (gameTexture == nullptr || gameTexture->GetWidth() != width || gameTexture->GetHeight() != height) {
                    gameTexture = std::make_unique<SoftwareTextureBuffer>();
                    gameTexture->Create(width, height);
                    GL_CHECK2("gameTexture->Create", "frame: %llu", frame_count);
                }
                //把核心渲染的数据写入到gameTexture上
                gameTexture->WriteTextureData(data, width, height, appContext->GetEnvironment()->pixelFormat);

                /*
                if (softwareRender == nullptr) {
                    softwareRender = std::make_unique<SoftwareRender>();
                    GL_CHECK2("gameRender->Create", "frame: %llu", frame_count);
                }

                //渲染(Test)
                softwareRender->Render(current_width, current_height, gameTexture->GetTexture());
                GL_CHECK2("gameRender->Render", "frame: %llu", frame_count);
                eglSwapBuffers(eglDisplay, eglSurface);
                */
                gamePass->DrawTexture(gameTexture->GetTexture());
            }
            DrawFrame();
        }
        frame_count++;
    }

    void GLVideoContext::DrawFrame() {

        if (current_width == 0 || current_height == 0) {
            LOGW_GLVIDEO("draw frame failed: current_width or current_height is 0.");
            return;
        }
        do {

            //LOGD_GLVIDEO("draw frame: %d x %d, thread: %d", current_width, current_height, gettid());
            gamePass->DrawToScreen(current_width, current_height);
            eglSwapBuffers(eglDisplay, eglSurface);

            if (false) {  //只在硬件渲染下使用，用于每一帧的清理
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glDisable(GL_DITHER);

                glDisable(GL_STENCIL_TEST);
                glDisable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBlendEquation(GL_FUNC_ADD);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            }

        } while ((false));

    }

    void GLVideoContext::SetSurfaceSize(unsigned int width, unsigned int height) {
        current_width = width;
        current_height = height;
    }

    unsigned int GLVideoContext::GetCurrentFramebuffer() {
        if (gamePass != nullptr) {
            return gamePass->GetFrameBuffer();
        }
        return 0;
    }

    bool GLVideoContext::eglContextMakeCurrent() {
        if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) != EGL_TRUE) {
            LOGE("eglMakeCurrent failed.");
            return false;
        }
        return true;
    }

    void GLVideoContext::OnGameGeometryChanged() {

    }

    void GLVideoContext::Prepare() {

    }

    void GLVideoContext::makeBackBuffer() {
        auto env = AppContext::Instance()->GetEnvironment();
        if (gamePass == nullptr) {
            gamePass = std::make_unique<GLShaderPass>(nullptr, nullptr);
            gamePass->SetPixelFormat(env->pixelFormat);
            gamePass->CreateFrameBuffer(env->gameGeometryMaxWidth, env->gameGeometryMaxWidth, false, env->renderUseDepth, env->renderUseStencil);
        }
    }

    void GLVideoContext::Reinit() {

    }
}
