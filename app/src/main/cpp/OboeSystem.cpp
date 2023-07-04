#include <jni.h>
#include <map>
#include <string>
#include "include/OboePlayer.h"

using namespace std;
//
// Created by christina on 2023/7/4.
//

typedef struct {
    OboePlayer &player;
    jobject owner;
} pcontext;

static JavaVM *G_JVM;

static int32_t get_obj_hashcode(JNIEnv *, jobject);

static pcontext *find_player_by_obj(JNIEnv *, jobject);
static pcontext *find_player_by_hashcode(int32_t hashcode);

static map<int32_t, pcontext *> ps_map;

static void completed_handler(OboePlayer &player, int32_t usr) {
    pcontext *target = find_player_by_hashcode(usr);
    JNIEnv * env;
    jint state = G_JVM->AttachCurrentThread(&env, nullptr);
    if (state == JNI_OK) {
        jclass clazz = env->GetObjectClass(target->owner);
        if (clazz == nullptr) {
            goto end;
        }
        jmethodID methodId = env->GetMethodID(clazz, "onCompleted", "()V");
        if (methodId == nullptr) {
            goto end;
        }
        env->CallVoidMethod(target->owner, methodId);
    }
    return;
    end:
        G_JVM->DetachCurrentThread();
}

static void error_handler(oboe::AudioStream &stream, oboe::Result result, int32_t usr) {
    pcontext *target = find_player_by_hashcode(usr);
    JNIEnv *env;
    jint state = G_JVM->AttachCurrentThread(&env, nullptr);
    if (state == JNI_OK) {
        jclass clazz = env->GetObjectClass(target->owner);
        if (clazz == nullptr) {
            goto end;
        }
        jmethodID methodId = env->GetMethodID(clazz, "onCompleted", "()V");
        if (methodId == nullptr) {
            goto end;
        }
        env->CallVoidMethod(target->owner, methodId, result);
    }
    return;
    end:
    G_JVM->DetachCurrentThread();
}

jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved){
    G_JVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    LOG_D("JVM loaded");
    return JNI_VERSION_1_4;
}



extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1setAudioPath(JNIEnv *env, jobject thiz,
                                                            jstring jpath) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr != nullptr) {
        const char *path = env->GetStringUTFChars(jpath, JNI_FALSE);
        ptr->player.set_data_source(path);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1prepare(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        return ptr->player.prepare();
    }
    return JNI_ERR;
}
extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1start(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        ptr->player.start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1pause(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        ptr->player.pause();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1stop(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        ptr->player.stop();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1release(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        ps_map.erase(get_obj_hashcode(env, thiz));
        ptr->player.release();
        env->DeleteGlobalRef(ptr->owner);
    }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1isPlaying(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        return ptr->player.is_playing();
    }

    return JNI_FALSE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1isLooped(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        return ptr->player.is_loop();
    }

    return JNI_FALSE;
}
extern "C"
JNIEXPORT jint JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1newPlayer(JNIEnv *env, jobject thiz, jint device_id,
                                                         jint sample_rate, jint channel_count,
                                                         jint fmt) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (!ptr) {
        int32_t usr = get_obj_hashcode(env, thiz);
        pcontext *ctx = new pcontext{
                .player = *new OboePlayer(device_id, sample_rate, channel_count, fmt, usr),
                .owner = env ->NewGlobalRef(thiz)
        };
        ctx -> player.set_on_completed_listener(completed_handler);
        ctx -> player.set_on_error_listener(error_handler);
        ps_map.emplace(usr, ctx);
    } else return JNI_FALSE;
    return JNI_TRUE;
}
extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1setLoop(JNIEnv *env, jobject thiz,
                                                       jboolean is_loop) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        ptr->player.set_loop(is_loop);
    }
}


JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
    G_JVM = nullptr;
}

static int32_t get_obj_hashcode(JNIEnv *env, jobject obj) {
    jclass clazz = env->FindClass("java/lang/Object");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    jmethodID methodId = env->GetMethodID(clazz, "hashCode", "()I");
    if (methodId == nullptr) {
        return JNI_ERR;
    }
    return env->CallIntMethod(obj, methodId);
}

static pcontext *find_player_by_hashcode(int32_t hashcode) {
    unsigned int has = ps_map.count(hashcode);
    if (has) {
        return ps_map.at(hashcode);
    } else {
        return nullptr;
    }
}

static pcontext *find_player_by_obj(JNIEnv *env, jobject obj) {
    int32_t hashcode = get_obj_hashcode(env, obj);
    if (hashcode == JNI_ERR) return nullptr;
    unsigned int has = ps_map.count(hashcode);
    if (has) {
        return ps_map.at(hashcode);
    } else {
        return nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1reset(JNIEnv *env, jobject thiz) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        ptr->player.reset();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_cn_touchair_oboeplayer_OboePlayer_native_1seek_1to(JNIEnv *env, jobject thiz, jfloat jrela) {
    pcontext *ptr = find_player_by_obj(env, thiz);
    if (ptr) {
        ptr->player.seek_to(jrela);
    }
}