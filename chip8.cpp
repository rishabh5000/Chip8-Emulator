#include "chip8.hpp"
#include <iostream>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <random>
#include <cstring>
using namespace std;

const unsigned int Start_Address = 0x200; // pehle ke reserved hoti hai memory
const unsigned int font_start_address = 0x50;

// loading fonts in emulator
uint8_t fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

chip8::chip8()
    : randGen(chrono::system_clock::now().time_since_epoch().count())
{
    pc = Start_Address;
    index = 0;
    sp = 0;
    for(int i=0;i<16;i++){registers[i] = 0;}

    for (int i = 0; i < 80; i++)
    {
        memory[0x50 + i] = fontset[i];
    }
    randBytes = uniform_int_distribution<uint8_t>(0, 255U);

    table[0x0] = &chip8::Table0;
    table[0x1] = &chip8::Op_1nnn; // address of function for opcode starting with 1
    table[0x2] = &chip8::Op_2nnn;
    table[0x3] = &chip8::Op_3xkk;
    table[0x4] = &chip8::Op_4xkk;
    table[0x5] = &chip8::Op_5xy0;
    table[0x6] = &chip8::Op_6xkk;
    table[0x7] = &chip8::Op_7xkk;
    table[0x8] = &chip8::Table8;
    table[0x9] = &chip8::Op_9xy0;
    table[0xA] = &chip8::Op_Annn;
    table[0xB] = &chip8::Op_Bnnn;
    table[0xC] = &chip8::Op_Cxkk;
    table[0xD] = &chip8::Op_Dxyn;
    table[0xE] = &chip8::TableE;
    table[0xF] = &chip8::TableF;

    // initalizing table0 E 8 with NULL function values
    for (uint8_t i = 0; i <= 0xE; i++)
    {
        table0[i] = &chip8::Op_NULL;
        tableE[i] = &chip8::Op_NULL;
        table8[i] = &chip8::Op_NULL;
    }
    // initalizing tableF with NULL function values
    for (uint8_t i = 0; i <= 0x65; i++)
    {
        tableF[i] = &chip8::Op_NULL;
    }

    // setting function pointer the particular table value
    table0[0x0] = &chip8::Op_00E0;
    table0[0xE] = &chip8::Op_00EE;

    table8[0x0] = &chip8::Op_8xy0;
    table8[0x1] = &chip8::Op_8xy1;
    table8[0x2] = &chip8::Op_8xy2;
    table8[0x3] = &chip8::Op_8xy3;
    table8[0x4] = &chip8::Op_8xy4;
    table8[0x5] = &chip8::Op_8xy5;
    table8[0x6] = &chip8::Op_8xy6;
    table8[0x7] = &chip8::Op_8xy7;
    table8[0xE] = &chip8::Op_8xyE;

    tableE[0x1] = &chip8::Op_ExA1;
    tableE[0xE] = &chip8::Op_Ex9E;

    tableF[0x07] = &chip8::Op_Fx07;
    tableF[0x0A] = &chip8::Op_Fx0A;
    tableF[0x15] = &chip8::Op_Fx15;
    tableF[0x18] = &chip8::Op_Fx18;
    tableF[0x1E] = &chip8::Op_Fx1E;
    tableF[0x33] = &chip8::Op_Fx33;
    tableF[0x55] = &chip8::Op_Fx55;
    tableF[0x65] = &chip8::Op_Fx65;
}
void chip8::Table0()
{
    uint8_t add = (opcode & 0x000F); // extracting the last digit
    chip8Func ptr = table0[add];     // creating a function pointer

    ((*this).*ptr)(); // calling function on the instance
}
void chip8::TableE()
{
    uint8_t add = (opcode & 0x000F); // extracting the last digit
    chip8Func ptr = tableE[add];     // creating a function pointer

    ((*this).*ptr)(); // calling function on the instance
}
void chip8::TableF()
{
    uint8_t add = (opcode & 0x000F); // extracting the last digit
    chip8Func ptr = tableF[add];     // creating a function pointer

    ((*this).*ptr)(); // calling function on the instance
}
void chip8::Table8()
{
    uint8_t add = (opcode & 0x000F); // extracting the last
    chip8Func ptr = table8[add];

    ((*this).*ptr)();
}
// loading rom file
void chip8::load_rom(const char *filename)
{
    // opening file in binary mode
    ifstream romfile(filename, ios::binary);
    if (romfile.is_open())
    {
        romfile.seekg(0, ios::end);
        cout<<"Opened"<<endl;
        // storing size of rom file
        streampos size = romfile.tellg();
        // taking pointer to begging of the file
        romfile.seekg(0, ios::beg);
        // making buffer memory location to store the file data
        char *buffer = new char[size];
        // reading binary file and storing data into buffer memory
        romfile.read(buffer, size);

        // writing romfile data into memory form location 0x200
        for (int i = 0; i < size; i++)
        {
            memory[Start_Address + i] = buffer[i];
        }
        delete[] buffer;
    }
    else
    {
        cout << "file not opened" << endl;
    }
}
// DO NOTHING
void chip8::Op_NULL() {}
// CLEAR SCREEN
void chip8::Op_00E0()
{   
    cout<<"fetched: 00E0"<<endl;
    memset(display, 0, sizeof(display));
}

// RETURN FROM SUBROUTINE
void chip8::Op_00EE()
{
    // deccreament sp
    cout<<"fetched: 00EE"<<endl;
    sp--;
    // store the address of instruction before call from stack
    pc = stack[sp];
}
// JUMP TO ADDRESS NNN
void chip8::Op_1nnn()
{
    cout<<"fetched: 1nnn"<<endl;
    // calculating lower 12 bit from opcode (the address where to jump)
    uint16_t address = opcode & 0x0FFF;
    // setting pc to the nnn address
    pc = address;
}
// CALL SUBROUTINE AT NNN
void chip8::Op_2nnn()
{
    cout<<"fetched: 2nnn"<<endl;
    // storing pc on stack
    stack[sp] = pc;
    // incremanting sp
    sp++;
    if (sp < 0 || sp > 15) {
    cout << "Stack Error: SP out of bounds (" << sp << ")\n";
    exit(EXIT_FAILURE);
    }
    cout << " SP is not out of bounds (" << sp << ")\n";
    // getting nnn
    uint16_t address = opcode & 0x0FFF;
    // setting pc to nnn
    pc = address;
}
// SKIP NEXT INST IF Vx = kk
void chip8::Op_3xkk()
{   
    cout<<"fetched: 3xkk"<<endl;
    // extracting kk
    uint8_t kk = opcode & 0x00FF;
    // extracting Vx
    uint8_t Vx = (opcode & 0x0F00) >> 8; //
    if (registers[Vx] == kk)
    {
        pc = pc + 2;
    }
}
// SKIP NEXT INST IF Vx != kk
void chip8::Op_4xkk()
{
    cout<<"fetched: 4xkk"<<endl;
    uint8_t kk = opcode & 0x00FF;
    uint8_t Vx = (opcode & 0x0F00) >> 8;

    if (registers[Vx] != kk)
    {
        pc = pc + 2;
    }
}
// SKIP NEXT INST IF Vx == Vy
void chip8::Op_5xy0()
{
    cout<<"fetched: 5xy0"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;

    if (registers[Vx] == registers[Vy])
    {
        pc = pc + 2;
    }
}
// SETS REGISTER Vx VALUE TO kk
void chip8::Op_6xkk()
{   
    cout<<"fetched: 6xkk"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    registers[Vx] = kk;
}
// ADD VALUE IN Vx WITH kk AND STORE IN Vx
void chip8::Op_7xkk()
{
    cout<<"fetched: 7xkk"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    registers[Vx] += kk;
}
// STORES VALUE IN Vy IN Vx
void chip8::Op_8xy0()
{
    cout<<"fetched: 8xy0"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;

    registers[Vx] = registers[Vy];
}
// Set Vx = Vx OR Vy.
void chip8::Op_8xy1()
{
    cout<<"fetched: 8xy1"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;
    // or between Vx and Vy
    registers[Vx] = registers[Vx] | registers[Vy];
}
// Set Vx = Vx AND Vy.
void chip8::Op_8xy2()
{
    cout<<"fetched: 8xy2"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;
    // and between Vx and Vy
    registers[Vx] = registers[Vx] & registers[Vy];
}
// Set Vx = Vx XOR Vy.
void chip8::Op_8xy3()
{
    cout<<"fetched: 8xy3"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;
    // xor between Vx and Vy
    registers[Vx] = registers[Vx] ^ registers[Vy];
}
// Set Vx = Vx + Vy, set VF = carry.
void chip8::Op_8xy4()
{
    cout<<"fetched: 8xy4"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;

    uint16_t sum = registers[Vx] + registers[Vy];
    // checking 8 bit or not
    if (sum >= 0 && sum <= 255)
    {
        registers[0xF] = 0;
    }
    else
    {
        registers[0xF] = 1;
    }
    // converting 16 bit into 8 bit
    registers[Vx] = sum & 0xFF;
}
// Set Vx = Vx - Vy, set VF = NOT borrow
void chip8::Op_8xy5()
{
    cout<<"fetched: 8xy5"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;
    // checking condition
    if (registers[Vx] > registers[Vy])
        registers[0xF] = 1;
    else
        registers[0xF] = 0;
    // subtracting Vy from Vx
    registers[Vx] = registers[Vx] - registers[Vy];
}
// Set Vx = Vx SHR 1.
void chip8::Op_8xy6()
{
    cout<<"fetched: 8xy6"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    // set Vf with lsb of Vx
    registers[0xF] = registers[Vx] & 0x01;
    // right shift by 1
    registers[Vx] >>= 1;
}
// Set Vx = Vy - Vx, set VF = NOT borrow.
void chip8::Op_8xy7()
{
    cout<<"fetched: 8xy7"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;
    // checking condition
    if (registers[Vy] > registers[Vx])
        registers[0xF] = 1;
    else
        registers[0xF] = 0;
    // subtracting Vx from Vy
    registers[Vx] = registers[Vy] - registers[Vx];
}
// Set Vx = Vx SHL 1.
void chip8::Op_8xyE()
{
    cout<<"fetched: 8xyE"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    // finding msb
    registers[0xF] = (registers[Vx] & 0x80) >> 7;
    // left rotate by 1 bit
    registers[Vx] <<= 1;
}
// Skip next instruction if Vx != Vy.
void chip8::Op_9xy0()
{
    cout<<"fetched: 9xy0"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Vy = (opcode & 0x00F0) >> 4;

    if (registers[Vx] != registers[Vy])
    {
        pc = pc + 2;
    }
}
// set index register with nnn
void chip8::Op_Annn()
{
    cout<<"fetched: Annn"<<endl;
    uint16_t nnn = opcode & 0x0FFFu;
    index = nnn;
}
// set pc to nnn + V0
void chip8::Op_Bnnn()
{
    cout<<"fetched: Bnnn"<<endl;
    uint16_t nnn = opcode & 0x0FFF;
    pc = nnn + registers[0];
}
// set Vx = randombyte AND kk
void chip8::Op_Cxkk()
{
    cout<<"fetched: Cxkk"<<endl;
    uint8_t Cx = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    registers[Cx] = randBytes(randGen) & kk;
}
// bhai ye nahi aaraha hai tu kr le
void chip8::Op_Dxyn()
{
    cout<<"fetched: Dxyn"<<endl;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &display[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}

}
// Skip next instruction if key with the value of Vx is pressed.
void chip8::Op_Ex9E()
{
    cout<<"fetched: Ex9E"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    // condition if the key corresponding to the value of Vx is currently in the down position
    if (keyboard[registers[Vx]])
    {
        pc = pc + 2;
    }
}
// Skip next instruction if key with the value of Vx is not pressed.
void chip8::Op_ExA1()
{
    cout<<"fetched: ExA1"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    // condition if the key corresponding to the value of Vx is currently in the up position
    if (!keyboard[registers[Vx]])
    {
        pc = pc + 2;
    }
}
// Set Vx = delay timer value.
void chip8::Op_Fx07()
{
    cout<<"fetched: Fx07"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    registers[Vx] = delayTimer;
}
// Wait for a key press, store the value of the key in Vx.
// All execution stops until a key is pressed, then the value of that key is stored in Vx.
void chip8::Op_Fx0A()
{
    cout<<"fetched: Fx0A"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    bool keyPressed = false;
    for (int i = 0; i < 16; i++) {
        if (keyboard[i]) {
            registers[Vx] = i;
            keyPressed = true;
            break;
        }
    }
    if (!keyPressed) {
        pc -= 2; // Keep repeating this instruction until a key is pressed.
    }
}
// Set delay timer = Vx.
void chip8::Op_Fx15()
{
    cout<<"fetched: Fx15"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    delayTimer = registers[Vx];
}
// Set sound timer = Vx.
void chip8::Op_Fx18()
{
    cout<<"fetched: Fx18"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    soundTimer = registers[Vx];
}
// Set I = I + Vx.
void chip8::Op_Fx1E()
{
    cout<<"fetched: Fx1E"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    index = index + registers[Vx];
}
// ye bhi nahi aaraha hai bhai tu kr le
void chip8::Op_Fx29()
{
    cout<<"fetched: Fx29"<<endl;
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index = font_start_address + (5 * digit);
}
// Store BCD representation of Vx in memory locations I, I+1, and I+2.
void chip8::Op_Fx33()
{
    cout<<"fetched: Fx33"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    uint8_t Value = registers[Vx];
    memory[index + 2] = Value % 10;
    Value = Value / 10;

    memory[index + 1] = Value % 10;
    Value = Value / 10;

    memory[index] = Value % 10;
}
// Store registers V0 through Vx in memory starting at location I.
void chip8::Op_Fx55()
{
    cout<<"fetched: Fx55"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= Vx; i++)
    {
        memory[index + i] = registers[i];
    }
}
// Read registers V0 through Vx from memory starting at location I.
void chip8::Op_Fx65()
{
    cout<<"fetched: Fx65"<<endl;
    uint8_t Vx = (opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= Vx; i++)
    {
        registers[i] = memory[index + i];
    }
}
void chip8::cycle()
{

    // fetching opcode 8 bit from pc and 8 bit from pc+1
    opcode = (memory[pc] << 8) | memory[pc + 1];
    // increamenting pc for next instruction
    pc += 2;
    // executing instruction
    // by fetching opcode table
    ((*this).*(table[(opcode & 0xF000) >> 12]))();
    // setting delay time
    if (delayTimer > 0)
        --delayTimer;
    if (soundTimer > 0)
        --soundTimer;
}