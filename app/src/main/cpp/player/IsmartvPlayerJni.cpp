//
// Created by huibin on 09/08/2017.
//


#include <assert.h>
#include <pthread.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <android/native_window_jni.h>
#include <unistd.h>

#include "IsmartvPlayerJni.h"
#include "IsmartvPlayerInternal.h"
#include "../looper.h"
#include "Mp4Extractor.h"

#define  JNI_CLASS_ISMARTVPLAYER "cn/ismartv/player/IsmartvPlayer"

static JavaVM *g_jvm;

typedef struct player_fields_t {
    pthread_mutex_t mutex;
    jclass clazz;
} player_fields_t;
static player_fields_t g_clazz;

typedef struct {
    int fd;
    const char *source;
    ANativeWindow *window;
    AMediaExtractor *ex;
    AMediaCodec *codec;
    int64_t renderstart;
    bool sawInputEOS;
    bool sawOutputEOS;
    bool isPlaying;
    bool renderonce;
    int64_t positionMs;
} workerdata;

workerdata data = {-1, NULL, NULL, NULL, 0, false, false, false, false};


enum {
    kMsgCodecBuffer,
    kMsgPause,
    kMsgResume,
    kMsgPauseAck,
    kMsgDecodeDone,
    kMsgSeek,
    kMsgRewind
};

int64_t system_nano_time() {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000000LL + now.tv_nsec;
}

class PlayerLooper : public looper {
    virtual void handle(int what, void *obj);
};

static PlayerLooper *playerLooper = NULL;

static IsmartvMediaPlayer *jni_get_media_player(JNIEnv *env, jobject thiz) {
//    pthread_mutex_lock(&g_clazz.mutex);
//
//    IsmartvMediaPlayer *mp = (IsmartvMediaPlayer *) (intptr_t) cn_ismartv_player_IsmartvPlayer__mIsmartvMediaPlayer__get__catchAll(
//            env, thiz);
//    if (mp) {
//        ismartv_mp_inc_ref(mp);
//    }
//
//    pthread_mutex_unlock(&g_clazz.mutex);
//    return mp;
    return NULL;
}

//static IjkMediaPlayer *jni_set_media_player(JNIEnv* env, jobject thiz, IjkMediaPlayer *mp)
//{
//    pthread_mutex_lock(&g_clazz.mutex);
//
//    IjkMediaPlayer *old = (IjkMediaPlayer*) (intptr_t) J4AC_IjkMediaPlayer__mNativeMediaPlayer__get__catchAll(env, thiz);
//    if (mp) {
//        ijkmp_inc_ref(mp);
//    }
//    J4AC_IjkMediaPlayer__mNativeMediaPlayer__set__catchAll(env, thiz, (intptr_t) mp);
//
//    pthread_mutex_unlock(&g_clazz.mutex);
//
//    // NOTE: ijkmp_dec_ref may block thread
//    if (old != NULL ) {
//        ijkmp_dec_ref_p(&old);
//    }
//
//    return old;
//}

static void IsmartvPlayer_init(JNIEnv *env) {
}

static void IsmartvPlayer_setup(JNIEnv *env, jobject thiz, jobject weak_this) {

}

static void IsmartvPlayer_setSurface(JNIEnv *env, jobject thiz, jobject jsurface) {
    ALOGD("IsmartvPlayer_setSurface");

    if (data.window) {
        ANativeWindow_release(data.window);
        data.window = NULL;
    }

    data.window = ANativeWindow_fromSurface(env, jsurface);
    ALOGD("@@@ setsurface %p", data.window);

}

static jboolean IsmartvPlayer_prepare(JNIEnv *env, jobject thiz, jstring jsource) {
    ALOGD("IsmartvPlayer_prepare");
    workerdata *d = &data;

    d->source = env->GetStringUTFChars(jsource, 0);

    AMediaExtractor *extractor = AMediaExtractor_new();

    media_status_t mediaStatus = AMediaExtractor_setDataSource(extractor, d->source);

//    free(d.data)

    if (mediaStatus != AMEDIA_OK) {
        ALOGE("setDataSource error: %d", mediaStatus);
        return JNI_FALSE;
    }

    int numberTracks = AMediaExtractor_getTrackCount(extractor);
    ALOGD("input has %d tracks", numberTracks);

    AMediaCodec *mediaCodec = NULL;

    for (int i = 0; i < numberTracks; ++i) {
        AMediaFormat *mediaFormat = AMediaExtractor_getTrackFormat(extractor, i);
        const char *format = AMediaFormat_toString(mediaFormat);
        ALOGV("track %d format: %s", i, format);

        const char *mime;
        if (!AMediaFormat_getString(mediaFormat, AMEDIAFORMAT_KEY_MIME, &mime)) {
            ALOGE("no mime type");
            return JNI_FALSE;
        } else if (!strncmp(mime, "video/", 6)) {
            AMediaExtractor_selectTrack(extractor, i);
            mediaCodec = AMediaCodec_createDecoderByType(mime);
            AMediaCodec_configure(mediaCodec, mediaFormat, d->window, NULL, 0);

            d->ex = extractor;
            d->codec = mediaCodec;
            d->renderstart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            d->isPlaying = false;
            d->renderonce = true;
            AMediaCodec_start(mediaCodec);
        }
        AMediaFormat_delete(mediaFormat);
    }

    playerLooper = new PlayerLooper();
    playerLooper->post(kMsgSeek, &data);

    return JNI_TRUE;
}

static void IsmartvPlayer_setPlayWhenReady(JNIEnv *env, jobject thiz, jboolean playWhenReady) {
    ALOGD("IsmartvPlayer_setPlayingStreamingMediaPlayer");
    if (playerLooper) {
        if (playWhenReady) {
            playerLooper->post(kMsgResume, &data);
        } else {
            playerLooper->post(kMsgPause, &data);
        }
    }
}

static void IsmartvPlayer_stop(JNIEnv *env, jobject thiz) {
    ALOGD("IsmartvPlayer_stop");
    if (playerLooper) {
        playerLooper->post(kMsgDecodeDone, &data, true);
        playerLooper->quit();
        delete playerLooper;
        playerLooper = NULL;
    }

    if (data.window) {
        ANativeWindow_release(data.window);
        data.window = NULL;
    }
}

static void IsmartvPlayer_rewind(JNIEnv *env, jobject thiz) {
    ALOGD("IsmartvPlayer_rewind");
    if (playerLooper) {
        playerLooper->post(kMsgRewind, &data);
    }
}

static jlong IsmartvPlayer_getDuration(JNIEnv *env, jobject instance) {
    ALOGD("IsmartvPlayer_getDuration");
    int64_t duration = Mp4Extractor_getDuration(data.source);
    return (jlong)duration;
}

static long IsmartvPlayer_getCurrentPosition(JNIEnv *env, jobject instance) {
    ALOGD("IsmartvPlayer_getCurrentPosition");
}

static void IsmartvPlayer_seekTo(JNIEnv *env, jobject instance, jlong positionMs) {
    ALOGD("IsmartvPlayer_seekTo");
    data.positionMs = (size_t)positionMs;
    if (playerLooper){
        playerLooper->post(kMsgSeek, &data);
    }
}

void doCodecWork(workerdata *d) {
    ssize_t bufidx = -1;

    if (!d->sawInputEOS) {
        bufidx = AMediaCodec_dequeueInputBuffer(d->codec, 2000);
        ALOGV("input buffer %zd", bufidx);
        if (bufidx >= 0) {
            size_t bufsize;
            auto buf = AMediaCodec_getInputBuffer(d->codec, bufidx, &bufsize);
            auto sampleSize = AMediaExtractor_readSampleData(d->ex, buf, bufsize);
            if (sampleSize < 0) {
                sampleSize = 0;
                d->sawInputEOS = true;
                ALOGV("EOS");
            }

            auto presentationTimeUs = AMediaExtractor_getSampleTime(d->ex);

            AMediaCodec_queueInputBuffer(d->codec, bufidx, 0, sampleSize, presentationTimeUs,
                                         d->sawInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
            AMediaExtractor_advance(d->ex);
        }
    }

    if (!d->sawOutputEOS) {
        AMediaCodecBufferInfo info;
        auto status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
        if (status >= 0) {
            if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                ALOGV("output EOS");
                d->sawOutputEOS = true;
            }

            int64_t presentationNano = info.presentationTimeUs * 1000;

            if (d->renderstart < 0) {
                d->renderstart = system_nano_time() - presentationNano;
            }

            int64_t delay = (d->renderstart + presentationNano) - system_nano_time();

            if (delay > 0) {
                usleep(delay / 1000);
            }

            AMediaCodec_releaseOutputBuffer(d->codec, status, info.size != 0);

            if (d->renderonce) {
                d->renderonce = false;
                return;
            }
        } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
            ALOGV("output buffers changed");
        } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            auto format = AMediaCodec_getOutputFormat(d->codec);
            ALOGV("format changed to: %s", AMediaFormat_toString(format));
            AMediaFormat_delete(format);
        } else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            ALOGV("no output buffer right now");
        } else {
            ALOGV("unexpected info code: %zd", status);
        }
    }

    if (!d->sawInputEOS || !d->sawOutputEOS) {
        playerLooper->post(kMsgCodecBuffer, d);
    }
}

void PlayerLooper::handle(int what, void *obj) {
    switch (what) {
        case kMsgCodecBuffer: {
            doCodecWork((workerdata *) obj);
        }
            break;
        case kMsgDecodeDone: {
            workerdata *d = (workerdata *) obj;
            AMediaCodec_stop(d->codec);
            AMediaCodec_delete(d->codec);
            AMediaExtractor_delete(d->ex);
            d->sawInputEOS = true;
            d->sawOutputEOS = true;
        }
            break;
        case kMsgSeek: {
            workerdata *d = (workerdata *) obj;
            AMediaExtractor_seekTo(d->ex, d->positionMs * 1000, AMEDIAEXTRACTOR_SEEK_NEXT_SYNC);
            AMediaCodec_flush(d->codec);
            d->renderstart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            if (!d->isPlaying) {
                d->renderonce = true;
                post(kMsgCodecBuffer, d);
            }
            ALOGV("seeked");
        }
            break;
        case kMsgPause: {
            workerdata *d = (workerdata *) obj;
            if (d->isPlaying) {
                d->isPlaying = false;
                post(kMsgPauseAck, NULL, true);
            }
        }
            break;
        case kMsgResume: {
            workerdata *d = (workerdata *) obj;
            if (!d->isPlaying) {
                d->renderstart = -1;
                d->isPlaying = true;
                post(kMsgCodecBuffer, d);
            }
        }
            break;
        case kMsgRewind: {
            workerdata *d = (workerdata *) obj;
            AMediaExtractor_seekTo(d->ex, 0, AMEDIAEXTRACTOR_SEEK_NEXT_SYNC);
            AMediaCodec_flush(d->codec);
            d->renderstart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            if (!d->isPlaying) {
                d->renderonce = true;
                post(kMsgCodecBuffer, d);
            }
        }
            break;

    }
}


static JNINativeMethod g_methods[] = {
        {
                "_prepare",
                "(Ljava/lang/String;)Z",
                (void *) IsmartvPlayer_prepare
        },

        {       "_setSurface",
                "(Landroid/view/Surface;)V",
                (void *) IsmartvPlayer_setSurface
        },

        {
                "_setPlayWhenReady",
                "(Z)V",
                (void *) IsmartvPlayer_setPlayWhenReady
        },

        {
                "_stop",
                "()V",
                (void *) IsmartvPlayer_stop
        },

        {
                "_rewind",
                "()V",
                (void *) IsmartvPlayer_rewind
        },

        {
                "_getDuration",
                "()J",
                (void *) IsmartvPlayer_getDuration
        },

        {
                "_getCurrentPosition",
                "()J",
                (void *) IsmartvPlayer_getCurrentPosition

        },

        {
                "_seekTo",
                "(J)V",
                (void *) IsmartvPlayer_seekTo
        }
};

//
//static int ismartv_find_java_class(JNIEnv *env, jobject jclazz, char *class_sign) {
//    do {
//        jclass clazz = env->FindClass(class_sign);
//        if (ExceptionCheck__catchAll(env) || !(clazz)) {
//            ALOGE("FindClass failed: %s", class_sign);
//            return -1;
//
//        }
//        jclazz = env->NewGlobalRef(clazz);
//        if (ExceptionCheck__catchAll(env) || !(jclazz)) {
//
//            ALOGE("FindClass::NewGlobalRef failed: %s", class_sign);
//            env->DeleteLocalRef(clazz);
//            return -1;
//
//        }
//        env->DeleteLocalRef(clazz);
//
//    } while (0);
//}
//

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    ALOGD("JNI_OnLoad");

    JNIEnv *env = NULL;

    g_jvm = vm;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    assert(env != NULL);

    pthread_mutex_init(&g_clazz.mutex, NULL);

    // FindClass returns LocalReference
//    ISMARTV_FIND_JAVA_CLASS(env, g_clazz.clazz, JNI_CLASS_ISMARTVPLAYER);
//    do {
    g_clazz.clazz = env->FindClass(JNI_CLASS_ISMARTVPLAYER);


    if (ExceptionCheck__catchAll(env) || !(g_clazz.clazz)) {
        ALOGE("FindClass failed: %s", JNI_CLASS_ISMARTVPLAYER);
        return -1;
    }

//        jobject  object = env->NewGlobalRef()
//    }while (0);

    env->RegisterNatives(g_clazz.clazz, g_methods, NELEM(g_methods));

    return JNI_VERSION_1_4;
}


JNIEXPORT void JNI_OnUnload(JavaVM *jvm, void *reserved) {
    pthread_mutex_destroy(&g_clazz.mutex);
}




