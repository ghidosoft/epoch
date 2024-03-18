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

#include "AudioPlayer.hpp"

#include "Audio.hpp"

namespace epoch::frontend
{
    AudioPlayer::AudioPlayer(int sampleRate, int channels) : m_audioContext{std::make_unique<AudioContext>()}
    {
        m_stream = std::make_unique<AudioStream>(sampleRate, channels);
        m_stream->start();
    }

    AudioPlayer::~AudioPlayer() = default;

    void AudioPlayer::push(const std::span<float> sample) const { m_stream->push(sample); }

    unsigned long AudioPlayer::neededSamples() const
    {
        return std::max(0L, 4096L - static_cast<long>(m_stream->ahead()));
    }
}  // namespace epoch::frontend
