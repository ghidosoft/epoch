add_library(epoch_sound
    "include/epoch/sound.hpp"

    "src/AY8910Device.cpp" "src/AY8910Device.hpp"
    "src/SoundDevice.hpp"
)

target_include_directories(epoch_sound INTERFACE include)

# target_link_libraries(epoch_sound PRIVATE Epoch::Core)

add_library(Epoch::Sound ALIAS epoch_sound)
