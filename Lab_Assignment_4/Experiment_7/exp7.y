%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T_INT 1
#define T_FLOAT 2
#define T_CHAR 3
#define T_ERROR -1

struct Symbol {
    char name[32];
    int type;
} symTab[100];

int symCount = 0;
int current_decl_type = 0;

int lookup(const char *name) {
    for (int i = 0; i < symCount; i++) {
        if (strcmp(symTab[i].name, name) == 0)
            return symTab[i].type;
    }
    return 0; // 0 indicates undeclared
}

int insert(const char *name, int type) {
    if (lookup(name) != 0) return 0; // Redeclaration
    strcpy(symTab[symCount].name, name);
    symTab[symCount].type = type;
    symCount++;
    return 1;
}

const char* getTypeName(int type) {
    switch(type) {
        case T_INT: return "int";
        case T_FLOAT: return "float";
        case T_CHAR: return "char";
        default: return "error/unknown";
    }
}

int yylex();
void yyerror(const char *s);
%}

%union {
    int type;
    char id[32];
}

%token <type> INT FLOAT CHAR
%token <id> ID
%token INT_NUM FLOAT_NUM CHAR_CONST

%type <type> type expr

%left '+' '-'
%left '*' '/'

%%
program:
    program statement
    | /* empty */
    ;

statement:
    declaration
    | assignment
    | error ';' { yyerrok; }
    ;

declaration:
    type var_list ';'
    ;

type:
    INT   { current_decl_type = T_INT;   $$ = T_INT; }
    | FLOAT { current_decl_type = T_FLOAT; $$ = T_FLOAT; }
    | CHAR  { current_decl_type = T_CHAR;  $$ = T_CHAR; }
    ;

var_list:
    var_list ',' ID {
        if (!insert($3, current_decl_type)) {
            printf(">> [SEMANTIC ERROR] Redeclaration of identifier '%s'\n", $3);
        } else {
            printf(">> [SUCCESS] Declared '%s' as %s\n", $3, getTypeName(current_decl_type));
        }
    }
    | ID {
        if (!insert($1, current_decl_type)) {
            printf(">> [SEMANTIC ERROR] Redeclaration of identifier '%s'\n", $1);
        } else {
            printf(">> [SUCCESS] Declared '%s' as %s\n", $1, getTypeName(current_decl_type));
        }
    }
    ;

assignment:
    ID '=' expr ';' {
        int idType = lookup($1);
        if (idType == 0) {
            printf(">> [SEMANTIC ERROR] Variable '%s' used without prior declaration\n", $1);
        } else if ($3 == T_ERROR) {
            printf(">> [SEMANTIC ERROR] Assignment aborted due to expression type errors\n");
        } else if (idType != $3) {
            printf(">> [SEMANTIC ERROR] Type mismatch: Cannot assign '%s' expression to '%s' variable '%s'\n",
                   getTypeName($3), getTypeName(idType), $1);
        } else {
            printf(">> [SUCCESS] Valid assignment: '%s' = (%s value)\n", $1, getTypeName(idType));
        }
    }
    ;

expr:
    expr '+' expr {
        if ($1 == T_ERROR || $3 == T_ERROR) $$ = T_ERROR;
        else if ($1 != $3) {
            printf(">> [SEMANTIC ERROR] Type mismatch on '+': '%s' and '%s'\n",
                   getTypeName($1), getTypeName($3));
            $$ = T_ERROR;
        } else $$ = $1;
    }
    | expr '-' expr {
        if ($1 == T_ERROR || $3 == T_ERROR) $$ = T_ERROR;
        else if ($1 != $3) {
            printf(">> [SEMANTIC ERROR] Type mismatch on '-': '%s' and '%s'\n",
                   getTypeName($1), getTypeName($3));
            $$ = T_ERROR;
        } else $$ = $1;
    }
    | expr '*' expr {
        if ($1 == T_ERROR || $3 == T_ERROR) $$ = T_ERROR;
        else if ($1 != $3) {
            printf(">> [SEMANTIC ERROR] Type mismatch on '*': '%s' and '%s'\n",
                   getTypeName($1), getTypeName($3));
            $$ = T_ERROR;
        } else $$ = $1;
    }
    | expr '/' expr {
        if ($1 == T_ERROR || $3 == T_ERROR) $$ = T_ERROR;
        else if ($1 != $3) {
            printf(">> [SEMANTIC ERROR] Type mismatch on '/': '%s' and '%s'\n",
                   getTypeName($1), getTypeName($3));
            $$ = T_ERROR;
        } else $$ = $1;
    }
    | '(' expr ')' { $$ = $2; }
    | ID {
        int t = lookup($1);
        if (t == 0) {
            printf(">> [SEMANTIC ERROR] Identifier '%s' is undeclared\n", $1);
            $$ = T_ERROR;
        } else $$ = t;
    }
    | INT_NUM    { $$ = T_INT; }
    | FLOAT_NUM  { $$ = T_FLOAT; }
    | CHAR_CONST { $$ = T_CHAR; }
    ;
%%

void yyerror(const char *s) {
    printf(">> [SYNTAX ERROR] Invalid statement syntax\n");
}

int main() {
    printf("===================================================================\n");
    printf("      EXP 7: SEMANTIC ANALYSER TYPE CHECKER (LEX & YACC)           \n");
    printf(" Enter declarations and expressions ending with ';':               \n");
    printf(" Example:                                                          \n");
    printf("   int a, b;                                                       \n");
    printf("   float x;                                                        \n");
    printf("   a = b + 10;                                                     \n");
    printf("   a = x;                                                          \n");
    printf(" Press Ctrl+C or Ctrl+D to terminate                               \n");
    printf("===================================================================\n\n");
    yyparse();
    return 0;
}
