# Epoch: ZX Spectrum emulator

![CMakeBadge](https://github.com/ghidosoft/epoch/actions/workflows/cmake.yml/badge.svg)

This is a ZX Spectrum 48K emulator written, for the fun of it, from scratch in C++.

Rendering is done using [GLFW](https://www.glfw.org/) and OpenGL Core 4.1, sound using [PortAudio](http://www.portaudio.com/) and ux uses [Dear ImGui](https://github.com/ocornut/imgui).

## Features
- Instruction level Z80 emulation: all instructions (excluding BIT N, (HL)) passes zexdoc and zexall, bit (hl) doesn't because WZ/MEMPTR is not yet completely implemented
- Load/Save snapshots (.sna)
- Tape loading (.tap)
- Sound

## Roadmap
- 128K spectrum
- .tzx loading
- .z80 snapshots
- Tape saving
- Debugging UX (memory, registers)
- Disassembler

## Screenshots

First 48K ROM running screenshot:

![ZX Spectrum 48K](docs/images/zx48k_main.png)

## Build

This project uses CMake. Builds on Windows (Visual Studio 2022), GNU/Linux (Ubuntu 22.04) and MacOS (12.6.7).

### Ubuntu 22.04 build

```bash
# Build tools
sudo apt install build-essential cmake ninja-build

# Requisites
sudo apt install libasound-dev xorg-dev
```

## License
Copyright &copy; 2023 Andrea Ghidini.

Epoch is [free software](https://www.gnu.org/philosophy/free-sw.html): you can redistribute it and/or modify
it under the terms of the [GNU General Public License](https://www.gnu.org/licenses/gpl-3.0.html) as published by
the [Free Software Foundation](https://www.fsf.org), either version 3 of the License, or
(at your option) any later version.

Epoch is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE**.
See the GNU General Public License for more details.

You should have received a [copy](LICENSE) of the GNU General Public License along with Epoch.
If not, see https://www.gnu.org/licenses/.
