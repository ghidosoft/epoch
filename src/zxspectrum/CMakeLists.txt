add_library(epoch_zxspectrum
    "include/epoch/zxspectrum.h"

    "src/Constants.h"
    "src/Io.cpp" "src/Io.h"
    "src/IoTzx.cpp" "src/IoTzx.h"
    "src/Rom.h"
    "src/TapeInterface.h"
    "src/Ula.cpp" "src/Ula.h"
    "src/Z80Cpu.cpp" "src/Z80Cpu.h"
    "src/Z80Interface.h"
    "src/ZXSpectrumEmulator.cpp" "src/ZXSpectrumEmulator.h"
)

target_include_directories(epoch_zxspectrum INTERFACE include)
target_link_libraries(epoch_zxspectrum PUBLIC Epoch::Core)

add_library(Epoch::ZXSpectrum ALIAS epoch_zxspectrum)

add_executable(epoch_zxspectrum_zex
    "src/zex.cpp"
    "src/Z80Cpu.h"
    "src/Z80Interface.h"
)
target_link_libraries(epoch_zxspectrum_zex PUBLIC Epoch::Core Epoch::ZXSpectrum)
