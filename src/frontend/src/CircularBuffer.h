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

#ifndef SRC_FRONTEND_CIRCULARBUFFER_H_
#define SRC_FRONTEND_CIRCULARBUFFER_H_

#include <array>
#include <atomic>
#include <mutex>
#include <span>

namespace epoch::frontend
{
    template<typename T, unsigned long N>
    class CircularBuffer final
    {
    public:
        void read(T* outputBuffer, unsigned long outputSize)
        {
            if (const auto count = std::min(outputSize, N - m_readHead - 1); count > 0)
            {
                std::memcpy(outputBuffer, &m_buffer[m_readHead], count * sizeof(float));
                m_readHead += count;
                outputSize -= count;
                outputBuffer += count;
            }
            if (m_readHead >= N)
            {
                if (outputSize > 0)
                {
                    std::memcpy(outputBuffer, m_buffer.data(), outputSize * sizeof(float));
                }
                m_readHead = outputSize;
            }

            {
                std::lock_guard guard(m_mutex);
                if (outputSize >= m_ahead)
                {
                    m_ahead = 0;
                    m_readHead = m_writeHead = 0; // TODO (?)
                }
                else
                {
                    m_ahead -= outputSize;
                }
            }
        }

        void write(const std::span<const T> inputData)
        {
            auto inputSize = static_cast<unsigned long>(inputData.size());
            const auto count = std::min(inputSize, N - m_writeHead - 1);
            const T* inputBuffer = inputData.data();
            if (count > 0)
            {
                std::memcpy(&m_buffer[m_writeHead], inputBuffer, count * sizeof(float));
                m_writeHead += count;
                inputSize -= count;
                inputBuffer += count;
            }
            if (m_writeHead >= N)
            {
                if (inputSize > 0)
                {
                    std::memcpy(m_buffer.data(), inputBuffer, inputSize * sizeof(T));
                }
                m_writeHead = inputSize;
            }

            {
                std::lock_guard guard(m_mutex);
                m_ahead += inputSize;
            }
        }

        void write(T sample)
        {
            write(std::span<const T, 1>{ &sample, 1 });
        }

        [[nodiscard]] unsigned long ahead() const { return m_ahead; }

    private:
        std::atomic<unsigned long> m_readHead{};
        std::atomic<unsigned long> m_writeHead{};

        std::atomic<unsigned long> m_ahead{};

        std::array<T, N> m_buffer{};

        std::mutex m_mutex{};
    };
}

#endif
