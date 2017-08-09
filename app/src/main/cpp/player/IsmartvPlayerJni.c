//
// Created by huibin on 09/08/2017.
//

#include "IsmartvPlayerJni.h"

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    ALOGD("JNI_OnLoad");

    return JNI_VERSION_1_4;
}