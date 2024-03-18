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

#ifndef SRC_FRONTEND_AUDIOPLAYER_HPP_
#define SRC_FRONTEND_AUDIOPLAYER_HPP_

#include <memory>
#include <span>

namespace epoch::frontend
{
    class AudioContext;
    class AudioStream;

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
        void push(std::span<float> sample) const;
        [[nodiscard]] unsigned long neededSamples() const;

    private:
        std::unique_ptr<AudioContext> m_audioContext{};
        std::unique_ptr<AudioStream> m_stream{};
    };
}  // namespace epoch::frontend

#endif
