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

#include <epoch/frontend.h>
#include <epoch/zxspectrum.h>

int main()
{
    const std::shared_ptr<epoch::Emulator> emulator{ std::make_shared<epoch::zxspectrum::ZXSpectrumEmulator>() };
    epoch::frontend::Application application{ emulator };
    return application.run();
}
