add_library(epoch_core
    "include/epoch/core.hpp"

    "src/Color.hpp"
    "src/Emulator.cpp" "src/Emulator.hpp"
    "src/Keyboard.hpp"
    "src/Palette.cpp" "src/Palette.hpp"
)

target_include_directories(epoch_core INTERFACE include)

add_library(Epoch::Core ALIAS epoch_core)
