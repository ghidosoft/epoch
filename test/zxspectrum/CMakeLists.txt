add_executable(epoch_zxspectrum_test
    "Z80Cpu_test.cpp"
)
target_link_libraries(epoch_zxspectrum_test GTest::gtest_main Epoch::ZXSpectrum)

include(GoogleTest)
gtest_discover_tests(epoch_zxspectrum_test)
