//
// Created by huibin on 09/08/2017.
//

#ifndef NATIVE_CODEC_ISMARTVBASE_H
#define NATIVE_CODEC_ISMARTVBASE_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <jni.h>
#include <android/log.h>

#define LOG_TAG "Ismartv-Player"
#define VLOGV(...)  __android_log_vprint(ANDROID_LOG_VERBOSE,   LOG_TAG, __VA_ARGS__)
#define VLOGD(...)  __android_log_vprint(ANDROID_LOG_DEBUG,     LOG_TAG, __VA_ARGS__)
#define VLOGI(...)  __android_log_vprint(ANDROID_LOG_INFO,      LOG_TAG, __VA_ARGS__)
#define VLOGW(...)  __android_log_vprint(ANDROID_LOG_WARN,      LOG_TAG, __VA_ARGS__)
#define VLOGE(...)  __android_log_vprint(ANDROID_LOG_ERROR,     LOG_TAG, __VA_ARGS__)

#define ALOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,    LOG_TAG, __VA_ARGS__)
#define ALOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,      LOG_TAG, __VA_ARGS__)
#define ALOGI(...)  __android_log_print(ANDROID_LOG_INFO,       LOG_TAG, __VA_ARGS__)
#define ALOGW(...)  __android_log_print(ANDROID_LOG_WARN,       LOG_TAG, __VA_ARGS__)
#define ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR,      LOG_TAG, __VA_ARGS__)


bool ExceptionCheck__catchAll(JNIEnv *env);

#endif //NATIVE_CODEC_ISMARTVBASE_H
