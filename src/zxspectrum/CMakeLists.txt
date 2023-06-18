add_library(epoch_zxspectrum
    "include/epoch/zxspectrum.h"

    "src/ZX80Cpu.cpp" "src/ZX80Cpu.h"
    "src/ZXSpectrumEmulator.cpp" "src/ZXSpectrumEmulator.h"
)

target_include_directories(epoch_zxspectrum INTERFACE include)
target_link_libraries(epoch_zxspectrum PUBLIC Epoch::Core)

add_library(Epoch::ZXSpectrum ALIAS epoch_zxspectrum)
