#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

struct Labels{
    char label[7];
    int address;
};

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
void findDuplicates(struct Labels list[MAXLINELENGTH], int num_labels);

int add(char *arg0, char *arg1, char *arg2);
int nor(char *arg0, char *arg1, char *arg2);
int lw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);
int sw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);
int beq(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);
int jalr(char *arg0, char *arg1, char *arg2);
int halt();
int noop();
int fill(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);



int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
    arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int num_labels = 0;
    
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
    
    struct Labels list[MAXLINELENGTH];
    // first pass
    while ( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        if (strlen(label)) {
            list[num_labels].address = address;
            strcpy (list[num_labels].label, label);
            num_labels++;
        }
        address++;
    }
    
    findDuplicates(list, num_labels);
    
    // second pass
    rewind(inFilePtr);
    
    int val;
    address = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        if (!strcmp(opcode, "add")) {
            val = add(arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "nor")) {
            val = nor(arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "lw")) {
            val = lw(2, list, address, num_labels, arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "sw")) {
            val = sw(3, list, address, num_labels, arg0, arg1, arg2);
        }
        else if (!strcmp(opcode, "beq")) {
            val = beq(4, list, address, num_labels, arg0, arg1, arg2);
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
            val = fill(-1, list, address, num_labels, arg0, arg1, arg2);
        }
        else {
            printf("%s", opcode);
            printf("error: wrong label \n");
            exit(1);
        }
        ++address;
        fprintf(outFilePtr, "%d\n", val);
    }
    
    
    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
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

int
isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

void findDuplicates(struct Labels list[MAXLINELENGTH], int num_labels){
    for (int i = 0; i < num_labels; ++i){
        for (int j = i + 1; j < num_labels; ++j){
            if(!strcmp(list[i].label, list[j].label)){
                printf("duplicate label\n");
                exit(1);
            }
        }
    }
}

int findAddress (int type, char* arg0, struct Labels list[], int address, int num_labels){
    int tempAddress = -1;
    for (int i = 0; i < num_labels; ++i){
        if (!strcmp(list[i].label, arg0)){
            tempAddress = list[i].address;
            break;
        }
    }
    if (tempAddress == -1){
        printf("error in find address\n");
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

int lw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
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
        val += findAddress(2, arg2, list, address, num_labels);
    }
    return val;
}

int sw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
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
        val += findAddress(3, arg2, list, address, num_labels);
    }
    return val;
}

int beq(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
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
        val += findAddress(4, arg2, list, address, num_labels);
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

int fill(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
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
        arg0_val = findAddress(-1, arg0, list, address, num_labels);
    }
    val += arg0_val; 
    return val;
}






