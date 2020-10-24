#pragma once

enum class SAPOpcode : uint8_t
{
    ADD_B   = 0x80,
    ADD_C   = 0x81,
    ANA_B   = 0xA0,
    ANA_C   = 0xA1,
    ANI     = 0xE6,
    CALL    = 0xCD,
    CMA     = 0x2F,
    DCR_A   = 0x3D,
    DCR_B   = 0x05,
    DCR_C   = 0x0D,
    HLT     = 0x76,
    IN      = 0xDB,
    INR_A   = 0x3C,
    INR_B   = 0x04,
    INR_C   = 0x0C,
    JM      = 0xFA,
    JMP     = 0xC3,
    JNZ     = 0xC2,
    JZ      = 0xCA,
    LDA     = 0x3A,
    MOV_AB  = 0x78,
    MOV_AC  = 0x79,
    MOV_BA  = 0x47,
    MOV_BC  = 0x41,
    MOV_CA  = 0x4F,
    MOV_CB  = 0x48,
    MVI_A   = 0x3E,
    MVI_B   = 0x06,
    MVI_C   = 0x0E,
    NOP     = 0,
    ORA_B   = 0xB0,
    ORA_C   = 0xB1,
    ORI     = 0xF6,
    OUT     = 0xD3,
    RAL     = 0x17,
    RAR     = 0x1F,
    RET     = 0xC9,
    STA     = 0x32,
    SUB_B   = 0x90,
    SUB_C   = 0x91,
    XRA_B   = 0xA8,
    XRA_C   = 0xA9,
    XRI     = 0xEE
};

enum class SAPOpType
{
    Register,
    Addressing,
    Immediate
};
