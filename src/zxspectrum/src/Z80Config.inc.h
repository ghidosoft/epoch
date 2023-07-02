static const char* Z80CONFIG = R"TSV(
#op     M1      M2      M3      M4      M5      M6      mne     op1     op2
00      op      -       -       -       -       -       NOP
06      op      -       -       -       -       -       LD      B       n
10      op      -       -       -       -       -       DJNZ    d
11      op      -       -       -       -       -       LD      DE      nn
12      op      -       -       -       -       -       LD      (DE)    A
14      op      -       -       -       -       -       INC     D
18      op      -       -       -       -       -       JR      d
3E      op      -       -       -       -       -       LD      A       n
)TSV";
