//
// Created by huibin on 09/08/2017.
//

#ifndef NATIVE_CODEC_ISMARTVPLAYER_H
#define NATIVE_CODEC_ISMARTVPLAYER_H

#include <jni.h>
#include "IsmartvBase.h"

typedef struct cn_ismartv_player_IsmartvPlayer {
    jclass id;

    jfieldID filed_mIsmartvPlayer;

} cn_ismartv_player_IsmartvPlayer;

static cn_ismartv_player_IsmartvPlayer class_cn_ismartv_player_IsmartvPlayer;


jlong cn_ismartv_player_IsmartvPlayer__mIsmartvPlayer__get(JNIEnv *env, jobject thiz) {
    return (*env)->GetLongField(env, thiz,
                                class_cn_ismartv_player_IsmartvPlayer.filed_mIsmartvPlayer);
}

jlong cn_ismartv_player_IsmartvPlayer__mIsmartvPlayer__get__catchAll(JNIEnv *env, jobject thiz) {
    jlong ret_value = cn_ismartv_player_IsmartvPlayer__mIsmartvPlayer__get(env, thiz);
    if (ExceptionCheck__catchAll(env)) {
        return 0;
    }
    return ret_value;
}


#endif //NATIVE_CODEC_ISMARTVPLAYER_H
