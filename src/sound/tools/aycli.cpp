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

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

struct TestConfiguration final
{
    uint16_t pitchA;
    uint16_t pitchB;
    uint16_t pitchC;
    uint8_t pitchNoise;
    uint8_t mixer;
    uint8_t volA;
    uint8_t volB;
    uint8_t volC;
    uint16_t envDuration;
    uint8_t envShape;
};

#define SET_REG(idx, value) \
    device.address(idx);    \
    device.data(static_cast<uint8_t>(value))

void executeTest(const TestConfiguration configuration, const unsigned long duration)
{
    epoch::sound::AY8910Device device{};

    SET_REG(0, configuration.pitchA & 0xff);
    SET_REG(1, configuration.pitchA >> 8);
    SET_REG(2, configuration.pitchB & 0xff);
    SET_REG(3, configuration.pitchB >> 8);
    SET_REG(4, configuration.pitchC & 0xff);
    SET_REG(5, configuration.pitchC >> 8);
    SET_REG(6, configuration.pitchNoise);
    SET_REG(7, configuration.mixer);
    SET_REG(8, configuration.volA);
    SET_REG(9, configuration.volB);
    SET_REG(10, configuration.volC);
    SET_REG(11, configuration.envDuration & 0xff);
    SET_REG(12, configuration.envDuration >> 8);
    SET_REG(13, configuration.envShape);

    unsigned long ticks = 0;
    double sampleCount = 0.;

    {
        const epoch::frontend::AudioPlayer player{48000};

        const auto startTime = std::chrono::system_clock::now();
        std::vector<float> samples{};

        do
        {
            if (const unsigned long count = player.neededSamples(); count > 0)
            {
                samples.resize(count * 2);
                auto i = 0;
                for (unsigned long x = 0; x < count; ++x)
                {
                    while (ticks < (sampleCount * 1764000. / 48000.))
                    {
                        device.clock();
                        ticks++;
                    }
                    samples[i++] = device.output().left;
                    samples[i++] = device.output().right;
                    sampleCount++;
                }
                player.push(samples);
            }
        } while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime)
                     .count() < duration);
    }

    std::cout << "Generated samples: " << sampleCount << "\n";
    std::cout << "Ticks:             " << ticks << "\n";
}

int main()
{
    std::cout << "AY8910 CLI utility\n";
    executeTest({.pitchA = 251,
                 .pitchB = 199,
                 .pitchC = 167,
                 .pitchNoise = 0,
                 .mixer = 0b11111000,
                 .volA = 0x0f,
                 .volB = 0x10,
                 .volC = 0x0f,
                 .envDuration = 1000,
                 .envShape = 0b1110},
                2000);
    return EXIT_SUCCESS;
}
