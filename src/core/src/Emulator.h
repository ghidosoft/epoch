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

#ifndef SRC_EPOCH_CORE_EMULATOR_H_
#define SRC_EPOCH_CORE_EMULATOR_H_

#include <span>
#include <string>

#include "Keyboard.h"

namespace epoch
{
    struct EmulatorInfo
    {
        std::string name;
        int width;
        int height;
        std::size_t frameClocks;
        double framesPerSecond;
    };

    class Emulator
    {
    public:
        explicit Emulator(EmulatorInfo info);
        virtual ~Emulator();

    public:
        Emulator(const Emulator& other) = delete;
        Emulator(Emulator&& other) noexcept = delete;
        Emulator& operator=(const Emulator& other) = delete;
        Emulator& operator=(Emulator&& other) noexcept = delete;

    public:
        virtual void reset() = 0;
        void clock();

        virtual void loadSnapshot(const std::string& path) = 0;
        virtual void saveSnapshot(const std::string& path) = 0;

        void frame();
        float generateNextAudioSample();

        [[nodiscard]] float audioSample() const { return m_audioSample; }

        [[nodiscard]] virtual std::span<const uint32_t> screenBuffer() = 0;

        virtual void keyEvent(Key key, KeyAction action) {}

    public:
        [[nodiscard]] const EmulatorInfo& info() const;

    protected:
        virtual void doClock() = 0;

        const EmulatorInfo m_info;
        float m_audioSample{};

    private:
        double m_clockDuration;
        double m_elapsed{};
    };
}

#endif
