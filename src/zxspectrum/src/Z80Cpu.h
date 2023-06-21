#ifndef SRC_EPOCH_ZXSPECTRUM_Z80CPU_H_
#define SRC_EPOCH_ZXSPECTRUM_Z80CPU_H_

namespace epoch::zxspectrum
{
    class Z80Cpu final
    {
    public:
        Z80Cpu();

    public:
        void clock();
        void reset();
    };
}

#endif
