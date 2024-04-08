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

#include "FloppyUpd765.hpp"

#include <cassert>

namespace epoch::zxspectrum
{
    void FloppyDrive::seek(const uint8_t cylinder) { m_cylinder = cylinder; }

    void FloppyDrive::load(std::shared_ptr<FloppyImage> image) { m_image = std::move(image); }

    void FloppyDrive::eject() { load(nullptr); }

    FloppyUpd765::FloppyUpd765() { m_input.reserve(9); }

    void FloppyUpd765::reset()
    {
        m_statusRegister = {};
        m_input.clear();
        m_drives = {};
        m_lastDrive = 0;
        m_drives[0].load(std::make_shared<FloppyImage>(std::vector<uint8_t>{0, 0, 0}));
    }

    uint8_t FloppyUpd765::read()
    {
        uint8_t result = 0;
        if (m_statusRegister.dio() == true)
        {
            result = m_output.back();
            m_output.pop_back();
            if (m_output.empty())
            {
                // No more data, set write mode
                m_statusRegister.dio(false);
                m_statusRegister.rqm(true);
            }
        }
        return result;
    }

    void FloppyUpd765::write(const uint8_t data)
    {
        if (m_statusRegister.dio() == false)
        {
            m_input.push_back(data);
            // TODO: parse (and execute) command

            if (m_input[0] == 0x03)
            {
                if (m_input.size() == 3)
                {
                    specify();
                    finishCommand();
                }
            }
            else if (m_input[0] == 0x04)
            {
                if (m_input.size() == 2)
                {
                    senseDriveStatus();
                    finishCommand();
                }
            }
            else if (m_input[0] == 0x07)
            {
                if (m_input.size() == 2)
                {
                    recalibrate();
                    finishCommand();
                }
            }
            else if (m_input[0] == 0x08)
            {
                if (m_input.size() == 1)
                {
                    senseInterruptStatus();
                    finishCommand();
                }
            }
            else if (m_input[0] == 0x0a)
            {
                if (m_input.size() == 2)
                {
                    readId();
                    finishCommand();
                }
            }
            else if (m_input[0] == 0x0f)
            {
                if (m_input.size() == 3)
                {
                    seek();
                    finishCommand();
                }
            }
            else
            {
                invalid();
                finishCommand();
            }
        }
    }

    void FloppyUpd765::load(std::shared_ptr<FloppyImage> image) { m_drives[0].load(std::move(image)); }

    void FloppyUpd765::eject() { m_drives[0].eject(); }

    void FloppyUpd765::specify()
    {
        // Only Non-DMA mode supported
        assert((m_input[2] & 0x01) == 0x01);
    }

    void FloppyUpd765::senseDriveStatus()
    {
        const uint8_t drive = m_input[1] & 0x03;
        if (m_drives[drive].hasImage())
        {
            m_output.push_back(0b00110000 | drive);
        }
        else
        {
            m_output.push_back(0b00101000 | drive);
        }
    }

    void FloppyUpd765::recalibrate() { m_lastDrive = m_input[1] & 0x03; }

    void FloppyUpd765::senseInterruptStatus()
    {
        if (m_drives[m_lastDrive].hasImage())
        {
            m_output.push_back(0b00100000 | m_lastDrive);  // ST0
        }
        else
        {
            m_output.push_back(0b11100000 | m_lastDrive);  // ST0
        }
        m_output.push_back(m_drives[m_lastDrive].cylinder());  // Physical track number
    }

    void FloppyUpd765::readId()
    {
        const uint8_t drive = m_input[1] & 0x03;
        // TODO
    }

    void FloppyUpd765::seek()
    {
        const uint8_t drive = m_input[1] & 0x03;
        m_drives[drive].seek(m_input[2]);
    }

    void FloppyUpd765::invalid() { m_output.push_back(0x80); }

    void FloppyUpd765::finishCommand()
    {
        m_input.clear();
        m_statusRegister.dio(m_output.empty() == false);
    }
}  // namespace epoch::zxspectrum
