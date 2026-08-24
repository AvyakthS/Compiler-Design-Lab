#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SYMBOLS 100

// --- Symbol Table Structural Definition ---
struct Symbol {
    int id;
    char name[50];       
    char type[20];       
    char kind[20];       
    char scope[30];      
    int address;
    int line_no;
    int size;
    char dimensions[20]; 
};

struct Symbol symTable[MAX_SYMBOLS];
int symCount = 0;

// Tracker state variables
int currentScopeLevel = 0;         // 0 = Global, 1 = Local/Function, 2+ = Nested blocks
int currentOffset = 1000;          // Simulated base memory address starting at 1000

// Helper to calculate data type size 
int getDataTypeSize(char *type) {
    if (strcmp(type, "int") == 0) return 4;
    if (strcmp(type, "float") == 0) return 4;
    if (strcmp(type, "char") == 0) return 1;
    if (strcmp(type, "double") == 0) return 8;
    return 0;
}

// Function to add/update identifier in Symbol Table
void insertSymbol(char *name, char *type, char *kind, int scopeLvl, int line, int size, char *dims) {
    char scopeStr[30];
    if (scopeLvl == 0) strcpy(scopeStr, "Global");
    else sprintf(scopeStr, "Local (Lvl %d)", scopeLvl);

    // Check if symbol already exists in the current scope
    for(int i = 0; i < symCount; i++) {
        if(strcmp(symTable[i].name, name) == 0 && strcmp(symTable[i].scope, scopeStr) == 0) {
            return; 
        }
    }
    
    if (symCount >= MAX_SYMBOLS) return;

    symTable[symCount].id = symCount + 1;
    strcpy(symTable[symCount].name, name);
    strcpy(symTable[symCount].type, (strlen(type) > 0) ? type : "N/A");
    strcpy(symTable[symCount].kind, kind);
    strcpy(symTable[symCount].scope, scopeStr);
    
    symTable[symCount].address = currentOffset;
    symTable[symCount].line_no = line;
    symTable[symCount].size = size;
    strcpy(symTable[symCount].dimensions, (strlen(dims) > 0) ? dims : "None");
    
    currentOffset += size;
    symCount++;
}

// Helper to check for C type keywords
int isTypeKeyword(char *word) {
    return (strcmp(word, "int") == 0 || strcmp(word, "float") == 0 || 
            strcmp(word, "char") == 0 || strcmp(word, "double") == 0 || 
            strcmp(word, "void") == 0);
}

// Helper to check for keywords that can be followed by a '(' but are NOT functions
int isControlKeyword(char *word) {
    return (strcmp(word, "if") == 0 || strcmp(word, "while") == 0 || 
            strcmp(word, "for") == 0 || strcmp(word, "return") == 0);
}

void generateSymbolTable(FILE *fp) {
    char line[256];
    int lineNum = 0;
    char currentType[20] = "";
    int inMultiLineComment = 0;

    while (fgets(line, sizeof(line), fp)) {
        lineNum++;
        int len = strlen(line);
        int i = 0;

        while (i < len) {
            // Handle multi-line comment exit
            if (inMultiLineComment) {
                if (line[i] == '*' && i + 1 < len && line[i + 1] == '/') {
                    inMultiLineComment = 0;
                    i += 2;
                } else {
                    i++;
                }
                continue;
            }

            // Skip spaces/tabs
            if (isspace(line[i])) {
                i++;
                continue;
            }

            // Handle Single-line and Multi-line comment entries
            if (line[i] == '/' && i + 1 < len) {
                if (line[i + 1] == '/') break; // Skip the rest of the line
                if (line[i + 1] == '*') {
                    inMultiLineComment = 1;
                    i += 2;
                    continue;
                }
            }

            // Track Scopes
            if (line[i] == '{') {
                currentScopeLevel++;
                i++;
                continue;
            }
            if (line[i] == '}') {
                if (currentScopeLevel > 0) currentScopeLevel--;
                i++;
                continue;
            }
            if (line[i] == ';') {
                strcpy(currentType, ""); // Reset active declaration token type context
                i++;
                continue;
            }

            // Extract alphanumeric words
            if (isalpha(line[i]) || line[i] == '_') {
                char buffer[50];
                int bufIdx = 0;
                while (i < len && (isalnum(line[i]) || line[i] == '_')) {
                    if (bufIdx < 49) buffer[bufIdx++] = line[i];
                    i++;
                }
                buffer[bufIdx] = '\0';

                if (isTypeKeyword(buffer)) {
                    strcpy(currentType, buffer);
                } 
                else if (strlen(currentType) > 0 && !isControlKeyword(buffer)) {
                    char kind[20] = "variable";
                    char dims[20] = "None";
                    int allocatedSize = getDataTypeSize(currentType);

                    // Peek explicitly ahead in the remaining characters on this line
                    int peek = i;
                    while (peek < len && isspace(line[peek])) {
                        peek++;
                    }

                    if (peek < len && line[peek] == '(') {
                        strcpy(kind, "function");
                        allocatedSize = 4; // Function base pointer footprint
                    } 
                    else if (peek < len && line[peek] == '[') {
                        int dIdx = 0;
                        while (peek < len && line[peek] != ']') {
                            if (dIdx < 19) dims[dIdx++] = line[peek];
                            peek++;
                        }
                        if (peek < len && line[peek] == ']') {
                            dims[dIdx++] = ']';
                        }
                        dims[dIdx] = '\0';

                        // Extract array size integer bound multiplier
                        int bound = 0;
                        for (int k = 0; dims[k] != '\0'; k++) {
                            if (isdigit(dims[k])) {
                                bound = bound * 10 + (dims[k] - '0');
                            }
                        }
                        if (bound > 0) {
                            allocatedSize *= bound;
                        }
                    }

                    insertSymbol(buffer, currentType, kind, currentScopeLevel, lineNum, allocatedSize, dims);
                }
            } else {
                i++; // Fallback increment for operator/separator symbols
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <source_file.c>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    generateSymbolTable(fp);
    fclose(fp);

    printf("\n=== Q1: COMPLETE SYMBOL TABLE GENERATION (STANDALONE C PROGRAM) ===\n");
    printf("-----------------------------------------------------------------------------------------------------------------------\n");
    printf("%-4s | %-12s | %-10s | %-10s | %-15s | %-10s | %-8s | %-6s | %-10s\n", 
           "ID", "NAME", "TYPE", "KIND", "SCOPE/LEVEL", "MEM ADDR", "LINE NO", "SIZE", "DIMENSIONS");
    printf("-----------------------------------------------------------------------------------------------------------------------\n");
    for(int i = 0; i < symCount; i++) {
        printf("%-4d | %-12s | %-10s | %-10s | %-15s | 0x%-8X | %-8d | %-6d | %-10s\n", 
               symTable[i].id, 
               symTable[i].name, 
               symTable[i].type, 
               symTable[i].kind, 
               symTable[i].scope, 
               symTable[i].address, 
               symTable[i].line_no, 
               symTable[i].size, 
               symTable[i].dimensions);
    }
    printf("-----------------------------------------------------------------------------------------------------------------------\n");

    return 0;
}
