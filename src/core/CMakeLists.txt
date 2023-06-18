add_library(epoch_core
    "include/epoch/core.h"

    "src/Emulator.cpp" "src/Emulator.h"
)

target_include_directories(epoch_core INTERFACE include)

add_library(Epoch::Core ALIAS epoch_core)
