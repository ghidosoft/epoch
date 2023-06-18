#ifndef SRC_EPOCH_CORE_EMULATOR_H_
#define SRC_EPOCH_CORE_EMULATOR_H_

namespace epoch
{
    class Emulator
    {
    public:
        virtual ~Emulator();

    public:
        Emulator(const Emulator& other) = delete;
        Emulator(Emulator&& other) noexcept = delete;
        Emulator& operator=(const Emulator& other) = delete;
        Emulator& operator=(Emulator&& other) noexcept = delete;
    };
}

#endif
