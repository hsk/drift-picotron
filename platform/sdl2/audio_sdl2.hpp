// audio_sdl2.hpp : SDL2 audio output for core/audio.hpp's mixer.
//
// Owns the SDL audio device and callback only; all synthesis happens in
// db::audio::render(), which has no SDL dependency (same function an
// ESP32-P4 I2S callback would call).
#pragma once
#include "../../core/audio.hpp"
#include <SDL.h>
#include <cstdio>
#include <vector>

namespace platform {

class Sdl2Audio {
public:
    bool init(int sample_rate = 44100) {
        SDL_AudioSpec want{}, have{};
        want.freq = sample_rate;
        want.format = AUDIO_F32SYS;
        want.channels = 1;
        want.samples = 1024;
        want.callback = &Sdl2Audio::callback;
        want.userdata = this;
        dev_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
        if (dev_ == 0) {
            std::fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
            return false;
        }
        sample_rate_ = have.freq;
        SDL_PauseAudioDevice(dev_, 0);
        return true;
    }

    void shutdown() {
        if (dev_) SDL_CloseAudioDevice(dev_);
        dev_ = 0;
    }

private:
    static void callback(void* userdata, Uint8* stream, int len) {
        auto* self = static_cast<Sdl2Audio*>(userdata);
        float* out = reinterpret_cast<float*>(stream);
        int n_frames = len / (int)sizeof(float);
        db::audio::render(out, n_frames, self->sample_rate_);
    }

    SDL_AudioDeviceID dev_ = 0;
    int sample_rate_ = 44100;
};

} // namespace platform
