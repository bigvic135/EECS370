#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

typedef struct set{
    int data[256];
    int valid;
    int tag;
    int dirty;
    int ranged_loc;
    int level;
} Set;

typedef struct cache{
    Set blocks[256];
} Cache;


int numberOfSets;
int blockSizeInWords;
int blocksPerSet;
int count = 0;

int load(int addr, Cache *cache, stateType *state);
void store(int addr, int regB, Cache *cache, stateType *state);
int convertNum(int num);
void simulate(stateType state);
int findLRU(Cache cache);

enum actionType
{cacheToProcessor, processorToCache, memoryToCache, cacheToMemory,
    cacheToNowhere};
/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *     cacheToProcessor: reading data from the cache to the processor
 *     processorToCache: writing data from the processor to the cache
 *     memoryToCache: reading data from the memory to the cache
 *     cacheToMemory: evicting cache data by writing it to the memory
 *     cacheToNowhere: evicting cache data by throwing it away
 */
void
printAction(int address, int size, enum actionType type)
{
    printf("@@@ transferring word [%d-%d] ", address, address + size - 1);
    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    } else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    } else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    } else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    } else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    blockSizeInWords = atoi(argv[2]);
    numberOfSets = atoi(argv[3]);
    blocksPerSet = atoi(argv[4]);
    
    if (argc != 5) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
    
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }
    
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
    }
    
    simulate(state);
    
    return(0);
}

void simulate(stateType state){
    int run = 0;
    int opcode, regA, regB, destReg, offsetField, instr;
    // setting all tags to -1
    Cache *cache = (Cache*)malloc(numberOfSets * sizeof(Cache));
    for (int i = 0; i < numberOfSets; ++i){
        for (int j = 0; j < blocksPerSet; ++j){
            cache[i].blocks[j].tag = -1;
        }
    }
    
    while (!run){
        instr = load(state.pc, cache, &state);
        /*
        opcode = (state.mem[state.pc] & 29360128) >> 22;
        regA = (state.mem[state.pc] & 3670016) >> 19;
        regB = (state.mem[state.pc] & 458752) >> 16;
        destReg = (state.mem[state.pc] & 7) >> 0;
        offsetField = convertNum(state.mem[state.pc] & 65535) >> 0;*/
        
        opcode = (instr & 29360128) >> 22;
        regA = (instr >> 19) & 0x7;
        regB = (instr >> 16) &0x7;
        destReg = (instr & 0x7);
        offsetField = convertNum(instr & 0xFFFF);
        
        
        // add
        if (opcode == 0){
            state.reg[destReg] = state.reg[regA] + state.reg[regB];
            ++state.pc;
        }
        // nor
        else if (opcode == 1){
            state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
            ++state.pc;
        }
        // lw
        else if (opcode == 2){
            state.reg[regB] = load(state.reg[regA] + offsetField, cache, &state);
            ++state.pc;
        }
        // sw
        else if (opcode == 3){
            //state.mem[state.reg[regA] + offsetField] = state.reg[regB];
            store(state.reg[regA] + offsetField, regB, cache, &state);
            ++state.pc;
        }
        // beq
        else if (opcode == 4){
            if (state.reg[regA] == state.reg[regB]){
                state.pc = state.pc + offsetField + 1;
            }
            else {
                ++state.pc;
            }
        }
        // jalr
        else if (opcode == 5){
            state.reg[regB] = state.pc + 1;
            state.pc  = state.reg[regA];
        }
        // halt
        else if (opcode == 6){
            run = 1;
            ++state.pc;
        }
        // noop
        else if (opcode == 7){
            ++state.pc;
        }
    }
    free(cache);
}

int convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Linux integer */
    if (num & (1<<15) ) {
        num -= (1<<16);
    }
    return(num);
}

// 1 is true
int load(int value, Cache *set, stateType *state){ // value = pc
    int index_of_set = (value / blockSizeInWords) % numberOfSets;
    int tag = value / blockSizeInWords / numberOfSets; //
    int offset = value % blockSizeInWords; // will give you the index of the address you need in words array
    int mem_index = value - offset; // mem_index - mem_index + blockoffsize will give you the range of the addresses you want
    int mem_index_temp = mem_index;
    
    // set[index_of_set] determines what set
    // set[index_of_set].blocks determines what blocks
    // set[index_of_set].blocks[i].data contains the lines of data
    // search cache for tags
    for (int i = 0; i < blocksPerSet; ++i){
        // if found load data from cache to reg
        if (set[index_of_set].blocks[i].tag == tag){
            set[index_of_set].blocks[i].level = count;
            ++count;
            
            printAction(value, 1, cacheToProcessor);
            return set[index_of_set].blocks[i].data[offset];
        }
    }
    
    // if tags not found search for empty spots in cache
    for(int i = 0; i < blocksPerSet; ++i){
        // found empty spot
        if(!set[index_of_set].blocks[i].valid){
            
            // updating cache
            set[index_of_set].blocks[i].level = count;
            ++count;
            set[index_of_set].blocks[i].ranged_loc = mem_index;
            set[index_of_set].blocks[i].valid = 1;
            set[index_of_set].blocks[i].dirty = 0;
            set[index_of_set].blocks[i].tag = tag;
            
            // load data from memory into cache
            for(int j = 0; j < blockSizeInWords; ++j){
                set[index_of_set].blocks[i].data[j] = state->mem[mem_index_temp];
                ++mem_index_temp;
            }
            printAction(mem_index, blockSizeInWords, memoryToCache);
            
            // cache to registers
            printAction(value, 1, cacheToProcessor);
            return set[index_of_set].blocks[i].data[offset];
        }
    }
    
    int lru = 0;
    lru = findLRU(set[index_of_set]);
    
    // no empty spots so replace lru
    if(set[index_of_set].blocks[lru].dirty == 1){ // if dirty
        
        // writing back to mem and update cache
        printAction(set[index_of_set].blocks[lru].ranged_loc, blockSizeInWords, cacheToMemory);
        for(int i = 0; i < blockSizeInWords; ++i){
            state->mem[set[index_of_set].blocks[lru].ranged_loc] = set[index_of_set].blocks[lru].data[i];
            set[index_of_set].blocks[lru].ranged_loc++;
            set[index_of_set].blocks[lru].data[i] = state->mem[mem_index_temp];
            ++mem_index_temp;
        }
        
        set[index_of_set].blocks[lru].level = count;
        ++count;
        set[index_of_set].blocks[lru].ranged_loc = mem_index;
        set[index_of_set].blocks[lru].tag = tag;
        set[index_of_set].blocks[lru].dirty = 0;
        printAction(mem_index, blockSizeInWords, memoryToCache);
        
        // cache to registers
        printAction(value, 1, cacheToProcessor);
        return set[index_of_set].blocks[lru].data[offset];
    }
    // not dirty just update
    else {
        
        // updating the LRU in the cache
        // Something wrong here
        printAction(set[index_of_set].blocks[lru].ranged_loc, blockSizeInWords, cacheToNowhere); // evicting LRU
        for(int i = 0; i < blockSizeInWords; ++i){
            set[index_of_set].blocks[lru].data[i] = state->mem[mem_index_temp];
            ++mem_index_temp;
        }
        
        set[index_of_set].blocks[lru].level = count;
        ++count;
        set[index_of_set].blocks[lru].ranged_loc = mem_index;
        set[index_of_set].blocks[lru].valid = 1;
        set[index_of_set].blocks[lru].dirty = 0;
        set[index_of_set].blocks[lru].tag = tag;
        
        printAction(mem_index, blockSizeInWords, memoryToCache);
        printAction(value, 1, cacheToProcessor);
        return set[index_of_set].blocks[lru].data[offset];
    }
}

void store(int value, int regB, Cache *set, stateType *state){
    int index_of_set = (value / blockSizeInWords) % numberOfSets;
    int tag = value / blockSizeInWords / numberOfSets;
    int offset = value % blockSizeInWords; // will give you the index of the address you need in words array
    int mem_index = value - offset; // mem_index - mem_index + blockoffsize will give you the range of the addresses you want
    int mem_index_temp = mem_index;
    
    //search for tags
    for(int i = 0; i < blocksPerSet; ++i){
        // if hit overwrite the value
        if(set[index_of_set].blocks[i].tag == tag){
            //gotta make it dirty, overwrite value
            set[index_of_set].blocks[i].level = count;
            ++count;
            set[index_of_set].blocks[i].dirty = 1;
            set[index_of_set].blocks[i].data[offset] = state->reg[regB];
            
            printAction(value, 1, processorToCache);
            return;
        }
    }
    
    // no matching tags so search for empty spots
    for(int i = 0; i < blocksPerSet; ++i){
        // empty spot
        if(!set[index_of_set].blocks[i].valid){
            // write to cache
            set[index_of_set].blocks[i].level = count;
            ++count;
            set[index_of_set].blocks[i].ranged_loc = mem_index;
            set[index_of_set].blocks[i].valid = 1;
            set[index_of_set].blocks[i].dirty = 1;
            set[index_of_set].blocks[i].tag = tag;
            
            // read the other address excluding regB from memory
            for(int j = 0; j < blockSizeInWords; ++j){ // inserting words
                set[index_of_set].blocks[i].data[j] = state->mem[mem_index_temp];
                ++mem_index_temp;
            }
            
            set[index_of_set].blocks[i].data[offset] = state->reg[regB];
            
            printAction(mem_index, blockSizeInWords, memoryToCache);
            printAction(value, 1, processorToCache);
            return;
        }
    }
    int lru = 0;
    lru = findLRU(set[index_of_set]);
    
    // cant find any empty so replace LRU
    // if dirty write old value to memory and update cache line
    if(set[index_of_set].blocks[lru].dirty){
        
        // send old value
        printAction(set[index_of_set].blocks[lru].ranged_loc, blockSizeInWords, cacheToMemory);
        for(int i = 0; i < blockSizeInWords; ++i){
            state->mem[set[index_of_set].blocks[lru].ranged_loc] = set[index_of_set].blocks[lru].data[i];
            set[index_of_set].blocks[lru].ranged_loc++;
            set[index_of_set].blocks[lru].data[i] = state->mem[mem_index_temp];
            ++mem_index_temp;
        }
        
        // update cache
        set[index_of_set].blocks[lru].level = count;
        ++count;
        set[index_of_set].blocks[lru].ranged_loc = mem_index;
        set[index_of_set].blocks[lru].tag = tag;
        set[index_of_set].blocks[lru].dirty = 1;
        set[index_of_set].blocks[lru].data[offset] = state->reg[regB];
        
        printAction(mem_index, blockSizeInWords, memoryToCache);
        
        printAction(value, 1, processorToCache);
        
    }
    // if not dirty
    else {
        // update cache
        // SOMETHING WRONG HERE
        printAction(set[index_of_set].blocks[lru].ranged_loc, blockSizeInWords, cacheToNowhere);
        
        for(int i = 0; i < blockSizeInWords; ++i){
            set[index_of_set].blocks[lru].data[i] = state->mem[mem_index_temp];
            ++mem_index_temp;
        }
        
        set[index_of_set].blocks[lru].level = count;
        ++count;
        set[index_of_set].blocks[lru].ranged_loc = mem_index;
        set[index_of_set].blocks[lru].tag = tag;
        set[index_of_set].blocks[lru].dirty = 1;
        set[index_of_set].blocks[lru].data[offset] = state->reg[regB];
        
        printAction(mem_index, blockSizeInWords, memoryToCache);
        printAction(value, 1, processorToCache);
    }
}

int findLRU(Cache cache){
    int min = cache.blocks[0].level;
    int lru = 0;
    for (int i = 1; i < blocksPerSet; ++i){
        if (cache.blocks[i].level < min){
            min = cache.blocks[i].level;
            lru = i;
        }
    }
    return lru;
}

