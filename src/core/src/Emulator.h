#ifndef SRC_EPOCH_CORE_EMULATOR_H_
#define SRC_EPOCH_CORE_EMULATOR_H_

#include <string>

namespace epoch
{
    struct EmulatorInfo
    {
        std::string name;
        int width;
        int height;
    };

    class Emulator
    {
    public:
        explicit Emulator(EmulatorInfo info);
        virtual ~Emulator();

    public:
        Emulator(const Emulator& other) = delete;
        Emulator(Emulator&& other) noexcept = delete;
        Emulator& operator=(const Emulator& other) = delete;
        Emulator& operator=(Emulator&& other) noexcept = delete;

    public:
        virtual void reset() = 0;
        virtual void clock() = 0;

    public:
        [[nodiscard]] const EmulatorInfo& info() const;

    protected:
        const EmulatorInfo m_info;
    };
}

#endif
