/* EECS 370 LC-2K Instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

int
convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) ) {
        num -= (1<<16);
    }
    return(num);
}

int isAdd(int input){
    if(input >= 0 && input <= 4128775){
        return 1;
    }
    return 0;
}

int isNor(int input){
    if(input >= 4194304 && input <= 8323079){
        return 1;
    }
    return 0;
}

int isLw(int input){
    if(input >= 8388608 && input <= 12582911){
        return 1;
    }
    return 0;
}

int isSw(int input){
    if(input >= 12582912 && input <= 16777215){
        return 1;
    }
    return 0;
}

int isBeq(int input){
    if(input >= 16777216 && input <= 20971519){
        return 1;
    }
    return 0;
}

int isJalr(int input){
    if(input >= 20971520 && input <= 25100288){
        return 1;
    }
    return 0;
}

int isHalt(int input){
    if(input == 25165824){
        return 1;
    }
    return 0;
}

int isNoop(int input){
    if(input == 29360128){
        return 1;
    }
    return 0;
}

char *getOpcode(int input){
    char *opcode;
    if(isAdd(input)){
        opcode = "add";
        return opcode;
    } else if(isNor(input)){
        opcode = "nor";
        return opcode;
    } else if(isLw(input)){
        opcode = "lw";
        return opcode;
    } else if(isSw(input)){
        opcode = "sw";
        return opcode;
    } else if(isBeq(input)){
        opcode = "beq";
        return opcode;
    } else if(isJalr(input)){
        opcode = "jalr";
        return opcode;
    } else if(isHalt(input)){
        opcode = "halt";
        return opcode;
    } else if(isNoop(input)){
        opcode = "noop";
        return opcode;
    } else {
        opcode = "faulty";
        return opcode;
    }
}

int getRegA(int input){
    int regA = (input >> 19) & 0x7;
    return regA;
}

int getRegB(int input){
    int regB = (input >> 16) & 0x7;
    return regB;
}

int getDestReg(int input){
    int destReg = input & 0x7;
    return destReg;
}

int getOffsetField(int input){
    int offsetField = input & 0xFFFF;
    offsetField = convertNum(offsetField);
    return offsetField;
}

int main(int argc, char *argv[]) {
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    char *opcode;
    int regA, regB, offsetField, destReg, numInstructions = 0;
    
    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
    
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }
    
    state.pc = 0;
    
    for(int i = 0; i < NUMREGS; ++i){
        state.reg[i] = 0;
    }
    
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
         state.numMemory++) {
        
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    
    while(state.pc < NUMMEMORY){
        printState(&state);
        opcode = getOpcode(state.mem[state.pc]);
        regA = getRegA(state.mem[state.pc]);
        regB = getRegB(state.mem[state.pc]);
        destReg = getDestReg(state.mem[state.pc]);
        offsetField = getOffsetField(state.mem[state.pc]);
        
        if(!strcmp(opcode, "add")){
            state.reg[destReg] = state.reg[regA] + state.reg[regB];
            ++state.pc;
        } else if(!strcmp(opcode, "nor")){
            state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
            ++state.pc;
        } else if(!strcmp(opcode, "lw")){
            state.reg[regB] = state.mem[offsetField + state.reg[regA]];
            ++state.pc;
        } else if(!strcmp(opcode, "sw")){
            state.mem[offsetField + state.reg[regA]] = state.reg[regB];
            ++state.pc;
        } else if(!strcmp(opcode, "beq")){
            if(state.reg[regA] == state.reg[regB]){
                state.pc = state.pc + 1 + offsetField;
            } else {
                ++state.pc;
            }
        } else if(!strcmp(opcode, "jalr")){
            state.reg[regB] = state.pc + 1;
            state.pc = state.reg[regA];
        } else if(!strcmp(opcode, "halt")){
            state.pc = state.pc + 1;
            ++numInstructions;
            printf("machine halted\n");
            printf("total of %d instructions executed\n", numInstructions);
            printf("final state of the machine:\n");
            printState(&state);
            exit(0);
        } else if(!strcmp(opcode, "noop")){
            ++state.pc;
        }
        ++numInstructions;
    }
    return(0);
}

void printState(stateType *statePtr) {
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

