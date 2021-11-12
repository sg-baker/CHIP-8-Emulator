#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Constants
#define STACK_MAX 256
#define RAM_MAX 512
#define DISP_ROW 32
#define DISP_COL 64
#define FONT_SET_SIZE 80
#define V_REG_NUM 16
#define FONT_SIZE 5

// Addresses of font characters
#define ZERO_OFFSET 0
#define ONE_OFFSET 5
#define TWO_OFFSET 10
#define THREE_OFFSET 15
#define FOUR_OFFSET 20
#define FIVE_OFFSET 25
#define SIX_OFFSET 30
#define SEVEN_OFFSET 35
#define EIGHT_OFFSET 40
#define NINE_OFFSET 45
#define A_OFFSET 50
#define B_OFFSET 55
#define C_OFFSET 60
#define D_OFFSET 65
#define E_OFFSET 70
#define F_OFFSET 75

uint16_t font_set[FONT_SET_SIZE] = {
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

typedef struct Chip_8 {


    /* Memory Map
    0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    0x200-0xFFF - Program ROM and work RAM
    */
   
    // Display is 64x32
    bool display[DISP_ROW][DISP_COL];

    // RAM is 4096 bytes. So 512 * 8 bytes = 4096.
    uint64_t RAM[RAM_MAX];
    // Program Counter
    uint16_t PC;

    // Opcode
    uint16_t opcode;

    // General Purpose Registers (V0 - VF)
    uint8_t V[V_REG_NUM];

    // Index register
    uint16_t I;

    // Stack can only contain 16-bit addresses
    uint16_t stack[STACK_MAX];
    uint16_t stack_pointer;
    // max capacity of the stack is 256


    // Timers
    uint8_t delayTimer;
    uint8_t soundTimer;




} Chip_8;

// an emulator's main task is to fetch instructions, decode
// instructions, and execute instructions


// chip8 functions
void initialize(Chip_8* chip);

// May just implement this in the main executable
void loadProgram(Chip_8* chip, uint16_t buffer[], uint16_t bufferSize);

// This function contains the actual fetch-decode-execute cycle of the emulator
void emulate(Chip_8* chip);