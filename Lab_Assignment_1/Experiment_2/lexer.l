%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
char currentType[20] = "";
char currentKind[20] = "variable"; 
int currentScopeLevel = 0;         // 0 = Global, 1 = Local/Function, 2+ = Nested blocks
int currentOffset = 1000;          // Simulated base memory address starting at 1000
int lineNum = 1;                   // Track current line number

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
%}

/* Rules Section */
%%

\n                          { lineNum++; } 
[ \t\r]+                    { /* Ignore spaces and tabs */ }

"//".*                      { printf("%-20s : %s\n", "COMMENT", yytext); } 
"/*"([^*]|\*+[^*/])*\*+"/"  { printf("%-20s : %s\n", "COMMENT", yytext); } 

    /* Keywords - Explicitly matched first */
"int"|"float"|"char"|"double"|"void" { 
    printf("%-20s : %s\n", "KEYWORD (TYPE)", yytext); 
    strcpy(currentType, yytext); 
}

"return"|"if"|"else"|"while"|"for"|"do"|"break"|"continue" {
    printf("%-20s : %s\n", "KEYWORD", yytext);
}

    /* MATCH FUNCTIONS ONLY using trailing context '/' */
    /* This matches an identifier only if it's followed by optional spaces and a '(' */
    /* It leaves the '(' in the stream to be matched by the separator rule later! */
[a-zA-Z_][a-zA-Z0-9_]*/[ \t]*\( {
    // Exclude conditional keywords from being treated as functions
    if (strcmp(yytext, "if") == 0 || strcmp(yytext, "while") == 0 || 
        strcmp(yytext, "for") == 0 || strcmp(yytext, "return") == 0) {
        printf("%-20s : %s\n", "KEYWORD", yytext);
    } else {
        printf("%-20s : %s\n", "IDENTIFIER (FUNC)", yytext);
        if (strcmp(currentType, "") != 0) {
            insertSymbol(yytext, currentType, "function", currentScopeLevel, lineNum, 4, "None");
        }
    }
}

    /* MATCH STANDARD VARIABLES */
[a-zA-Z_][a-zA-Z0-9_]* { 
    printf("%-20s : %s\n", "IDENTIFIER", yytext);
    
    if (strcmp(currentType, "") != 0) {
        int size = getDataTypeSize(currentType);
        insertSymbol(yytext, currentType, "variable", currentScopeLevel, lineNum, size, "");
    }
}

"["[0-9]+"]" {
    printf("%-20s : %s\n", "ARRAY DIMENSION", yytext);
    if (symCount > 0) {
        strcpy(symTable[symCount-1].dimensions, yytext);
        int bound = atoi(yytext + 1); 
        symTable[symCount-1].size *= bound;
        currentOffset += (symTable[symCount-1].size - symTable[symCount-1].size/bound); 
    }
}

[0-9]+          { printf("%-20s : %s\n", "INTEGER CONSTANT", yytext); }
[0-9]*\.[0-9]+  { printf("%-20s : %s\n", "FLOAT CONSTANT", yytext); }
"'"."'"         { printf("%-20s : %s\n", "CHAR CONSTANT", yytext); }

"="|"+"|"-"|"*"|"/"|"%"|">"|"<"|">="|"<="|"=="|"!=" {
    printf("%-20s : %s\n", "OPERATOR", yytext);
}

"{" {
    printf("%-20s : %s\n", "SEPARATOR", yytext);
    currentScopeLevel++; 
}

"}" {
    printf("%-20s : %s\n", "SEPARATOR", yytext);
    if(currentScopeLevel > 0) currentScopeLevel--; 
}

"("|")"|"," {
    printf("%-20s : %s\n", "SEPARATOR", yytext);
}

";" { 
    printf("%-20s : %s\n", "SEPARATOR", yytext); 
    strcpy(currentType, ""); 
}

. { printf("%-20s : %s\n", "UNKNOWN", yytext); }

%%

int yywrap() { return 1; }

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r"); 
        if (!file) {
            perror("Error opening file");
            return 1;
        }
        yyin = file;
    }

    printf("\n=== Q2: LEXICAL ANALYSIS OUTPUT ===\n");
    printf("%-20s   %s\n", "TOKEN TYPE", "VALUE");
    printf("----------------------------------------\n");
    
    yylex(); 

    printf("\n\n=== Q1: COMPLETE SYMBOL TABLE GENERATION ===\n");
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
