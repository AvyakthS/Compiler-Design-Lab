%{
#include <stdio.h>
#include <stdlib.h>
 
double num1 = 0, num2 = 0;
char op = 0;
 
/* State tracker:
   0 = expect first number
   1 = expect operator
   2 = expect second number
   3 = calculation ready
*/
int state = 0;
 
/* Helper function to perform the math */
void calculate() {
    if (state != 3) {
        printf("Error: Incomplete expression. Please enter format: [Number] [Operator] [Number]\n");
        return;
    }
 
    switch(op) {
        case '+': printf("Result: %g\n", num1 + num2); break;
        case '-': printf("Result: %g\n", num1 - num2); break;
        case '*': printf("Result: %g\n", num1 * num2); break;
        case '/':
            if (num2 != 0.0) {
                printf("Result: %g\n", num1 / num2);
            } else {
                printf("Error: Division by zero\n");
            }
            break;
        case '%':
            printf("Result: %d\n", (int)num1 % (int)num2);
            break;
    }
}
 
/* Helper function to reset for the next calculation */
void reset() {
    num1 = 0;
    num2 = 0;
    op = 0;
    state = 0;
    printf("Enter calculation (e.g., 5 + 3): ");
}
%}
 
%%
 
[0-9]+(\.[0-9]+)? {
    if (state == 0) {
        num1 = atof(yytext);
        state = 1; /* Now look for an operator */
    }
    else if (state == 2) {
        num2 = atof(yytext);
        state = 3; /* Now ready to calculate */
    }
    else if (state == 3) {
        printf("Error: Only two numbers are allowed per calculation.\n");
    }
    else {
        printf("Error: Expected an operator, but got a number.\n");
    }
}
 
[+\-*/%] {
    if (state == 1) {
        op = yytext[0];
        state = 2; /* Now look for the second number */
    }
    else {
        printf("Error: Operator '%s' is out of place.\n", yytext);
    }
}
 
\n {
    /* When the user hits Enter, calculate and reset */
    if (state > 0) {
        calculate();
    }
    reset();
}
 
[ \t] { /* Ignore spaces and tabs */ }
 
. { printf("Invalid character: %s\n", yytext); }
 
%%
 
int yywrap() {
    return 1;
}
 
int main() {
    printf("--- Two-Number Lex Calculator ---\n");
    printf("Format: [Number] [Operator] [Number]\n");
    reset();
    yylex();
    return 0;
}
