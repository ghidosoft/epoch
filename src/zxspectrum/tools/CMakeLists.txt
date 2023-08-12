add_executable(epoch_zxspectrum_zexall
    utils.hpp zex.cpp)
target_link_libraries(epoch_zxspectrum_zexall PRIVATE Epoch::ZXSpectrum)
target_compile_definitions(epoch_zxspectrum_zexall PRIVATE ZEX_ROM=zexall)

add_executable(epoch_zxspectrum_zexdoc
    utils.hpp zex.cpp)
target_link_libraries(epoch_zxspectrum_zexdoc PRIVATE Epoch::ZXSpectrum)
target_compile_definitions(epoch_zxspectrum_zexdoc PRIVATE ZEX_ROM=zexdoc)

add_executable(epoch_zxspectrum_z80tests
    utils.hpp z80tests.cpp)
target_link_libraries(epoch_zxspectrum_z80tests PRIVATE nlohmann_json::nlohmann_json Epoch::ZXSpectrum)
