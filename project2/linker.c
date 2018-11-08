#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSIZE 300
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
    char label[7];
    char location;
    int offset;
};

struct RelocationTableEntry {
    int offset;
    char inst[7];
    char label[7];
    int file;
};

struct FileData {
    int textSize;
    int dataSize;
    int symbolTableSize;
    int relocationTableSize;
    int textStartingLine; // in final executible
    int dataStartingLine; // in final executible
    int text[MAXSIZE];
    int data[MAXSIZE];
    SymbolTableEntry symbolTable[MAXSIZE];
    RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
    int text[MAXSIZE];
    int data[MAXSIZE];
    SymbolTableEntry     symTable[MAXSIZE];
    RelocationTableEntry relocTable[MAXSIZE];
    int textSize;
    int dataSize;
    int symTableSize;
    int relocTableSize;
};

void fixLocal(FileData *files, int);
int findOrigOffset(int);
int findSizes(struct FileData files[MAXFILES], int, int);
int isUpper(char *);
int findGlobals(FileData files[MAXFILES], int, int, char *);
void assignStarts(FileData files[MAXFILES], int);
int convert(FileData files[MAXFILES], int, int, int);

int totalSize = 0;

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    int i, j;
    
    if (argc <= 2) {
        printf("error: usage: %s <obj file> ... <output-exe-file>\n",
               argv[0]);
        exit(1);
    }
    
    outFileString = argv[argc - 1];
    
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }
    
    FileData files[MAXFILES];
    int textcount = 0;
    //Reads in all files and combines into master
    for (i = 0; i < argc - 2; i++) {
        inFileString = argv[i+1];
        
        inFilePtr = fopen(inFileString, "r");
        printf("opening %s\n", inFileString);
        
        if (inFilePtr == NULL) {
            printf("error in opening %s\n", inFileString);
            exit(1);
        }
        
        char line[MAXLINELENGTH];
        int sizeText, sizeData, sizeSymbol, sizeReloc;
        
        // parse first line
        fgets(line, MAXSIZE, inFilePtr);
        sscanf(line, "%d %d %d %d",
               &sizeText, &sizeData, &sizeSymbol, &sizeReloc);
        
        files[i].textSize = sizeText;
        files[i].dataSize = sizeData;
        files[i].symbolTableSize = sizeSymbol;
        files[i].relocationTableSize = sizeReloc;
        files[i].textStartingLine = textcount;

        totalSize += files[i].dataSize + files[i].textSize;
        
        // read in text
        int instr;
        for (j = 0; j < sizeText; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            instr = atoi(line);
            files[i].text[j] = instr;
            ++textcount;
        }
        
        // read in data
        int data;
        for (j = 0; j < sizeData; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            data = atoi(line);
            files[i].data[j] = data;
        }
        
        // read in the symbol table
        char label[7];
        char type;
        int addr;
        for (j = 0; j < sizeSymbol; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            sscanf(line, "%s %c %d",
                   label, &type, &addr);
            files[i].symbolTable[j].offset = addr;
            strcpy(files[i].symbolTable[j].label, label);
            files[i].symbolTable[j].location = type;
            
            if (!strcmp("Stack", label) && type != 'U'){
                printf("defined stack label\n");
                exit(1);
            }
            // maybe something wrong here
        }
        
        // read in relocation table
        char opcode[7];
        for (j = 0; j < sizeReloc; j++) {
            fgets(line, MAXLINELENGTH, inFilePtr);
            sscanf(line, "%d %s %s",
                   &addr, opcode, label);
            files[i].relocTable[j].offset = addr;
            strcpy(files[i].relocTable[j].inst, opcode);
            strcpy(files[i].relocTable[j].label, label);
            files[i].relocTable[j].file    = i;
        }
        fclose(inFilePtr);
    } // end reading files
    assignStarts(files, argc - 2);
    i = 0;
    // every file
    int count = 0;
    
    while (i < argc - 2){
        // check other files
        for (int j = i + 1; j < argc - 2; ++j){
            // something wrong
            for (int k = 0; k < files[j].symbolTableSize; ++k){
                if ((!strcmp(files[i].symbolTable[count].label, files[j].symbolTable[k].label)) && files[i].symbolTable[count].location != 'U' && files[j].symbolTable[k].location != 'U'){
                    printf("duplicate defined label\n");
                    exit(1);
                }
            }
        }
        ++count;
        // increment to next file
        if (count >= files[i].symbolTableSize){
            ++i;
            count = 0;
        }
    }
    
    fixLocal(files, argc);
    
    // error checking make a struct for every global label
    // store in an array of some size
    // if you come accross an undefined global increment by 1
    // if you come across same global label but defined set = to 0 and set the char to D
    
    // if you come across another same global label and defined exit with error
    
    // at the end exit with error if error != 0
    
    
    
    // go through again and look for duplicate defined global labels
    // look for undefined global labels (dont report Stack as undefined)
    // make sure stack is not defined in input object file
    
    // loop through how many files
    for (int i = 0; i < argc - 2; ++i){
        
        for (int j = 0; j < files[i].textSize; ++j){
            fprintf(outFilePtr, "%d\n", files[i].text[j]);
        }
    }
    for (int i = 0; i < argc - 2; ++i){
        for (int j = 0; j < files[i].dataSize; ++j){
            fprintf(outFilePtr, "%d\n", files[i].data[j]);
        }
    }
    // *** INSERT YOUR CODE BELOW ***
    //    Begin the linking process
    //    Happy coding!!!
    return(0);
} // end main

void fixLocal(FileData *files, int argc){
    for (int i = 0; i < argc - 2; ++i){
        for (int j = 0; j < files[i].relocationTableSize; ++j){
            
            // check if in text
            if ((!strcmp(files[i].relocTable[j].inst , "lw")) || (!strcmp(files[i].relocTable[j].inst , "sw"))){
                // if global label
                if (isUpper(files[i].relocTable[j].label)){
                    
                    // gives decimal value of instruction
                    int index = files[i].text[files[i].relocTable[j].offset];
                    
                    // check for stack
                    if (!strcmp(files[i].relocTable[j].label, "Stack")){
                        int temp = findGlobals(files, i, argc - 2, files[i].relocTable[j].label); // temp should = 0
                        files[i].text[files[i].relocTable[j].offset] = index + totalSize + temp;
                    }
                    else {
                        // finds new offset
                        int temp = findGlobals(files, i, argc - 2, files[i].relocTable[j].label);
                        
                        //int result = index + temp;
                        
                        // updates address
                        files[i].text[files[i].relocTable[j].offset] = convert(files, argc - 2, temp, index);
                    }
                }
                // local label
                else {
                    // gives decimal value of instruction
                    int index = files[i].text[files[i].relocTable[j].offset];
                    
                    // finds original offset
                    int temp = findOrigOffset(index);
                    
                    // finds sizes of other files
                    int size = findSizes(files, i, argc - 2); // find sizes function might be wrong
                    
                    // updates addresses
                    files[i].text[files[i].relocTable[j].offset] = convert(files, argc - 2, temp + size, index);
                }
                
            }
            // check if in data
            // This doesnt account for if label was defined in data section
            if ((!strcmp(files[i].relocTable[j].inst , ".fill"))){
                
                // if upper case label
                if (isUpper(files[i].relocTable[j].label)){
                    
                    if (!strcmp(files[i].relocTable[j].label, "Stack")){
                        int temp = findGlobals(files, i, argc - 2, files[i].relocTable[j].label);
                        files[i].data[files[i].relocTable[j].offset] = totalSize + temp;
                    }
                    
                    else {
                        // finds updated position in combined file (basically data starting position + original offset)
                        // think this is wrong
                        int temp = findGlobals(files, i, argc - 2, files[i].relocTable[j].label);
                        
                        files[i].data[files[i].relocTable[j].offset] = temp;
                    }
                }
                // lower case label
                else {
                    // gives you original location (location in input file)
                    int temp = files[i].data[files[i].relocTable[j].offset];
                    
                    // add only text size files that came before
                    // doesnt account for if the label was defined in the data section, might have to add previous files data
                    int sum = 0;
                    for (int k = i; k >= 0; --k){
                        if (k != i){
                            // if label was defined in data section
                            if (temp >= files[i].textSize){
                                sum += files[k].dataSize + files[k].textSize;
                            }
                            // if label was defined in text section
                            else {
                                sum += files[k].textSize;
                            }
                        }
                    }
                    sum += temp;
                    files[i].data[files[i].relocTable[j].offset] = sum;
                }
            }
        }
    }
    return;
}

int findOrigOffset(int input){
    char hexaDeciNum[100];
    
    // counter for hexadecimal number array
    int i = 0;
    while(input != 0)
    {
        // temporary variable to store remainder
        int temp  = 0;
        
        // storing remainder in temp variable.
        temp = input % 16;
        
        // check if temp < 10
        if(temp < 10)
        {
            hexaDeciNum[i] = temp + 48;
            i++;
        }
        else
        {
            hexaDeciNum[i] = temp + 55;
            i++;
        }
        
        input = input/16;
    }
    int last = hexaDeciNum[0] - '0';
    return last;
}

int findSizes(FileData files[MAXFILES], int exception, int argc){
    // dont have to add data sizes of files that come after FIX THIS!!!!!!!!
    int size = 0;
    for (int i = 0; i < argc; ++i){
        if (i != exception){
            if (exception == 0){
                size += files[i].textSize;
            }
            else {
                // if file come after exception dont add its data size
                if (i > exception){
                    size += files[i].textSize;
                }
                else {
                    size += files[i].dataSize + files[i].textSize;
                }
            }
        }
    }
    return size;
}

int isUpper(char *input){
    if (input[0] >= 'A' && input[0] <= 'Z'){
        return 1;
    }
    else {
        return 0;
    }
}

int findGlobals(FileData files[MAXFILES], int exception, int size, char* input){
    // finds location in combined file
    int temp = -1;
    for (int i = 0; i < size; ++i){
        for (int j = 0; j < files[i].symbolTableSize; ++j){
            if ((!strcmp(files[i].symbolTable[j].label , input))){
                // have to determine whether global label is defined in text or data and add corresponding size
                if (files[i].symbolTable[j].location == 'T'){
                    temp = files[i].symbolTable[j].offset + files[i].textStartingLine;
                }
                else if (files[i].symbolTable[j].location == 'D') {
                    temp = files[i].symbolTable[j].offset + files[i].dataStartingLine;
                }
                // come across another undefined stack global label
                else if (files[i].symbolTable[j].location == 'U' && !strcmp("Stack", input)){
                    temp = 0;
                }
            }
        }
    }
    // not found and its not stack
    if (temp == -1 && strcmp("Stack", input)){
        printf("undefined global label\n");
        exit(1);
    } // test case 7 has to do with something in the same file
    // stack label and you do not find label in symbols table
    if (temp == -1 && !strcmp("Stack", input)){
        temp = 0;
    }
    return temp;
    // rethink this logic for errors, dont think its right
}

void assignStarts(FileData files[MAXFILES], int size){
    // LOOK OVER THIS AGAIN
    int sum = 0;
    for (int i = 0; i < size; ++i){
        if (i == 0){
            sum += files[i].textSize;
        }
        else {
            sum += files[i].textSize;
        }
    }
    files[0].dataStartingLine = sum;
    for (int i = 1; i < size; ++i){
        sum += files[i - 1].dataSize;
        files[i].dataStartingLine = sum;
    }
}

int convert(FileData files[MAXFILES], int size, int input, int original){
    // array to store binary number
    int binaryNum[100];
    int origBinary[100];
    
    // convert new value
    int i = 0;
    while (input > 0) {
        
        // storing remainder in binary array
        binaryNum[i] = input % 2;
        input = input / 2;
        i++;
    }
    int j = 0;
    while (original > 0){
        origBinary[j] = original % 2;
        original = original / 2;
        ++j;
    }
    for (int k = 0; k < i; ++k){
        origBinary[k] = binaryNum[k];
    }
    int decimal = 0;
    for (int i = j - 1; i >= 0; --i){
        decimal = decimal << 1 | origBinary[i];
    }
    return decimal;
}


