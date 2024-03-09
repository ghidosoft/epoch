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

#ifndef SRC_FRONTEND_CIRCULARBUFFER_HPP_
#define SRC_FRONTEND_CIRCULARBUFFER_HPP_

#include <array>
#include <atomic>
#include <cassert>
#include <cstring>
#include <mutex>
#include <span>

namespace epoch::frontend
{
    template<typename T, unsigned long N>
    class CircularBuffer final
    {
    public:
        void read(T* outputBuffer, const unsigned long outputSize)
        {
            assert(outputSize <= N);
            auto remaining = outputSize;
            auto rh = m_readHead.load();
            if (const auto count = std::min(outputSize, N - rh); count > 0)
            {
                std::memcpy(outputBuffer, &m_buffer[rh], count * sizeof(T));
                rh = m_readHead += count;
                remaining -= count;
                outputBuffer += count;
            }
            if (rh >= N)
            {
                if (remaining > 0)
                {
                    std::memcpy(outputBuffer, m_buffer.data(), remaining * sizeof(T));
                }
                m_readHead = remaining;
            }

            {
                std::lock_guard guard(m_mutex);
                if (outputSize >= m_available)
                {
                    m_available = 0;
                    m_readHead = m_writeHead.load();  // TODO Is this the right thing to do?
                }
                else
                {
                    m_available -= outputSize;
                }
            }
        }

        void write(const T* inputData, const unsigned long inputSize)
        {
            auto remaining = inputSize;
            auto wh = m_writeHead.load();
            const auto count = std::min(inputSize, N - wh);
            if (count > 0)
            {
                std::memcpy(&m_buffer[wh], inputData, count * sizeof(T));
                wh = m_writeHead += count;
                remaining -= count;
                inputData += count;
            }
            if (wh >= N)
            {
                if (remaining > 0)
                {
                    std::memcpy(m_buffer.data(), inputData, remaining * sizeof(T));
                }
                m_writeHead = remaining;
            }

            {
                std::lock_guard guard(m_mutex);
                m_available = std::min(m_available + inputSize, N);
            }
        }

        void write(const std::span<const T> inputData)
        {
            write(inputData.data(), static_cast<unsigned long>(inputData.size()));
        }

        void write(const T sample) { write(&sample, 1); }

        [[nodiscard]] unsigned long available() const { return m_available; }

    private:
        std::atomic<unsigned long> m_readHead{};
        std::atomic<unsigned long> m_writeHead{};

        std::atomic<unsigned long> m_available{};

        std::array<T, N> m_buffer{};

        std::mutex m_mutex{};
    };
}  // namespace epoch::frontend

#endif
