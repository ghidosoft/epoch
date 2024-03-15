add_executable(epoch_sound_aycli
    aycli.cpp)
target_link_libraries(epoch_sound_aycli PRIVATE Epoch::Frontend Epoch::Sound)
