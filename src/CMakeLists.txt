add_subdirectory(core)
add_subdirectory(frontend)
add_subdirectory(zxspectrum)

add_executable(epoch main.cpp)
target_link_libraries(epoch PRIVATE Epoch::Core Epoch::Frontend Epoch::ZXSpectrum)
