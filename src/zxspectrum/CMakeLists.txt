add_library(epoch_zxspectrum
    "include/epoch/zxspectrum.hpp"

    "src/Constants.hpp"
    "src/Io.cpp" "src/Io.hpp"
    "src/IoSnapshot.cpp" "src/IoSnapshot.hpp"
    "src/IoTzx.cpp" "src/IoTzx.hpp"
    "src/Rom.hpp"
    "src/TapeInterface.hpp"
    "src/Ula.cpp" "src/Ula.hpp"
    "src/Z80Cpu.cpp" "src/Z80Cpu.hpp"
    "src/Z80Interface.hpp"
    "src/Z80Tables.hpp"
    "src/ZXSpectrumEmulator.cpp" "src/ZXSpectrumEmulator.hpp"
)

target_include_directories(epoch_zxspectrum INTERFACE include)
target_link_libraries(epoch_zxspectrum PUBLIC Epoch::Core)

add_library(Epoch::ZXSpectrum ALIAS epoch_zxspectrum)

add_subdirectory(tools)
