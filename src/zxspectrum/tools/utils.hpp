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

#ifndef SRC_EPOCH_ZXSPECTRUM_TOOLS_UTILS_HPP_
#define SRC_EPOCH_ZXSPECTRUM_TOOLS_UTILS_HPP_

#include <cassert>
#include <span>
#include <stdexcept>
#include <vector>

#include "../src/Z80Cpu.hpp"

class RamZ80Interface : public epoch::zxspectrum::Z80Interface
{
public:
    struct IoOperation
    {
        IoOperation() = default;
        IoOperation(const uint16_t port, const uint8_t value, const bool write) : port(port), value(value), write(write) {}
        uint16_t port;
        uint8_t value;
        bool write;
    };

    RamZ80Interface() = default;
    explicit RamZ80Interface(const std::span<uint8_t> ram)
    {
        assert(ram.size() <= m_ram.size());
        std::memcpy(m_ram.data(), ram.data(), ram.size());
    }

    uint8_t read(const uint16_t address) override
    {
        return m_ram[address];
    }
    void write(const uint16_t address, const uint8_t value) override
    {
        m_ram[address] = value;
    }

    uint8_t ioRead(const uint16_t port) override
    {
        if (m_nextIoOperation >= m_ioOperations.size()) throw std::runtime_error("Unexpected IO operation");
        const auto& op = m_ioOperations[m_nextIoOperation++];
        if (op.write) throw std::runtime_error("Invalid IO operation (should be read)");
        if (op.port != port) throw std::runtime_error("Invalid IO read operation port, expected=" + std::to_string(op.port) + " actual=" + std::to_string(port));
        return op.value;
    }
    void ioWrite(const uint16_t port, const uint8_t value) override
    {
        if (m_nextIoOperation >= m_ioOperations.size()) throw std::runtime_error("Unexpected IO operation");
        const auto& op = m_ioOperations[m_nextIoOperation++];
        if (!op.write) throw std::runtime_error("Invalid IO operation (should be write)");
        if (op.port != port) throw std::runtime_error("Invalid IO write operation port, expected=" + std::to_string(op.port) + " actual=" + std::to_string(port));
        if (op.value != value) throw std::runtime_error("Invalid IO write operation value, expected=" + std::to_string(op.value) + " actual=" + std::to_string(value));
    }

    std::span<uint8_t> ram() { return m_ram; }

    void setIoOperations(std::span<const IoOperation> ioOperations)
    {
        m_ioOperations.assign(ioOperations.begin(), ioOperations.end());
        m_nextIoOperation = 0;
    }

private:
    std::array<uint8_t, 0x10000> m_ram;
    std::vector<IoOperation> m_ioOperations;
    std::size_t m_nextIoOperation{};
};

#endif
