#include "chip-8.h"

void initialize(Chip_8* chip) {
    // Resetting registers and clearing memory
    
    // Programs to be run will start at memory location 0x200
    // therfore the program counter will start there
    chip->PC = 0x200;
    chip->opcode = 0x000;
    chip->I = 0x000;
    chip->stack_pointer = 0x000;

    // Clearing display
    for (int i = 0; i < DISP_ROW; i++) {
        for (int j = 0; j < DISP_COL; j++) {
            chip->display[i][j] = 0;
        }
    }

    // Clearing Stack
    for (int i = 0; i < STACK_MAX; i++) {
        chip->stack[i] = 0x00;
    }

    // Clearing registers
    for (int i = 0; i < V_REG_NUM; i++) {
        chip->V[i] = 0x00;
    }

    // Loading the font set into memory
    // Font will be loaded in memory from 0x50 and onwards
    for (int i = 0; i < FONT_SET_SIZE; i++) {
        chip->RAM[i + 0x50] = chip->font_set[i];
    }

    // Resetting timers
    // Timers are set to max values and will be decremented
    // at 60Hz
    chip->delayTimer = 255;
    chip->soundTimer = 255;

    return;
}

// May just implement this in the main executable
void loadProgram(Chip_8* chip, uint16_t buffer[], uint16_t bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        chip->RAM[i + 0x200] = buffer[i];
    }
    return;
}

/*
BIG NOTE: May have to increment PC by 2 with each individual instruction,
may try a more elegant solution later
*/

void emulate(Chip_8* chip) {

    // Fetch Opcode from memory, which is specified
    // by the location given by the PC

    // The first two bytes are shifted to the left by 8, and bitwise OR'd
    // witht the next two bytes, given us the full opcode
    chip->opcode = chip->RAM[chip->PC] << 8 | chip->RAM[chip->PC + 1];

    // Incrementing the PC
    chip->PC += 2;

    // Decode Opcode
    // In this step, we will consult the opcode table and see what
    // each opcode does, and then execute the opcode


    // This gets the most-significant hex bit (ex. 0xA2F0 & 0xF000 == 0xA000)
    switch (chip->opcode & 0xF000)
    {

    case 0x0000:
    {
        if (chip->opcode == 0x00E0) {
            // Clear the screen
            for (int i = 0; i < DISP_ROW; i++) {
                for (int j = 0; i < DISP_COL; j++) {
                    chip->display[i][j] = 0;
                }
            }
        }

        if (chip->opcode == 0x00EE) {
            // return from subroutine call
            
            // this is done by popping the last address from
            // the stack and assigning the PC that address
            chip->stack_pointer--;
            chip->PC = chip->stack[chip->stack_pointer];

        }
    }
        break;
    
    case 0x1000:
    {
        // jump to address xxx
        chip->PC = chip->opcode & 0x0FFF;

        /*
        NOTE: Whenever 0xABCD & 0x0FFF is performed, it will return
        0x0BCD, or the 12-least signficant bits 
        */
    }
        break;
    
    case 0x2000:
    {
        // jump to subroutine at address xxx

        // This saves the original instruction location for when
        // the subroutine needs to be exited by pushing it onto the stack
        chip->stack[chip->stack_pointer] = chip->PC;
        chip->stack_pointer++;
        chip->PC = chip->opcode & 0x0FFF;
    }
        break;

    case 0x3000:
    {
        // 3rxx: skip if register r == constant
        if (chip->V[(chip->opcode & 0x0F00) >> 8 ] == (chip->opcode & 0x00FF)) {
            // Incrementing by 2 skips the PC to the next instruction
            chip->PC += 2;
        }
    }
        break;

    case 0x4000:
    {
        // 4rxx: skip if register r != constant xx
        if (chip->V[(chip->opcode & 0x0F00) >> 8] != (chip->opcode & 0x00FF)) {
            chip->PC += 2;
        }
    }
        break;

    case 0x5000:
    {
        // 5ry0: skip if register r == register y
        if (chip->V[(chip->opcode & 0x0F00) >> 8] == chip->V[(chip->opcode & 0x00F0) >> 4]) {
            chip->PC += 2;
        }
    }
        break;
    
    case 0x6000:
    {
        // 6rxx: move constant to register r
        chip->V[(chip->opcode & 0x0F00) >> 8] = (chip->opcode & 0x00FF);
    }
        break;

    case 0x7000:
    {
        // 7rxx: add constant to register r
        chip->V[(chip->opcode & 0x0F00) >> 8] += (chip->opcode & 0x00FF);
    }
        break;

    case 0x8000:
    {

        switch (chip->opcode & 0x000F)
        {
        // 8ry0: move register vy intro vr
        case 0x0000:
        {
            chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x00F0) >> 4];
        }
            break;
        
        // 8ry1: vr = vr | vy
        case 0x0001:
        {
            chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] | chip->V[(chip->opcode & 0x00F0) >> 4];
        }
            break;
        
        // 8ry2: vr = vr & vy
        case 0x0002:
        {
            chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] & chip->V[(chip->opcode & 0x00F0) >> 4];
        }
            break;

        // 8ry3: vr = vr XOR vy
        case 0x0003:
        {
            chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 8] ^ chip->V[(chip->opcode & 0x00F0) >> 4];
        }
            break;

        // 8ry4: vr = vr + vy
        case 0x0004:
        {
            // VF is where the carry is placed whenever the operation overflows
            if (chip->V[(chip->opcode & 0x0F00) >> 8] + chip->V[(chip->opcode & 0x00F0) >> 4] > 255) {
                chip->V[0xF] = 1;
            } else {
                chip->V[0xF] = 0;
            }
            chip->V[(chip->opcode & 0x0F00) >> 8] += chip->V[(chip->opcode & 0x00F0) >> 4];
        }
            break;

        // 8ry5: vr = vr - vy
        case 0x0005:
        {
            // sets VF to 1 if lhs > rhs
            if ((chip->V[(chip->opcode & 0x0F00) >> 8]) > (chip->V[(chip->opcode & 0x00F0) >> 4])) {
                chip->V[0xF] = 1;
            } else {
                chip->V[0xF] = 0;
            }
            chip->V[(chip->opcode & 0x0F00) >> 8] -= chip->V[(chip->opcode & 0x00F0) >> 4];
        }
            break;

        // 8r06: shift vr to the right, bit 0 goes into vf
        case 0x0006:
        {
            // This gets the lsb, or bit 0, and puts it into register VF
            chip->V[0xF] = chip->V[(chip->opcode & 0x0F00) >> 8] & 0b1;
            // Then shift vr to the right
            chip->V[(chip->opcode & 0x0F00) >> 8] >> 1;
        }
            break;

        // 8ry7: vr = vy - vr
        case 0x0007:
        {
            // sets VF to 1 if lhs > rhs
            if ((chip->V[(chip->opcode & 0x0F00) >> 4]) > (chip->V[(chip->opcode & 0x00F0) >> 8])) {
                chip->V[0xF] = 1;
            } else {
                chip->V[0xF] = 0;
            }
            chip->V[(chip->opcode & 0x0F00) >> 8] = chip->V[(chip->opcode & 0x0F00) >> 4] - chip->V[(chip->opcode & 0x00F0) >> 8];
        }
            break;

        // 8r0e: shift vr to the left, put msb into vf
        case 0x000E:
        {
            chip->V[0xF] = (chip->V[(chip->opcode & 0x0F00) >> 8] & 0x80) >> 7;
            // then shifr vr to the left
            chip->V[(chip->opcode & 0x0F00) >> 8] << 1;
        }
            break;
        }
    }

        break;

    case 0x9000:
        // 9xy0: skip if register rx != ry
        if (chip->V[(chip->opcode & 0x0F00) >> 8] != chip->V[(chip->opcode & 0x00F0) >> 4]) {
            chip->PC += 2;
        }
        break;

    case 0xA000:
    {
        // Loads index register with XXX given by opcode
        chip->I = chip->opcode & 0x0FFF;
        break;
    }
    case 0xB000:
    {
        // jump to address xxx + register V0
        chip->PC = (chip->opcode & 0x0FFF) + chip->V[0];
        break;
    }
    case 0xC000:
    {
        // crxx: vr = random number less than or equal to xx

        // NOTE: May need to change this
        chip->V[(chip->opcode & 0x0F00) >> 8] = rand() % (chip->opcode & 0x00FF); 
        break;
    }
    case 0xD000:
    {
        // Drys: draws sprite at screen location rx, ry, of height s
        uint8_t x_coord = (chip->V[(chip->opcode & 0x0F00) >> 8] % DISP_COL);
        uint8_t y_coord = (chip->V[(chip->opcode & 0x00F0) >> 4] % DISP_ROW);
        uint8_t height = (chip->opcode & 0x000F);
        chip->V[0xF] = 0;

        // Drawing to display

        // XOR drawing is done, i.e. only flipping pixels, and if the XOR
        // evaluates to 0, a carry is added to VF

        for (int i = 0; i < height; i++) {
            uint8_t sprite_byte = chip->RAM[chip->I + i];
            for (uint8_t bit = 0b1; bit <= 0x255; bit << 1) {
                if ( ( (sprite_byte & bit) ^ chip->display[y_coord][x_coord] ) == 0) {
                    chip->display[y_coord][x_coord] = 0;
                    chip->V[0xF] = 1;
                } else {
                    chip->display[y_coord][x_coord] = 1;
                }

                // Stops drawing row if right edge of screen is reached
                if (x_coord == DISP_COL) {
                    break;
                }
                x_coord++;
            }
            y_coord++;
        }
    }

    
        break;

    case 0xE000:
    {
        /*
        NOTE: This is a tenative solution until SDL or some other
        input library can be implemented.
        */

        // Ex9E: skip one instruction if the key in vx is pressed
        if ((chip->opcode & 0x00FF) == 0x009E) {
            uint8_t key = chip->V[(chip->opcode & 0x0F00) >> 8] >> 4;
            if ( _kbhit() && ((uint8_t)(getch() - '0') == key)) {
                chip->PC += 2;
            }
        }



        // ExA1: skips one instruction if the key in vx is not pressed
        if ((chip->opcode & 0x00FF) == 0x00A1) {
            uint8_t key = chip->V[(chip->opcode & 0x0F00) >> 8] >> 4;
            if ( _kbhit() && ((uint8_t)(getch() - '0') != key)) {
                chip->PC += 2;
            }
        }

        break;
    }

    case 0xF000:
    {
        
        switch (chip->opcode & 0x00FF)
        {
        // Fx07: sets vx to the current value of the delay timer
        case 0x0007:
        {
            chip->V[(chip->opcode & 0x0F00) >> 8] = chip->delayTimer;
            break;
        }
        // FXOA: waits for an input and then puts the hexadecimal 
        // value in register vx
        case 0x000A:
        {
            uint8_t input;
            scanf("%X", &input);
            chip->V[(chip->opcode & 0x0F00) >> 8] = input;
            break;
        }
        // Fx15: sets the delay timer to the value in vx
        case 0x0015:
        {
            chip->delayTimer = chip->V[(chip->opcode & 0x0F00) >> 8];
            break;
        }
        // Fx18: sets the sound timer to the value in vx
        case 0x0018:
        {
            chip->soundTimer = chip->V[(chip->opcode & 0x0F00) >> 8];
            break;
        }
        // Fx1E: add value in vx to the index register I
        case 0x001E:
        {
            if ((chip->I + chip->V[(chip->opcode & 0x0F00) >> 8]) > 255) {
                chip->V[0xF] = 1;
            } else {
                chip->V[0xF] = 0;
            }
            chip->I += chip->V[(chip->opcode & 0x0F00) >> 8];
            break;
        }
        // Fr29: point I to sprite for hexadecimal character in vr
        case 0x0029:
        {
            uint16_t hexChar = chip->V[(chip->opcode & 0x0F00) >> 8];
            // This gets the left-most nibble of the hex character,
            // the one we want to display
            hexChar >> 4;

            switch (hexChar)
            {

            // The font set starts in RAM at address 0x50, so all the offsets will
            // be added to that
            case 0:
            {
                chip->I = chip->RAM[0x50 + ZERO_OFFSET];
            }
                break;
            case 1:
            {
                chip->I = chip->RAM[0x50 + ONE_OFFSET];
            }
                break;
            case 2:
            {
                chip->I = chip->RAM[0x50 + TWO_OFFSET];
            }
                break;
            case 3:
            {
                chip->I = chip->RAM[0x50 + THREE_OFFSET];
            }
                break;
            case 4:
            {
                chip->I = chip->RAM[0x50 + FOUR_OFFSET];
            }
                break;
            case 5:
            {
                chip->I = chip->RAM[0x50 + FIVE_OFFSET];
            }
                break;
            case 6:
            {
                chip->I = chip->RAM[0x50 + SIX_OFFSET];
            }
                break;
            case 7:
            {
                chip->I = chip->RAM[0x50 + SEVEN_OFFSET];
            }
                break;
            case 8:
            {
                chip->I = chip->RAM[0x50 + EIGHT_OFFSET];
            }
                break;
            case 9:
            {
                chip->I = chip->RAM[0x50 + NINE_OFFSET];
            }
                break;
            case 0xA:
            {
                chip->I = chip->RAM[0x50 + A_OFFSET];
            }
                break;
            case 0xB:
            {
                chip->I = chip->RAM[0x50 + B_OFFSET];
            }
                break;
            case 0xC:
            {
                chip->I = chip->RAM[0x50 + C_OFFSET];
            }
                break;
            case 0xD:
            {
                chip->I = chip->RAM[0x50 + D_OFFSET];
            }
                break;
            case 0xE:
            {
                chip->I = chip->RAM[0x50 + E_OFFSET];
            }
                break;
            case 0xF:
            {
                chip->I = chip->RAM[0x50 + F_OFFSET];
            }
                break;
            }
        }
            break;

        // Fr33: stores the bcd representation of vr at location
        // index register, index + 1, and index + 2
        case 0x0033:
        {
            uint8_t num = chip->V[(chip->opcode & 0x0F00) >> 8];
            for (int i = 0; i <= 2; i++) {
                chip->RAM[chip->I + i] = num % 10;
                num /= 10;
            }
        }
            break;

        // Fr55: store registers v0 through vr at location pointed to
        // by Index register and onwards (r is inclusive)
        case 0x0055:
        {
            for (int i = 0; i <= ( (chip->opcode & 0x0F00) >> 8); i++) {
                chip->RAM[chip->I + i] = chip->V[i];
            }
            break;
        }
        
        // Fr65: load registers v0 through vr with data in locations
        // pointed to by Index register and onward (r is inclusive)
        case 0x0065:
        {
            for (int i = 0; i <= ( (chip->opcode & 0x0F00) >> 8); i++) {
                chip->V[i] = chip->RAM[chip->I + i];
            }
            break;
        }
        
        }


        break;
    }

    default:
        printf("Unrecognized opcode: 0x%X\n", chip->opcode);
        break;
    }

    // Update timers
    if (chip->delayTimer > 0) {
        chip->delayTimer--;
    }
    
    if (chip->soundTimer > 0) {
        if (chip->soundTimer == 1) {
            printf("BEEP!\n");
        }
        chip->soundTimer--;
    }

    return;
}