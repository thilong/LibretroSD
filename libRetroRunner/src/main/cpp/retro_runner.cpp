#include <jni.h>
#include "app.h"
#include "utils/jnistring.h"
#include "app.h"
#include "environment.h"
#include "rr_log.h"
#include "video.h"
#include "rr_types.h"

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
    LOGD("[JNI] create app context");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setSurface(JNIEnv *env, jclass clazz, jobject surface) {
    if (surface == nullptr) {

    } else {
        auto appContext = libRetroRunner::AppContext::Instance();
        if (appContext == nullptr) return;
        libRetroRunner::VideoContext *video = appContext->GetVideo();
        if (video == nullptr) return;
        video->SetSurface(env, surface);
        appContext->AddCommand(AppCommands::kInitVideo);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setSurfaceSize(JNIEnv *env, jclass clazz, jint width, jint height) {
    auto appContext = libRetroRunner::AppContext::Instance();
    if (appContext == nullptr) return;
    libRetroRunner::VideoContext *video = appContext->GetVideo();
    if (video == nullptr) return;
    video->SetSurfaceSize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setVariable(JNIEnv *env, jclass clazz, jstring key, jstring value) {
    auto appContext = libRetroRunner::AppContext::Instance();
    if (appContext == nullptr) return;
    libRetroRunner::Environment *environment = appContext->GetEnvironment();
    if (environment == nullptr) return;
    JString keyVal(env, key);
    JString valueVal(env, key);
    environment->UpdateVariable(keyVal.stdString(), valueVal.stdString());
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