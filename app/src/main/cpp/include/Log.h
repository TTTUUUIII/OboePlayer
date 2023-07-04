//
// Created by CHRISTNA on 2023/6/29.
//

#ifndef AAUDIOPLAYER_LOG_H
#define AAUDIOPLAYER_LOG_H
#include <android/log.h>
#define TAG "AAudioPlayer"
#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__);
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);
#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__);

#endif //AAUDIOPLAYER_LOG_H
