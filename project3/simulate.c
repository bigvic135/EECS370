#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct {
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
    int cycles; /* number of cycles run so far */
} stateType;

int
convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit integer */
    if (num & (1 << 15) ) {
        num -= (1 << 16);
    }
    return num;
}

int
field0(int instruction)
{
    return( (instruction>>19) & 0x7);
}

int
field1(int instruction)
{
    return( (instruction>>16) & 0x7);
}

int
field2(int instruction)
{
    return(instruction & 0xFFFF);
}

int
opcode(int instruction)
{
    return(instruction>>22);
}

void
printInstruction(int instr)
{
    
    char opcodeString[10];
    
    if (opcode(instr) == ADD) {
        strcpy(opcodeString, "add");
    } else if (opcode(instr) == NOR) {
        strcpy(opcodeString, "nor");
    } else if (opcode(instr) == LW) {
        strcpy(opcodeString, "lw");
    } else if (opcode(instr) == SW) {
        strcpy(opcodeString, "sw");
    } else if (opcode(instr) == BEQ) {
        strcpy(opcodeString, "beq");
    } else if (opcode(instr) == JALR) {
        strcpy(opcodeString, "jalr");
    } else if (opcode(instr) == HALT) {
        strcpy(opcodeString, "halt");
    } else if (opcode(instr) == NOOP) {
        strcpy(opcodeString, "noop");
    } else {
        strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
           field2(instr));
}

void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);
    
    printf("\tdata memory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

///////////////////////////////IMPLEMENTATION BEGINS///////////////////////////////////////////////


int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    stateType newState;
    FILE *filePtr;
    
    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
    
    filePtr = fopen(argv[1], "r");
    
    if (filePtr == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }
    
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++)
    {
        
        if (sscanf(line, "%d", state.instrMem+state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        
        printf("memory[%d]=%d\n", state.numMemory, state.instrMem[state.numMemory]);
    }
    
    printf("%d memory words\n", state.numMemory);
    printf("\tinstruction memory:\n");
    
    for (int i = 0; i < state.numMemory; ++i){
        printf("\t\tinstrMem[ %d ] ", i);
        printInstruction(state.instrMem[i]);
    }
    
    
    //Initializing registers to and pc to 0
    for (int i = 0; i < NUMREGS; ++i){
        state.reg[i] = 0;
    }
    for (int i = 0; i < state.numMemory; ++i){
        state.dataMem[i] = state.instrMem[i];
    }
    
    
    // destinations needed for detect and forward
    int EXEM_dest = 0;
    int MEMWB_dest = 0;
    int WBEND_dest = 0;
    int regA_val = 0;
    int regB_val = 0;
    
    /*
    state.IFID.pcPlus1 = -12973480;
    state.IDEX.readRegA = 6;
    state.IDEX.readRegB = 1;
    state.IDEX.offset = 0;
    state.EXMEM.branchTarget = -12974332;
    state.EXMEM.aluResult = -14024712;
    state.EXMEM.readRegB = 12;
    state.MEMWB.writeData = -14040720;
    state.WBEND.writeData = -4262240;*/
    
    // initialize all pipeline registers to the noop instruction ( 0x1c00000)
    state.IFID.instr = NOOPINSTRUCTION;
    state.IDEX.instr = NOOPINSTRUCTION;
    state.EXMEM.instr = NOOPINSTRUCTION;
    state.MEMWB.instr = NOOPINSTRUCTION;
    state.WBEND.instr = NOOPINSTRUCTION;
    state.cycles = 0;
    state.pc = 0;
    
    int current_instr;
    while (1)
    {
        // prints complete state of machine
        printState(&state);
        
        /* check for halt */
        if (opcode(state.MEMWB.instr) == HALT) {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
        }
        // newState will be the stae of the machine at the end of the cycle
        newState = state;
        newState.cycles++;
        
        /* --------------------- IF stage --------------------- */
        // stores instruction bits and PC + 1 into IF/ID register
        newState.IFID.instr = state.instrMem[state.pc];
        current_instr = newState.IFID.instr;
        newState.IFID.pcPlus1 = state.pc + 1;
        newState.pc++;
        
        
        /* --------------------- ID stage --------------------- */
        // stores instruction bits and PC + 1 into ID/EX register
        newState.IDEX.instr = state.IFID.instr;
        current_instr = newState.IDEX.instr;
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        
        
        // state:       lw 0 1 2
        // newstate:   add 1 3 4  OR
        
        // state:       lw 0 1 2
        // newstate:   add 3 1 4
        // stall for lw followed by an immediate dependent instruction
        if (opcode(state.IDEX.instr) == LW && (field0(newState.IDEX.instr) == field1(state.IDEX.instr) || field1(newState.IDEX.instr) == field1(state.IDEX.instr)))
        {
            newState.IDEX.instr = NOOPINSTRUCTION;
            newState.pc = state.pc;
            newState.IFID = state.IFID;
        }
        // no lw data hazard
        else
        {
            // gets the values of regA and regB and stores into ID/EX register
            newState.IDEX.offset = convertNum(field2(state.IFID.instr));
            newState.IDEX.readRegA = state.reg[field0(state.IFID.instr)];
            newState.IDEX.readRegB = state.reg[field1(state.IFID.instr)];
        }
        /* --------------------- EX stage --------------------- */
        // stores instruction bits PC + 1 + offset to EX/MEM branch
        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
        current_instr = newState.EXMEM.instr;
        regA_val = state.IDEX.readRegA;
        regB_val = state.IDEX.readRegB;
        
        int current_regA = field0(newState.EXMEM.instr);
        int current_regB = field1(newState.EXMEM.instr);
        
        // WBEND_dest is field 1 if instruction is lw
        if (opcode(state.WBEND.instr) == LW)
        {
            WBEND_dest = field1(state.WBEND.instr);
        }
        // WBEND_dest is field2 if instruction in anything else
        else
        {
            WBEND_dest = field2(state.WBEND.instr);
        }
        // checks for data hazard and if there is forwards the correct value
        // three instructions away
        if (opcode(state.WBEND.instr) == ADD || opcode(state.WBEND.instr) == NOR || opcode(state.WBEND.instr) == LW)
        {
            // if WBEND_dest matches with regA get the new value of regA
            if (current_regA == WBEND_dest)
            {
                regA_val = state.WBEND.writeData;
            }
            
            // if WBEND_dest matches with regB get the new value of regB
            if (current_regB == WBEND_dest)
            {
                regB_val = state.WBEND.writeData;
            }
        }
        
        
        // assigns MEMWB_dest = regB if instruction is lw
        if (opcode(state.MEMWB.instr) == LW)
        {
            MEMWB_dest = field1(state.MEMWB.instr);
        }
        // assigns MEMWB_dest to field2 (third value)
        else
        {
            MEMWB_dest = field2(state.MEMWB.instr);
        }
        // checks for data hazard and if there is forwards the correct value
        // two instructions away
        if (opcode(state.MEMWB.instr) == ADD || opcode(state.MEMWB.instr) == NOR || opcode(state.MEMWB.instr) == LW)
        {
            // if destination of MEM/WB register == regA of new instruction use new WB value
            if (current_regA == MEMWB_dest)
            {
                regA_val = state.MEMWB.writeData;
            }
            
            // if destination of MEM/WB register == regB of new instruction use new WB value
            if (current_regB == MEMWB_dest)
            {
                regB_val = state.MEMWB.writeData;
            }
        }
        
        
        if (opcode(state.EXMEM.instr) == LW){
            EXEM_dest = field1(state.EXMEM.instr);
        }
        else {
            EXEM_dest = field2(state.EXMEM.instr);
        }
        if (opcode(state.EXMEM.instr) == ADD || opcode(state.EXMEM.instr) == NOR || opcode(state.EXMEM.instr) == LW)
        {
            //EXMEM -- RegA
            if (current_regA == EXEM_dest)
            {
                regA_val = state.EXMEM.aluResult;
            }
            //EXMEM -- RegB
            else if (current_regB == EXEM_dest)
            {
                regB_val = state.EXMEM.aluResult;
            }
        }
        
        
        // compute and store the ALU result depending on the operation
        if (opcode(current_instr) == ADD)
        {
            newState.EXMEM.aluResult = regA_val + regB_val;
        }
        else if (opcode(current_instr) == NOR)
        {
            newState.EXMEM.aluResult = ~(regA_val | regB_val);
        }
        else if (opcode(current_instr) == LW)
        {
            newState.EXMEM.aluResult = regA_val + state.IDEX.offset;
        }
        else if (opcode(current_instr) == SW)
        {
            newState.EXMEM.aluResult = regA_val + state.IDEX.offset;
        }
        else if (opcode(current_instr) == BEQ)
        {
            newState.EXMEM.aluResult = regA_val - regB_val;
        }
        
        if (opcode(current_instr) != NOOP){
            newState.EXMEM.readRegB = regB_val;
        }
        
        
        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr = state.EXMEM.instr;
        current_instr = newState.MEMWB.instr;
        
        if (opcode(current_instr) == LW)
        {
            newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
        }
        else if (opcode(current_instr) == SW)
        {
            newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
        }
        else if (opcode(current_instr) == BEQ)
        {
            // if branch actually taken discard the instructions we have made so far and reset pc counter
            if (state.EXMEM.aluResult == 0)
            {
                newState.pc = state.EXMEM.branchTarget;
                newState.IFID.instr = NOOPINSTRUCTION;
                newState.IDEX.instr = NOOPINSTRUCTION;
                newState.EXMEM.instr = NOOPINSTRUCTION;
            }
        }
        else if (opcode(current_instr) != NOOP && opcode(current_instr) != HALT)
        {
            newState.MEMWB.writeData = state.EXMEM.aluResult;
        }
        
        /* --------------------- WB stage --------------------- */
        newState.WBEND.instr = state.MEMWB.instr;
        current_instr = newState.WBEND.instr;
        newState.WBEND.writeData = state.MEMWB.writeData;
        
        if (opcode(state.MEMWB.instr) == ADD ||  opcode(state.MEMWB.instr) == NOR)
        {
            newState.reg[field2(current_instr)] = state.MEMWB.writeData;
        }
        
        if (opcode(state.MEMWB.instr) == LW)
        {
            newState.reg[field1(current_instr)] = state.MEMWB.writeData;
        }
        
        
        /* ------------------------- END ---------------------- */
        
        
        state = newState; /* this is the last statement before end of the loop.
                           It marks the end of the cycle and updates the
                           current state with the values calculated in this
                           cycle */
    }
}
