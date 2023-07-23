/* This file is part of Epoch, Copyright (C) 2023 Andrea Ghidini.
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

#ifndef SRC_FRONTEND_AUDIO_H_
#define SRC_FRONTEND_AUDIO_H_

#include <memory>

#include "CircularBuffer.h"

#ifndef PaStream
typedef void PaStream;
#endif
#ifndef PaStreamCallbackFlags
typedef unsigned long PaStreamCallbackFlags;
#endif
struct PaStreamCallbackTimeInfo;

namespace epoch::frontend
{
    class AudioStream final
    {
    public:
        AudioStream(int sampleRate, int channels);
        ~AudioStream();

    public:
        AudioStream(const AudioStream& other) = delete;
        AudioStream(AudioStream&& other) noexcept = delete;
        AudioStream& operator=(const AudioStream& other) = delete;
        AudioStream& operator=(AudioStream&& other) noexcept = delete;

    public:
        void start() const;
        void stop() const;

        void push(const float sample) { m_buffer.write(sample); }
        [[nodiscard]] unsigned long ahead() const { return m_buffer.ahead(); }

        static constexpr unsigned long BufferSize = 1 << 15;

    private:
        PaStream* m_handle{};

        CircularBuffer<float, BufferSize> m_buffer{};

        int callback(float* outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags);

    private:
        static int s_callback(const void* inputBuffer, void* outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void* userData);
    };

    class AudioContext final
    {
    public:
        AudioContext();
        ~AudioContext();

    public:
        AudioContext(const AudioContext& other) = delete;
        AudioContext(AudioContext&& other) noexcept = delete;
        AudioContext& operator=(const AudioContext& other) = delete;
        AudioContext& operator=(AudioContext&& other) noexcept = delete;
    };

    class AudioPlayer final
    {
    public:
        AudioPlayer(int sampleRate, int channels);
        ~AudioPlayer();

    public:
        AudioPlayer(const AudioPlayer& other) = delete;
        AudioPlayer(AudioPlayer&& other) noexcept = delete;
        AudioPlayer& operator=(const AudioPlayer& other) = delete;
        AudioPlayer& operator=(AudioPlayer&& other) noexcept = delete;

    public:
        void push(const float sample) { m_stream->push(sample); }
        [[nodiscard]] unsigned long neededSamples() const { return std::max(0l, 1024 - static_cast<long>(m_stream->ahead())); }

    private:
        AudioContext m_audioContext{};
        std::unique_ptr<AudioStream> m_stream{};
    };
}

#endif
