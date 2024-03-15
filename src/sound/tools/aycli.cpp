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

#include <epoch/sound.hpp>

#include "../../frontend/src/AudioPlayer.hpp"

#include <cstdlib>
#include <iostream>

struct TestConfiguration final
{
    uint16_t toneA;
    uint16_t toneB;
    uint16_t toneC;
    uint8_t noise;
    uint8_t mixer;
    uint8_t volA;
    uint8_t volB;
    uint8_t volC;
    uint8_t envFreq;
    uint8_t env;
};

#define SET_REG(idx, value) \
    device.address(idx);    \
    device.data(static_cast<uint8_t>(value))

void executeTest(const TestConfiguration configuration)
{
    epoch::sound::AY8910Device device{};

    SET_REG(0, configuration.toneA & 0xff);
    SET_REG(1, configuration.toneA >> 8);
    SET_REG(2, configuration.toneB & 0xff);
    SET_REG(3, configuration.toneB >> 8);
    SET_REG(4, configuration.toneC & 0xff);
    SET_REG(5, configuration.toneC >> 8);
    SET_REG(6, configuration.noise);
    SET_REG(7, configuration.mixer);
    SET_REG(8, configuration.volA);
    SET_REG(9, configuration.volB);
    SET_REG(10, configuration.volC);
    SET_REG(11, configuration.envFreq & 0xff);
    SET_REG(12, configuration.envFreq >> 8);
    SET_REG(13, configuration.env);

    epoch::frontend::AudioPlayer player{48000, 1};

    // TODO: call device.clock() and feed the sound player
}

int main()
{
    std::cout << "AY8910 CLI utility\n";
    {
        executeTest({.toneA = 400,
                     .toneB = 0,
                     .toneC = 0,
                     .noise = 0,
                     .mixer = 0b11111110,
                     .volA = 15,
                     .volB = 0,
                     .volC = 0,
                     .envFreq = 0,
                     .env = 0});
    }
    return EXIT_SUCCESS;
}
