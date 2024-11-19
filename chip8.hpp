#include <iostream>
#include <cstdint>
#include <random>
using namespace std;

const unsigned int MEMORY_SIZE = 4096;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class chip8
{
public:
    chip8();
    void load_rom(const char *filename);
    void cycle();

    uint8_t keyboard[16]{};
    uint32_t display[VIDEO_WIDTH * VIDEO_HEIGHT]{};

    // memory array
    uint8_t memory[MEMORY_SIZE]{};
    // register array
    uint8_t registers[16]{};
    // 16 bit index register
    uint16_t index{};
    // 16 bit pc
    uint16_t pc{};
    // 16 bit stack memory array with size 16
    uint16_t stack[16]{};
    // 16 bit opcode
    uint16_t opcode{};
    // 8 bit stack pointer
    uint8_t sp{};
    // 8 bit Delay Timer
    uint8_t delayTimer{};
    // 8 bit Sound Timer
    uint8_t soundTimer{};

    default_random_engine randGen;
    uniform_int_distribution<uint8_t> randBytes;

    // all instruction
    void Op_NULL();
    void Op_00E0();
    void Op_00EE();
    void Op_1nnn();
    void Op_2nnn();
    void Op_3xkk();
    void Op_4xkk();
    void Op_5xy0();
    void Op_6xkk();
    void Op_7xkk();
    void Op_8xy0();
    void Op_8xy1();
    void Op_8xy2();
    void Op_8xy3();
    void Op_8xy4();
    void Op_8xy5();
    void Op_8xy6();
    void Op_8xy7();
    void Op_8xyE();
    void Op_9xy0();
    void Op_Annn();
    void Op_Bnnn();
    void Op_Cxkk();
    void Op_Dxyn();
    void Op_Ex9E();
    void Op_ExA1();
    void Op_Fx07();
    void Op_Fx0A();
    void Op_Fx15();
    void Op_Fx18();
    void Op_Fx1E();
    void Op_Fx29();
    void Op_Fx33();
    void Op_Fx55();
    void Op_Fx65();
    void Table0();
    void Table8();
    void TableE();
    void TableF();

    typedef void (chip8::*chip8Func)();
    // initalizing array with size for 0 to f tables
    chip8Func table[0xF + 1];
    // initalizing array for 0th table from 0 to E (as f is not needed)
    chip8Func table0[0xE + 1];
    // initalizing array for 8th table from 0 to E (as F is not needed )
    chip8Func table8[0xE + 1];
    // initalizing array for Eth table from 0 to E (as F is not needed )
    chip8Func tableE[0xE + 1];
    // initalizing array with size for 0 to 65 hex value
    chip8Func tableF[0x65 + 1];
};