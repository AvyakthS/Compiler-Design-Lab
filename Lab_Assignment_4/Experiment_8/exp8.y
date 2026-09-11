%{
#include <stdio.h>
#include <stdlib.h>

int yylex();
void yyerror(const char *s);
%}

%%
input:
    input line
    | /* empty */
    ;

line:
    expr '\n' {
        printf(">> RESULT: VALID (All brackets & parentheses are balanced)\n\n");
    }
    | '\n' {
        /* silently ignore empty lines */
    }
    | error '\n' {
        printf(">> RESULT: INVALID (Unbalanced or improperly nested brackets)\n\n");
        yyerrok;
    }
    ;

expr:
    '(' S ')' S
    | '[' S ']' S
    | '{' S '}' S
    ;

S:
    '(' S ')' S
    | '[' S ']' S
    | '{' S '}' S
    | /* empty */
    ;
%%

void yyerror(const char *s) {
    /* Errors are captured and reported via the error '\n' rule */
}

int main() {
    printf("===============================================================\n");
    printf("   EXP 8: BALANCED BRACKETS & PARENTHESES CHECKER (LEX/YACC)   \n");
    printf("   Supports: ( ), [ ], { } inside expressions or code lines    \n");
    printf("   Press Ctrl+C or Ctrl+D to exit                              \n");
    printf("===============================================================\n\n");
    printf("Enter statements/bracket sequences:\n");
    yyparse();
    return 0;
}
