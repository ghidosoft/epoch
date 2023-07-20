# Epoch: ZX Spectrum emulator

![CMakeBadge](https://github.com/ghidosoft/epoch/actions/workflows/cmake.yml/badge.svg)

This is a ZX Spectrum 48K emulator written from scratch in C++.

Rendering is done using GLFW and OpenGL Core 4.1.

## Features
- Basic Z80 emulation (all currently implemented instructions passes zexdoc, missing cpd, cpr, daa, cpl, scf, ccf)
- WIP

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
