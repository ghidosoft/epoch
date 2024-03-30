add_library(epoch_core
    "include/epoch/core.hpp"

    "src/Emulator.cpp" "src/Emulator.hpp"
    "src/Keyboard.hpp"
    "src/Profiler.hpp"
    "src/Tape.hpp"
)

target_include_directories(epoch_core INTERFACE include)

target_compile_definitions(epoch_core PUBLIC $<$<BOOL:${EPOCH_ENABLE_PROFILER}>:EPOCH_PROFILER>)

add_library(Epoch::Core ALIAS epoch_core)
