#include <jni.h>
#include "app_context.h"
#include "utils/jnistring.h"

namespace libRetroRunner {
    JavaVM *gVm = nullptr;


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
    libRetroRunner::AppContext *app = (new libRetroRunner::AppContext());
    JString rom(env, rom_path);
    JString core(env, core_path);
    JString system(env, system_path);
    JString save(env, save_path);
    app->SetFiles(rom.stdString(), core.stdString(), system.stdString(), save.stdString());
}


extern "C" void testWithSurface(JNIEnv *env, jobject surface);
extern "C" void setSurfaceSize(int width, int height);

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setSurface(JNIEnv *env, jclass clazz, jobject surface) {
    testWithSurface(env, surface);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setSurfaceSize(JNIEnv *env, jclass clazz, jint width, jint height) {
    setSurfaceSize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_setVariables(JNIEnv *env, jclass clazz, jobject variables) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_aidoo_retrorunner_NativeRunner_start(JNIEnv *env, jclass clazz) {

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