//
// Created by aidoo on 2024/10/31.
//
#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <unistd.h>
#include <pthread.h>
#include "rr_log.h"
#include "GLRender.h"

static JavaVM *gVm;
static EGLSurface gSurface;
static EGLDisplay gDisplay;
static EGLContext gContext;

static int currentWidth, currentHeight;

void *runLoop(void *) {
    // running on bind thread.
    LOGD("start thread.");
    JNIEnv *env = nullptr;
    gVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    gVm->AttachCurrentThread(&env, nullptr);

    eglMakeCurrent(gDisplay, gSurface, gSurface, gContext);
    GLRender render;
    GLuint glProgram = render.createGlProgram();
    GLuint glPosition = glGetAttribLocation(glProgram, "a_Position");

    int count = 1;
    while (count++ < 1000) {
        glViewport(0, 0, currentWidth, currentHeight);
        if (count % 2 == 0) {
            glClearColor(1.0, 0, 0, 1.0);
        } else {
            glClearColor(0, 1.0, 1.0, 1);
        }
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glUseProgram(glProgram);

        eglSwapBuffers(gDisplay, gSurface);
        glUseProgram(0);
        sleep(1);
    }

    gVm->DetachCurrentThread();
    return nullptr;
}


extern "C" void testWithSurface(JNIEnv *env, jobject surface) {
    env->GetJavaVM(&gVm);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGE("egl have not got display.");
        return;
    }
    if (eglInitialize(display, 0, 0) != EGL_TRUE) {
        LOGE("egl Initialize failed.%d", eglGetError());
        return;
    }
    gDisplay = display;
    const EGLint atrribs[] = {
            EGL_BUFFER_SIZE, 32,
            EGL_ALPHA_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    EGLConfig eglConfig;
    EGLint numOfEglConfig;
    if (eglChooseConfig(display, atrribs, &eglConfig, 1, &numOfEglConfig) != EGL_TRUE) {
        LOGE("egl choose config failed.%d,", eglGetError());
        return;
    }
    EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    gContext = eglCreateContext(display, eglConfig, nullptr, attributes);
    if (!gContext) {
        LOGE("eglCreateContext failed.");
        return;
    }
    ANativeWindow_acquire(window);
    ANativeWindow_setBuffersGeometry(window, 0, 0, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);
    gSurface = eglCreateWindowSurface(display, eglConfig, window, 0);
    if (!gSurface) {
        LOGE("eglCreateWindowSurface failed.");
        return;
    }
    LOGE("renderer start run.");
    pthread_t thread;
    pthread_create(&thread, nullptr, runLoop, nullptr);
}

extern "C" void setSurfaceSize(int width, int height) {
    currentWidth = width;
    currentHeight = height;
}