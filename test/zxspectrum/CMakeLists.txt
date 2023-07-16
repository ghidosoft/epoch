add_executable(epoch_zxspectrum_test
    "TestZ80Interface.h"
    "Z80Cpu_ED_test.cpp"
    "Z80Cpu_test.cpp"
)
target_link_libraries(epoch_zxspectrum_test GTest::gtest_main Epoch::ZXSpectrum)

include(GoogleTest)
gtest_discover_tests(epoch_zxspectrum_test)
