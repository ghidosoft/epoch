/* This file is part of Epoch, Copyright (C) 2024 Andrea Ghidini.
 *
 * Epoch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epoch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Epoch.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Audio.hpp"

#include <portaudio.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#define PA_CHECK(a)                                                          \
    do                                                                       \
    {                                                                        \
        const auto PA_CHECK_paResult = a;                                    \
        if (PA_CHECK_paResult != paNoError)                                  \
        {                                                                    \
            std::stringstream ss{};                                          \
            ss << "PortAudio error: " << Pa_GetErrorText(PA_CHECK_paResult); \
            const auto s = ss.str();                                         \
            std::cerr << s << std::endl;                                     \
            throw std::runtime_error(s);                                     \
        }                                                                    \
    } while (0)

namespace epoch::frontend
{
    AudioStream::AudioStream(const int sampleRate)
    {
        const auto device = Pa_GetDefaultOutputDevice();
        const auto info = Pa_GetDeviceInfo(device);
        assert(info);
        const PaStreamParameters outputParameters{
            .device = device,
            .channelCount = 2,
            .sampleFormat = paFloat32,
            .suggestedLatency = info->defaultLowOutputLatency,
        };
        const auto hostApi = Pa_GetHostApiInfo(info->hostApi);
        std::cout << "Audio output: " << info->name << " (" << hostApi->name << ")" << std::endl;
        PA_CHECK(Pa_OpenStream(&m_handle, nullptr, &outputParameters, sampleRate, paFramesPerBufferUnspecified,
                               paClipOff, &s_callback, this));
    }

    AudioStream::~AudioStream()
    {
        if (m_handle)
        {
            Pa_CloseStream(m_handle);
        }
    }

    void AudioStream::start() const { PA_CHECK(Pa_StartStream(m_handle)); }

    void AudioStream::stop() const { PA_CHECK(Pa_StopStream(m_handle)); }

    int AudioStream::callback(float* outputBuffer, const unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags)
    {
        m_buffer.read(outputBuffer, framesPerBuffer * 2);
        return paContinue;
    }

    int AudioStream::s_callback(const void* inputBuffer, void* outputBuffer, const unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo, const PaStreamCallbackFlags statusFlags,
                                void* userData)
    {
        return static_cast<AudioStream*>(userData)->callback(static_cast<float*>(outputBuffer), framesPerBuffer,
                                                             timeInfo, statusFlags);
    }

    AudioContext::AudioContext() { PA_CHECK(Pa_Initialize()); }

    AudioContext::~AudioContext() { Pa_Terminate(); }
}  // namespace epoch::frontend
