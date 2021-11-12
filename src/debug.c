#include "chip-8.h"
#define MAX_SIZE 500


// This will be a debugging executable for each function
int main(int argc, char** args) {

    char input;

    // Initializing the chip
    Chip_8* chip;
    chip = (Chip_8*)malloc(sizeof(Chip_8));
    initialize(chip);
    //printf("Size of chip: %d\n", sizeof(Chip_8));

    // Loading program
    uint16_t progSize;
    uint16_t instructions[MAX_SIZE];
    uint16_t currInstrcution = 0;
    FILE *f = fopen("test.txt", "r");

    while (fscanf(f, "%x", &instructions[currInstrcution]) > 0) {
        progSize++;
        currInstrcution++;
    }

    printf("Test loaded...\n\n");

    loadProgram(chip, instructions, progSize);


    printf("Entering emulation...\n\n");
    // Entering emulation loop
    while (1) {
        emulate(chip);
        printf("Hit enter to step\n");
        scanf("%c", &input);
    }
    printf("\n\nExiting emulation...\n");

    return 0;
}