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
void simulate(stateType *);
int convertNum(int);



int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    
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
    
    for (int i = 0; i < 65536; ++i){
        state.mem[i] = 0;
    }
    
    for (int i = 0; i < 8; ++i){
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
    simulate(&state);
    
    return(0);
}

void printState(stateType *statePtr)
{
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

void simulate(stateType * state){
    int run = 0, count = 0;
    int opcode, arg0, arg1, arg2, offsetField;
    
    while (!run){
        printState(state);

        opcode = (state->mem[state->pc] & 29360128) >> 22;
        arg0 = (state->mem[state->pc] & 3670016) >> 19;
        arg1 = (state->mem[state->pc] & 458752) >> 16;
        arg2 = (state->mem[state->pc] & 7) >> 0;
        offsetField = convertNum(state->mem[state->pc] & 65535) >> 0;
        
        // add
        if (opcode == 0){
            state->reg[arg2] = state->reg[arg0] + state->reg[arg1];
            ++state->pc;
        }
        // nand
        else if (opcode == 1){
            state->reg[arg2] = ~(state->reg[arg0] | state->reg[arg1]);
            ++state->pc;
        }
        // lw
        else if (opcode == 2){
            state->reg[arg1] = state->mem[state->reg[arg0] + offsetField];
            ++state->pc;
        }
        // sw
        else if (opcode == 3){
            state->mem[state->reg[arg0] + offsetField] = state->reg[arg1];
            ++state->pc;
        }
        // beq
        else if (opcode == 4){
            if (state->reg[arg0] == state->reg[arg1]){
                state->pc = state->pc + offsetField + 1;
            }
            else {
                ++state->pc;
            }
        }
        // jalr
        else if (opcode == 5){
            state->reg[arg1] = state->pc + 1;
            state->pc  = state->reg[arg0];
        }
        // halt
        else if (opcode == 6){
            run = 1;
            ++state->pc;
        }
        // noop
        else if (opcode == 7){
            ++state->pc;
        }
        ++count;
    }
    printf("machine halted\n");
    printf("total of %d instructions executed\n", count);
    printf("final state of machine:\n");
    printState(state);
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) ) {
        num -= (1<<16);
    }
    return(num);
}
