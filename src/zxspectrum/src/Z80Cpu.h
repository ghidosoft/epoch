#ifndef SRC_EPOCH_ZXSPECTRUM_Z80CPU_H_
#define SRC_EPOCH_ZXSPECTRUM_Z80CPU_H_

#include <cstdint>

namespace epoch::zxspectrum
{
    union WordRegister
    {
        uint16_t value;
        struct
        {
            // TODO: endianness
            uint8_t low;
            uint8_t high;
        };
    };

    union Flags
    {
        struct
        {
            // TODO: endianness
            bool c : 1;
            bool n : 1;
            bool p : 1;
            bool x : 1;
            bool h : 1;
            bool y : 1;
            bool z : 1;
            bool s : 1;
        };
        uint8_t value;
    };

    union WordFlagsRegister
    {
        uint16_t value;
        struct
        {
            // TODO: endianness
            uint8_t low;
            uint8_t high;
        };
        Flags flags;
    };

    struct Z80Registers
    {
        // PC Program counter
        uint16_t pc{0x0000};
        // SP Stack pointer
        uint16_t sp{0xffff};
        // IX Index X
        WordRegister ix{0xffff};
        // IY Index Y
        WordRegister iy{0xffff};
        // IR Interrupt/Refresh register
        WordRegister ir{0x00ff};
        // AF Accumulator and flags
        WordFlagsRegister af{0xffff};
        // BC generale purpose
        WordRegister bc{0xffff};
        // DE generale purpose
        WordRegister de{0xffff};
        // HL generale purpose
        WordRegister hl{0xffff};
        // AF shadow
        WordFlagsRegister af2{0xffff};
        // BC shadow
        WordRegister bc2{0xfff};
        // DE shadow
        WordRegister de2{0xffff};
        // HL shadow
        WordRegister hl2{0xffff};
    };

    class Z80Cpu final
    {
    public:
        Z80Cpu();

    public:
        void clock();
        void reset();

        Z80Registers& registers();

    private:
        Z80Registers m_registers{};
    };
}

#endif
