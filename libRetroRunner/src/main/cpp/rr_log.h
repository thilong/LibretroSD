//
// Created by aidoo on 2024/10/31.
//

#ifndef LIBRETROSD_LOG_H
#define LIBRETROSD_LOG_H

#include <android/log.h>

namespace libRetroRunner {

#ifdef DEBUG
    /*是否输出核心的日志中debug级别的信息*/
#define CORE_DEUBG_LOG 1
#endif
#define LOG_TAG "RetroRunner"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

}
#endif //LIBRETROSD_LOG_H
