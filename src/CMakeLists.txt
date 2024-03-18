add_subdirectory(core)
add_subdirectory(frontend)
add_subdirectory(sound)
add_subdirectory(zxspectrum)

add_executable(epoch main.cpp)
target_link_libraries(epoch PRIVATE Epoch::Core Epoch::Frontend Epoch::ZXSpectrum)

if(WIN32)
    set_target_properties(epoch PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_options(epoch PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/ENTRY:mainCRTStartup>)
endif()
