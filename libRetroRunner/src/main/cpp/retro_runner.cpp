#include <jni.h>
#include "app.h"
#include "utils/jnistring.h"
#include "app.h"
#include "environment.h"
#include "rr_log.h"
#include "video.h"
#include "rr_types.h"

#define JNILOGD(...) LOGD("[JNI] " __VA_ARGS__)
#define JNILOGW(...) LOGW("[JNI] " __VA_ARGS__)
#define JNILOGE(...) LOGE("[JNI] " __VA_ARGS__)
#define JNILOGI(...) LOGI("[JNI] " __VA_ARGS__)

namespace libRetroRunner {
    extern "C" JavaVM *gVm = nullptr;


}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_initEnv(JNIEnv *env, jclass clazz) {
    if (libRetroRunner::gVm != nullptr) return;
    env->GetJavaVM(&(libRetroRunner::gVm));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_create(JNIEnv *env, jclass clazz, jstring rom_path, jstring core_path, jstring system_path, jstring save_path) {
    libRetroRunner::AppContext *app = libRetroRunner::AppContext::NewInstance();
    JString rom(env, rom_path);
    JString core(env, core_path);
    JString system(env, system_path);
    JString save(env, save_path);
    app->SetFiles(rom.stdString(), core.stdString(), system.stdString(), save.stdString());
    JNILOGD("new app context created.");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setSurface(JNIEnv *env, jclass clazz, jobject surface) {
    auto appContext = libRetroRunner::AppContext::Instance();
    if (appContext == nullptr) return;
    if (surface == nullptr) {
        appContext->SetVideoRenderTarget(nullptr, 0);
    } else {
        void *args[2] = {env, surface};
        appContext->SetVideoRenderTarget(args, 2);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setSurfaceSize(JNIEnv *env, jclass clazz, jint width, jint height) {
    auto appContext = libRetroRunner::AppContext::Instance();
    if (appContext == nullptr) return;
    appContext->SetVideoRenderSize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setVariable(JNIEnv *env, jclass clazz, jstring key, jstring value) {
    auto appContext = libRetroRunner::AppContext::Instance();
    if (appContext == nullptr) return;
    JString keyVal(env, key);
    JString valueVal(env, key);
    appContext->SetVariable(keyVal.stdString(), valueVal.stdString());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_start(JNIEnv *env, jclass clazz) {
    libRetroRunner::AppContext *appContext = libRetroRunner::AppContext::Instance();
    if (appContext != nullptr) appContext->Start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_pause(JNIEnv *env, jclass clazz) {
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_resume(JNIEnv *env, jclass clazz) {
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_stop(JNIEnv *env, jclass clazz) {
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_reset(JNIEnv *env, jclass clazz) {
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_updateButtonState(JNIEnv *env, jclass clazz, jint player, jint key, jboolean down) {
    auto appContext = libRetroRunner::AppContext::Instance();
    if (appContext == nullptr) {
        LOGE("[JNI] app context is null, can't set button state");
        return;
    }
    auto input = appContext->GetInput();
    if (input == nullptr) {
        LOGE("[JNI] input context is null, can't set button state");
        return;
    }
    input->UpdateButton(player, key, down);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_UpdateAxisState(JNIEnv *env, jclass clazz, jint player, jint axis, jfloat value) {
    // TODO: implement UpdateAxisState()
}