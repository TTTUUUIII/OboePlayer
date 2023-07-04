//
// Created by christina on 2023/7/3.
//
#include <string>
#include "include/OboePlayer.h"

using namespace std;

void OboePlayer::set_data_source(string path) {
    LOG_D("OboePlayer::set_data_source %s", path.c_str())
    this->__source_path = path;
}

void OboePlayer::set_loop(bool is_loop) {
    LOG_D("OboePlayer::set_loop %d", is_loop)
    __is_loop = is_loop;
}

void OboePlayer::stop() {
    LOG_D("OboePlayer::stop");
    if (_is_playing) {
    _is_playing = false;
        __stream->requestStop();
        wait_for();
    }
}

int OboePlayer::prepare() {
    LOG_D("OboePlayer::prepare");
    if (__source_path.empty()) return -ERR_NO_SET_SOURCE;
    __stream_in.open(__source_path, ios::in | ios::binary);
    if (!__stream_in.is_open()) return -ERR_FILE_NOT_FOUNT;
    if (__builder.openStream(__stream) != oboe::Result::OK) return -ERR_OPEN_STREAM;
    __stream_in.seekg(ios::end);
    __source_len = __stream_in.tellg();
    __stream_in.seekg(ios::beg);
    return NO_ERR;
}

void OboePlayer::start() {
    LOG_D("OboePlayer::start");
    if (_is_playing) return;
    _is_playing = true;
    __stream->requestStart();
    wait_for();
}

void OboePlayer::pause() {
    LOG_D("OboePlayer::pause");
    _is_playing = false;
    __stream->requestPause();
    __stream->requestFlush();
    wait_for();
}

void OboePlayer::reset() {
    LOG_D("OboePlayer::reset");
    stop();
    if (__stream_in.is_open()) __stream_in.close();
}

void OboePlayer::release() {
    LOG_D("OboePlayer::release");
    stop();
    delete this;
}

OboePlayer::OboePlayer(uint32_t device_id, uint32_t sample_rate, uint32_t channel_count,
                       uint32_t fmt, int32_t usr) {
    __builder.setDirection(oboe::Direction::Output)
    -> setDeviceId(device_id)
    -> setPerformanceMode(oboe::PerformanceMode::LowLatency)
    -> setSharingMode(oboe::SharingMode::Exclusive)
    -> setFormat(of_format_from_int(fmt))
    -> setSampleRate(sample_rate)
    -> setChannelCount(of_channel_count_from_int(channel_count))
    -> setDataCallback(this)
    -> setErrorCallback(this);
    __usr = usr;
}

OboePlayer::~OboePlayer() {
    if (__stream_in.is_open()) __stream_in.close();
    __stream->close();
}

oboe::ChannelCount OboePlayer::of_channel_count_from_int(uint32_t channel_count) {
    oboe::ChannelCount cct = oboe::ChannelCount::Unspecified;
    switch (channel_count) {
        case CHANNEL_COUNT_MONO:
            cct = oboe::ChannelCount::Mono;
            __channel_count = 1;
            break;
        case CHANNEL_COUNT_STEREO:
            cct = oboe::ChannelCount::Stereo;
            __channel_count = 2;
            break;
        default:
            /* ignored */
            ;
    }
    return cct;
}

oboe::AudioFormat OboePlayer::of_format_from_int(uint32_t format) {
    oboe::AudioFormat fmt = oboe::AudioFormat::I16;
    switch (format) {
        case FORMAT_PCM_FLOAT:
            fmt = oboe::AudioFormat::Float;
            __sample_len = 4;
            break;
        case FORMAT_PCM_I16:
            fmt = oboe::AudioFormat::I16;
            __sample_len = 2;
            break;
        case FORMAT_PCM_I24:
            fmt = oboe::AudioFormat::I24;
            __sample_len = 3;
            break;
        case FORMAT_PCM_I32:
            fmt = oboe::AudioFormat::I32;
            __sample_len = 4;
            break;
        default:
            /* ignored */
            ;
    }
    return fmt;
}

oboe::DataCallbackResult
OboePlayer::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) {
    if (__stream_in.is_open() && _is_playing) {
        __stream_in.read(static_cast<char *>(audioData),
                         numFrames * __channel_count * __sample_len);
        if (__stream_in.eof()) {
            __stream_in.close();
            if (__is_loop) {
                __stream_in.open(__source_path, ios::in | ios::binary);
            } else {
                if (__completed_handler) __completed_handler(*this, __usr);
                _is_playing = false;
            }
        }
    } else {
        _is_playing = false;
    }
    return _is_playing ? oboe::DataCallbackResult::Continue : oboe::DataCallbackResult::Stop;
}

bool OboePlayer::is_playing() {
    return _is_playing;
}

bool OboePlayer::is_loop() {
    return __is_loop;
}

bool OboePlayer::onError(oboe::AudioStream *stream, oboe::Result result) {
    if (__error_handler) __error_handler(*stream, result, __usr);
    return true;
}

void OboePlayer::set_on_completed_listener(void (*c_proc)(OboePlayer &, int32_t)) {
    __completed_handler = c_proc;
}

void OboePlayer::set_on_error_listener(void (*e_proc)(oboe::AudioStream &, oboe::Result, int32_t)) {
    __error_handler = e_proc;
}

void OboePlayer::seek_to(float rela) {
    if (__stream_in.is_open()) __stream_in.seekg(__source_len * rela, ios::beg);
}

void OboePlayer::wait_for() {
    oboe::StreamState next_state = oboe::StreamState::Uninitialized;
    __stream->waitForStateChange(__stream->getState(), &next_state, TIME_OUT_NANO);
}
