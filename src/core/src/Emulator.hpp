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

#ifndef SRC_EPOCH_CORE_EMULATOR_HPP_
#define SRC_EPOCH_CORE_EMULATOR_HPP_

#include <span>
#include <string>
#include <vector>

#include "Keyboard.hpp"

namespace epoch
{
    struct EmulatorFileFormat
    {
        std::string description;
        std::string extensions;
        bool load;
        bool save;
    };

    struct EmulatorInfo
    {
        std::string name;
        unsigned width;
        unsigned height;
        std::size_t frameClocks;
        double framesPerSecond;
        std::vector<EmulatorFileFormat> fileFormats;

        [[nodiscard]] float aspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }
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

        virtual void load(const std::string& path) = 0;
        virtual void save(const std::string& path) = 0;

        void frame();
        float generateNextAudioSample();

        [[nodiscard]] float audioOut() const { return m_audioOut; }
        void audioIn(const float sample) { m_audioIn = sample; }

        [[nodiscard]] virtual std::span<const uint32_t> screenBuffer() = 0;

        virtual void keyEvent(Key key, KeyAction action) {}

    public:
        [[nodiscard]] const EmulatorInfo& info() const;

    protected:
        virtual void doClock() = 0;

        const EmulatorInfo m_info;
        float m_audioOut{};
        float m_audioIn{};

    private:
        double m_clockDuration;
        double m_elapsed{};
    };
}  // namespace epoch

#endif
