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

#ifndef SRC_SOUND_AY8910DEVICE_HPP_
#define SRC_SOUND_AY8910DEVICE_HPP_

#include "SoundDevice.hpp"

#include <epoch/core.hpp>

#include <array>
#include <cstdint>

namespace epoch::sound
{
    class AY8910Device : public SoundDevice
    {
    public:
        using StereoMix = std::array<SoundSample, 3>;

    public:
        explicit AY8910Device();

    public:
        void reset() override;
        virtual void clock();

        void address(uint8_t value);
        void data(uint8_t data);
        [[nodiscard]] uint8_t data() const;

        [[nodiscard]] SoundSample output() const;

        void stereoMix(StereoMix mix);

        static constexpr StereoMix Mono = {
            SoundSample{1},
            SoundSample{1},
            SoundSample{1},
        };
        static constexpr StereoMix StereoABC = {
            SoundSample{1, 0},
            SoundSample{1, 1},
            SoundSample{0, 1},
        };
        static constexpr StereoMix StereoACB = {
            SoundSample{1, 0},
            SoundSample{0, 1},
            SoundSample{1, 1},
        };

    private:
        struct Channel final
        {
            uint16_t period{1};
            bool envelope;
            float volume;
            uint16_t count;
            bool output;
        };

        struct Noise final
        {
            uint8_t period{1};
            uint8_t count;
            uint32_t random{1};
            bool output;
        };

        struct Envelope final
        {
            uint32_t period{1};
            uint32_t count;
            uint8_t step;
            uint8_t shape;
            float volume;
        };

        class EnvelopeLookupTable final
        {
        public:
            static constexpr int Size = 128;

            EnvelopeLookupTable();
            [[nodiscard]] float get(const int shape, const int position) const { return m_values[shape][position]; }

        private:
            std::array<std::array<float, Size>, 16> m_values;
        };

        std::array<uint8_t, 16> m_registers{};
        uint8_t m_counter{};
        uint8_t m_address{};
        uint8_t m_data{};
        std::array<Channel, 3> m_channels{};
        StereoMix m_channelMix{StereoABC};
        Noise m_noise{};
        Envelope m_envelope{};
        EnvelopeLookupTable m_envelopeLookup{};
    };
}  // namespace epoch::sound

#endif
