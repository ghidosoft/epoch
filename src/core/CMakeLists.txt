add_library(epoch_core
    "include/epoch/core.h"

    "src/Color.h"
    "src/Emulator.cpp" "src/Emulator.h"
    "src/Keyboard.h"
    "src/Palette.cpp" "src/Palette.h"
)

target_include_directories(epoch_core INTERFACE include)

add_library(Epoch::Core ALIAS epoch_core)
