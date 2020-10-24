#include <iostream>
#include <fstream>
#include "Opcode.h"

#define UPDATE_FLAGS(state) \
    (state).sign = (int8_t)(state).a < 0; \
    (state).zero = (state).a == 0

struct SAPState
{
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    uint16_t pc = 0;

    bool sign : 1;
    bool zero : 1;

    uint8_t* memory = nullptr;
};

union SAPInstruction
{
    SAPOpcode opcode;

    struct {
        uint32_t opcode : 8;
    } type0;

    struct {
        uint32_t opcode : 8;
        uint32_t operand8 : 8;
    } type1;

    struct {
        uint32_t opcode : 8;
        uint32_t operand16 : 16;
    } type2;

    uint32_t instr;
};

struct SAPInstructionInfo
{
    SAPOpcode opcode;
    SAPOpType optype;
    uint8_t size;
};

class SAPVCPU
{
public:
    SAPVCPU()
    {
        resetState();
    }

    ~SAPVCPU()
    {
        delete state.memory;
    }

    void loadProgram(const uint8_t* program, size_t size)
    {
        std::memcpy(state.memory, program, size);
    }

    void setPC(uint16_t pc)
    {
        state.pc = pc;
    }

    void run()
    {
        isRunning = true;

        while (isRunning) {
            SAPInstruction currentInstruction;

            // fetch
            currentInstruction.instr = *((uint32_t*)(state.memory + state.pc));

            // decode
            switch (currentInstruction.opcode) {
                case SAPOpcode::ADD_B:
                    state.a += state.b;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::ADD_C:
                    state.a += state.c;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::ANA_B:
                    state.a &= state.b;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::ANA_C:
                    state.a &= state.c;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::ANI:
                    state.a &= currentInstruction.type1.operand8;
                    UPDATE_FLAGS(state);
                    state.pc += 2;
                    break;
                case SAPOpcode::CALL: {
                    uint16_t nextPC = state.pc + 3;
                    state.pc = currentInstruction.type2.operand16;
                    *((uint16_t*)(state.memory + 0xFFFE)) = nextPC;
                    break;
                }
                case SAPOpcode::CMA:
                    state.a = ~state.a;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::DCR_A:
                    --state.a;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::DCR_B:
                    --state.b;
                    state.pc += 1;
                    break;
                case SAPOpcode::DCR_C:
                    --state.c;
                    state.pc += 1;
                    break;
                case SAPOpcode::HLT: // we just exit the program instead
                    isRunning = false;
                    state.pc += 1;
                    break;
                case SAPOpcode::IN:
                    state.a = doInput(currentInstruction.type1.operand8);
                    UPDATE_FLAGS(state);
                    break;
                case SAPOpcode::INR_A:
                    ++state.a;
                    UPDATE_FLAGS(state);
                    break;
                case SAPOpcode::INR_B:
                    ++state.b;
                    break;
                case SAPOpcode::INR_C:
                    ++state.c;
                    break;
                case SAPOpcode::JM:
                    if (state.sign) {
                        state.pc = currentInstruction.type2.operand16;
                    }
                    break;
                case SAPOpcode::JMP:
                    state.pc = currentInstruction.type2.operand16;
                    break;
                case SAPOpcode::JZ:
                    if (state.zero) {
                        state.pc = currentInstruction.type2.operand16;
                    }
                    break;
                case SAPOpcode::JNZ:
                    if (!state.zero) {
                        state.pc = currentInstruction.type2.operand16;
                    }
                    break;
                case SAPOpcode::LDA:
                    state.a = state.memory[currentInstruction.type2.operand16];
                    UPDATE_FLAGS(state);
                    state.pc += 3;
                    break;
                case SAPOpcode::MOV_AB:
                    state.a = state.b;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::MOV_AC:
                    state.a = state.c;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::MOV_BA:
                    state.b = state.a;
                    state.pc += 1;
                    break;
                case SAPOpcode::MOV_BC:
                    state.b = state.c;
                    state.pc += 1;
                    break;
                case SAPOpcode::MOV_CA:
                    state.c = state.a;
                    state.pc += 1;
                    break;
                case SAPOpcode::MOV_CB:
                    state.c = state.b;
                    state.pc += 1;
                    break;
                case SAPOpcode::MVI_A:
                    state.a = currentInstruction.type1.operand8;
                    UPDATE_FLAGS(state);
                    state.pc += 2;
                    break;
                case SAPOpcode::MVI_B:
                    state.b = currentInstruction.type1.operand8;
                    state.pc += 2;
                    break;
                case SAPOpcode::MVI_C:
                    state.c = currentInstruction.type1.operand8;
                    state.pc += 2;
                    break;
                case SAPOpcode::NOP:
                    state.pc += 1;
                    break;
                case SAPOpcode::ORA_B:
                    state.a |= state.b;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::ORA_C:
                    state.a |= state.c;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::ORI:
                    state.a |= currentInstruction.type1.operand8;
                    UPDATE_FLAGS(state);
                    state.pc += 2;
                    break;
                case SAPOpcode::OUT:
                    doOutput(currentInstruction.type1.operand8);
                    state.pc += 2;
                    break;
                case SAPOpcode::RAL:
                    state.a = (state.a << 1) | (state.a >> 7);
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::RAR:
                    state.a = (state.a >> 1) | (state.a << 7);
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::RET:
                    state.pc = *((uint16_t*)(state.memory + 0xFFFE));
                    break;
                case SAPOpcode::STA:
                    state.memory[currentInstruction.type2.operand16] = state.a;
                    state.pc += 3;
                    break;
                case SAPOpcode::SUB_B:
                    state.a -= state.b;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::SUB_C:
                    state.a -= state.c;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::XRA_B:
                    state.a ^= state.b;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::XRA_C:
                    state.a ^= state.c;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                case SAPOpcode::XRI:
                    state.a ^= currentInstruction.type1.operand8;
                    UPDATE_FLAGS(state);
                    state.pc += 1;
                    break;
                default:
                    throw std::runtime_error("Unimplemented or invalid instruction");
            }
        }
    }

    void resetState()
    {
        state.a = 0;
        state.b = 0;
        state.c = 0;

        UPDATE_FLAGS(state);
        
        if (state.memory == nullptr) {
            state.memory = new uint8_t[0xFFFF + 8]; // +8 bytes padding for convenient
        }

        std::memset(state.memory, 0, 0xFFFF + 8);
    }

private:
    SAPState state;
    bool isRunning;

    uint8_t doInput(uint8_t port)
    {
        char c;
        std::cin.get(c);

        state.a = c;
        
        switch (port) {
            case 1:
            case 2:
                return 1;
            default:
                return 0;
        }
    }

    void doOutput(uint8_t port)
    {
        if (port != 3) {
            return;
        }

        std::cout << std::hex << (int)state.a;
    }
};

void loadFile(const std::string& str, SAPVCPU& vcpu)
{
    std::ifstream file(str, std::ios::binary | std::ios::in);
    uint8_t* program = nullptr;
    uint16_t codeSize = 0;
    uint16_t initialPC = 0;
    uint32_t fourcc = 0;

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file");
    }

    file.read((char*)&fourcc, 4);

    if (fourcc != 0x43504153) {
        throw std::runtime_error("File not compatible");
    }

    file.read((char*)&codeSize, 2);
    file.read((char*)&initialPC, 2);
    program = new uint8_t[codeSize];
    file.read((char*)program, codeSize);

    vcpu.setPC(initialPC);
    vcpu.loadProgram(program, codeSize);

    delete[] program;
}

// testing some of the instruction to make sure it work properly
uint8_t program[] = {
    0x3E, 9,
    0x90,
    0xD3, 3,
    0xC9,
    0x3E, 4,
    0x06, 2,
    0xD3, 3,
    0xCD, 0x00, 0x00,
    0x76
};

int main(int argc, char** argv)
{
    SAPVCPU vcpu;

    if (argc < 2) {
        throw std::runtime_error("No input file");
    }

    loadFile(argv[1], vcpu);
    vcpu.run();

    return 0;
}
