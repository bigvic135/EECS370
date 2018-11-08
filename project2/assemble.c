//
//  assemble.c
//  project2
//
//  Created by BigVic on 2/7/18.
//  Copyright © 2018 mac. All rights reserved.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

struct Labels{
    char label[7];
    int address;
    char section;
    char opcode[6];
    int offset;
};

struct Labels local[MAXLINELENGTH];
struct Labels global[MAXLINELENGTH];
struct Labels undefined[MAXLINELENGTH];

int local_size = 0, global_size = 0, undef_size = 0;

int foundGlobal(char *);
int foundUndef(char *, struct Labels list[MAXLINELENGTH]);
int isNumber(char *);
int readAndParse(FILE *, char *, char *, char *, char *, char *);
void findDuplies(struct Labels list[MAXLINELENGTH], int num_labels);
int add(char *arg0, char *arg1, char *arg2);
int nor(char *arg0, char *arg1, char *arg2);
int lw(int type, int address, char *arg0, char *arg1, char *arg2);
int sw(int type, int address, char *arg0, char *arg1, char *arg2);
int beq(int type, int address, char *arg0, char *arg1, char *arg2);
int jalr(char *arg0, char *arg1, char *arg2);
int isUpper(char *input);
int halt(void);
int noop(void);
int fill(int type, int address, char *arg0, char *arg1, char *arg2);
int findAddress (int type, char* arg0, int address);

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
    arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    
    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }
    
    inFileString = argv[1];
    outFileString = argv[2];
    
    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }
    
    int address = 0;
    
    int text_size = 0, data_size = 0;
    
    // array for local labels, defined global labels, and undefined global labels
    // first pass
    int relocation_size = 0;
    char temp = 'T';
    int offset = 0;
    struct Labels reloTable[MAXLINELENGTH];
    //struct Labels local[MAXLINELENGTH];
    //struct Labels global[MAXLINELENGTH];
    //struct Labels undefined[MAXLINELENGTH];
    // rethink this tomrrow
    while ( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        if ((!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) &&
            
            
            !isNumber(arg2)){
            reloTable[relocation_size].offset = offset;
            strcpy(reloTable[relocation_size].opcode, opcode);
            strcpy(reloTable[relocation_size].label, arg2);
            ++relocation_size;
        }
        if ((!strcmp(opcode, ".fill"))){
            if (temp == 'T'){
                offset = 0;
                temp = 'D';
            }
            // not number
            if (!isNumber(arg0)){
                reloTable[relocation_size].offset = offset;
                strcpy(reloTable[relocation_size].opcode, opcode);
                strcpy(reloTable[relocation_size].label, arg0);
                ++relocation_size;
                if (isUpper(arg0) && !foundGlobal(arg0) && foundUndef(arg0, undefined) == -1){
                    undefined[undef_size].address = 0;
                    undefined[undef_size].section = temp;
                    strcpy (undefined[undef_size].opcode, opcode);
                    strcpy (undefined[undef_size].label, arg0);
                    ++undef_size;
                }
            }
            ++data_size;
        }
        // there is a label
        if (strlen(label)) {
            // if global label
            if (isUpper(label)){
                // if found in undefined get rid of and decrement undf size
                if (foundUndef(label, undefined) != -1){
                    int temp = foundUndef(label, undefined);
                    for (; temp < undef_size; ++temp){
                        undefined[temp] = undefined[temp + 1];
                    }
                    --undef_size;
                }
                global[global_size].address = address;
                global[global_size].section = temp;
                global[global_size].offset = offset;
                strcpy (global[global_size].opcode, opcode);
                strcpy (global[global_size].label, label);
                ++global_size;
            }
            // local label
            else {
                local[local_size].address = address;
                local[local_size].section = temp;
                local[local_size].offset = offset;
                strcpy (local[local_size].opcode, opcode);
                strcpy (local[local_size].label, label);
                ++local_size;
            }
        }
        // if undefined global symbolic address
        if (isUpper(arg2) && !foundGlobal(arg2) && foundUndef(arg2, undefined) == -1){
            undefined[undef_size].address = 0;
            undefined[undef_size].section = temp;
            strcpy (undefined[undef_size].opcode, opcode);
            strcpy (undefined[undef_size].label, arg2);
            ++undef_size;
        }
        if (strcmp(opcode, ".fill")){
            ++text_size;
        }
        ++offset;
        address++;
    }
    int symbolSize = global_size + undef_size;
    // global_size = symbol tabel
    
    findDuplies(local, local_size);
    findDuplies(global, global_size);
    //findDuplies(undefined, undef_size);
    
    // second pass
    rewind(inFilePtr);
    
    int val;
    address = 0;
    fprintf(outFilePtr, "%d ", text_size);
    fprintf(outFilePtr, "%d ", data_size);
    fprintf(outFilePtr, "%d ", symbolSize);
    fprintf(outFilePtr, "%d\n", relocation_size);
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        if (!strcmp(opcode, "add")) {
            val = add(arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "nor")) {
            val = nor(arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "lw")) {
                // lw and sw that will pass all data structures and sizes
                // make a find address function specifically for lw and sw
                // if arg is upper, search undefined and global
                    // if found in undefined set address equal to 0
                    // else do normal shit
                // if arg is lower search local
            val = lw(2, address, arg0, arg1, arg2);
                        // local
        }
        else if (!strcmp(opcode, "sw")) {
            val = sw(3, address, arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "beq")) {
            // still gotta do other shit for this one
            val = beq(4, address, arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "jalr")) {
            val = jalr(arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "halt")) {
            val = halt();
        }
        else if (!strcmp(opcode, "noop")) {
            val = noop();
        }
        else if (!strcmp(opcode, ".fill")) {
            val = fill(-1, address, arg0, arg1, arg2);
        }
        else {
            printf("%s", opcode);
            printf("error: wrong label \n");
            exit(1);
        }
        ++address;
        fprintf(outFilePtr, "%d\n", val);
    }
    // symbol data
    for (int i = 0; i < global_size; ++i){
        fprintf(outFilePtr, "%s ", global[i].label);
        fprintf(outFilePtr, "%c ", global[i].section);
        fprintf(outFilePtr, "%d\n", global[i].offset);
    }
    
    //symbol data
    for (int i = 0; i < undef_size; ++i){
        fprintf(outFilePtr, "%s ", undefined[i].label);
        fprintf(outFilePtr, "%c ", 'U');
        fprintf(outFilePtr, "%d\n", 0);
    }
    
    for (int i = 0; i < relocation_size; ++i){
        fprintf(outFilePtr, "%d ", reloTable[i].offset);
        fprintf(outFilePtr, "%s ", reloTable[i].opcode);
        fprintf(outFilePtr, "%s\n", reloTable[i].label);
    }
    
    
    return(0);
}

int isUpper(char *input){
    if (input[0] >= 'A' && input[0] <= 'Z'){
        return 1;
    }
    else {
        return 0;
    }
}

int foundGlobal(char *input){
    // 1 means found
    for (int i = 0; i < global_size; ++i){
        if (!strcmp(global[i].label, input)){
            return 1;
        }
    }
    return 0;
}

int foundUndef(char *input, struct Labels undefined[MAXLINELENGTH]){
    for (int i = 0; i < undef_size; ++i){
        
        if (!strcmp(undefined[i].label, input)){
            return i;
        }
    }
    return -1;
}


int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
             char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
    
    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }
    
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }
    
    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }
    
    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);
    return(1);
}
                
int isNumber(char *string){
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

void findDuplies(struct Labels list[MAXLINELENGTH], int num_labels){
    for (int i = 0; i < num_labels; ++i){
        for (int j = i + 1; j < num_labels; ++j){
            if(!strcmp(list[i].label, list[j].label)){
                printf("duplie label\n");
                exit(1);
            }
        }
    }
}

int findAddress (int type, char* arg0, int address){
    int tempAddress = -1;
    if (isUpper(arg0)){
        // check undefined
        for (int i = 0; i < undef_size; ++i){
            if (!strcmp(undefined[i].label, arg0)){
                if (type == 4){
                    printf("beq found undefined address\n");
                    exit(1);
                }
                else {
                    tempAddress = 0;
                    break;
                }
            }
        }
        // check global
        for (int i = 0; i < global_size; ++i){
            if (!strcmp(global[i].label, arg0)){
                tempAddress = global[i].address;
                break;
            }
        }
    }
    else {
        // check local
        for (int i = 0; i < local_size; ++i){
            if (!strcmp(local[i].label, arg0)){
                tempAddress = local[i].address;
                break;
            }
        }
    }
    // could be problem
    if (tempAddress == -1){
        printf("error in find label\n");
        exit(1);
    }
    if (type == 2 || type == 3) {
        tempAddress &= 65535;
    }
    else if (type == 4) {
        tempAddress = (65535 & (tempAddress - address - 1));
    }
    return tempAddress;
}

int add(char *arg0, char *arg1, char *arg2){
    unsigned int arg0_val;
    unsigned int arg1_val;
    unsigned int arg2_val;
    int val = 0;
    
    val += (0 << 22);
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    arg2_val = atoi(arg2);
    val = val + (arg2_val);
    return val;
}

int nor(char *arg0, char *arg1, char *arg2){
    unsigned int arg0_val;
    unsigned int arg1_val;
    unsigned int arg2_val;
    int val = 0;
    
    val += (1 << 22);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    arg2_val = atoi(arg2);
    val = val + (arg2_val);
    
    return val;
}

int lw(int type, int address, char *arg0, char *arg1, char *arg2){
    int arg0_val;
    int arg1_val;
    int arg2_val;
    int val = 0;
    
    val += (1 << 23);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    if (isNumber(arg2)){
        arg2_val = atoi(arg2);
        if (arg2_val <= -32769 || arg2_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
        val += arg2_val & 0xFFFF;
    }
    // not number
    else {
        val += findAddress(2, arg2, address);
        // find address for every type of address
    }
    return val;
}

int sw(int type, int address, char *arg0, char *arg1, char *arg2){
    int arg0_val;
    int arg1_val;
    int arg2_val;
    int val = 0;
    
    val += (3 << 22);
    
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    if (isNumber(arg2)){
        arg2_val = atoi(arg2);
        if (arg2_val <= -32769 || arg2_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
        val += arg2_val & 0xFFFF;
    }
    else {
        val += findAddress(3, arg2, address);
    }
    return val;
}

int beq(int type, int address, char *arg0, char *arg1, char *arg2){
    signed int arg0_val;
    signed int arg1_val;
    signed int arg2_val;
    int val = 0;
    
    val += (4 << 22);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    if (isNumber(arg2)){
        arg2_val = atoi(arg2);
        if (arg2_val <= -32769 || arg2_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
        val += arg2_val & 0xFFFF;
    }
    else {
        val += findAddress(4, arg2, address);
    }
    return val;
}

int jalr(char *arg0, char *arg1, char *arg2){
    int arg0_val;
    int arg1_val;
    int val = 0;
    
    val += (5 << 22);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    return val;
}

int halt(){
    int val = 0;
    
    val += (6 << 22);
    
    return val;
}

int noop(){
    int val = 0;
    
    val += (7 << 22);
    
    return val;
}

int fill(int type, int address, char *arg0, char *arg1, char *arg2){
    int val = 0;
    int arg0_val = 0;
    if (isNumber(arg0)){
        arg0_val = atoi(arg0);
        if (arg0_val <= -32769 || arg0_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
    }
    else {
        arg0_val = findAddress(-1, arg0, address);
    }
    val += arg0_val;
    return val;
}

                
