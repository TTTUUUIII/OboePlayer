//
// Created by christina on 2023/7/3.
//

#ifndef OBOEPLAYER_OBOEPLAYER_H
#define OBOEPLAYER_OBOEPLAYER_H
#include <string>
#include <fstream>
#include "oboe/Oboe.h"
#include "Log.h"
using namespace std;

#define TIME_OUT_NANO 8000L
#define NO_ERR 0
#define ERR_NO_SET_SOURCE 1
#define ERR_FILE_NOT_FOUNT 2
#define ERR_OPEN_STREAM 3

#define CHANNEL_COUNT_MONO 1
#define CHANNEL_COUNT_STEREO 2

#define FORMAT_PCM_FLOAT 0
#define FORMAT_PCM_I16 2
#define FORMAT_PCM_I24 3
#define FORMAT_PCM_I32 4

class OboePlayer : public oboe::AudioStreamDataCallback , oboe::AudioStreamErrorCallback{
public:
    oboe::DataCallbackResult
    onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override;
    bool onError(oboe::AudioStream *stream, oboe::Result result) override;
    void set_data_source(string path);
    void set_loop(bool is_loop);
    void seek_to(float rela);
    int prepare();
    void start();
    void pause();
    void stop();
    void reset();
    void release();
    bool is_loop();
    bool is_playing();
    OboePlayer(uint32_t device_id, uint32_t sample_rate, uint32_t channel_count, uint32_t format, int32_t usr);
    ~OboePlayer();
    void set_on_completed_listener(void (*proc)(OboePlayer&, int32_t));
    void set_on_error_listener(void (*proc)(oboe::AudioStream&, oboe::Result, int32_t));

private:
    int32_t __usr;
    string __source_path;
    long __source_len;
    fstream __stream_in;
    bool __is_loop;
    bool _is_playing;
    uint32_t __channel_count;
    uint32_t __sample_len;
    oboe::AudioStreamBuilder __builder;
    shared_ptr<oboe::AudioStream> __stream;
    oboe::ChannelCount of_channel_count_from_int(uint32_t channel_count);
    oboe::AudioFormat of_format_from_int(uint32_t format);
    void (*__completed_handler)(OboePlayer &player, int32_t usr) = nullptr;
    void (*__error_handler)( oboe::AudioStream &stream, oboe::Result result, int32_t tag) = nullptr;
    void wait_for();
};


#endif //OBOEPLAYER_OBOEPLAYER_H
