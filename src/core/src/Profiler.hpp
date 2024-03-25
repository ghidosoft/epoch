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

#ifndef SRC_EPOCH_CORE_PROFILER_HPP_
#define SRC_EPOCH_CORE_PROFILER_HPP_

#include <chrono>

namespace epoch
{
#ifdef EPOCH_PROFILER
    class BlockProfiler final
    {
    public:
        using value_t = float;

        explicit BlockProfiler(value_t* target) : m_target{target}
        {
            m_start = std::chrono::high_resolution_clock::now();
        }
        ~BlockProfiler()
        {
            const auto end = std::chrono::high_resolution_clock::now();
            const auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
            *m_target = static_cast<value_t>(diff.count()) * 0.001f;
        }

        BlockProfiler(const BlockProfiler& other) = default;
        BlockProfiler(BlockProfiler&& other) noexcept = default;
        BlockProfiler& operator=(const BlockProfiler& other) = default;
        BlockProfiler& operator=(BlockProfiler&& other) noexcept = default;

    private:
        value_t* m_target;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    };
}  // namespace epoch

#define PROFILE_BLOCK(x) BlockProfiler __epoch__profiler(x)
#else
#define PROFILE_BLOCK(x) \
    do                   \
    {                    \
    } while (false)
#endif

#endif
