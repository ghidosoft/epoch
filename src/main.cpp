#include <epoch/frontend.h>
#include <epoch/zxspectrum.h>

int main()
{
    const std::shared_ptr<epoch::Emulator> emulator{ std::make_shared<epoch::zxspectrum::ZXSpectrumEmulator>() };
    epoch::Application application{ emulator };
    return application.run();
}
