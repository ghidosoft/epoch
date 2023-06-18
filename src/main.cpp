#include <epoch/frontend.h>
#include <epoch/zxspectrum.h>

int main()
{
    std::shared_ptr<epoch::Emulator> emulator{ std::make_unique<epoch::ZXSpectrumEmulator>() };
    epoch::Application application{ emulator };
    return application.run();
}
