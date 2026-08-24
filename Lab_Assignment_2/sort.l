%{
#include <stdio.h>
#include <stdlib.h>
 
#define MAX_SIZE 100
 
double arr[MAX_SIZE];
int count = 0;
 
/* Standard Bubble Sort implementation in C */
void bubble_sort() {
    if (count == 0) {
        return; /* Nothing to sort */
    }
 
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (arr[j] > arr[j+1]) {
                /* Swap the elements */
                double temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
 
    /* Print the sorted array */
    printf("Sorted result: ");
    for (int i = 0; i < count; i++) {
        printf("%g ", arr[i]);
    }
    printf("\n");
}
 
/* Reset the array counter for the next input */
void reset() {
    count = 0;
    printf("Enter numbers separated by spaces: ");
}
%}
 
%%
 
-?[0-9]+(\.[0-9]+)? {
    /* When a number is found, add it to the array */
    if (count < MAX_SIZE) {
        arr[count] = atof(yytext);
        count++;
    } else {
        printf("Warning: Array full (max %d). Ignoring %s\n", MAX_SIZE, yytext);
    }
}
 
\n {
    /* When Enter is pressed, sort the array and reset */
    bubble_sort();
    reset();
}
 
[ \t] { /* Ignore spaces and tabs */ }
 
. {
    /* Catch letters or special characters */
    printf("Invalid character ignored: %s\n", yytext);
}
 
%%
 
int yywrap() {
    return 1;
}
 
int main() {
    printf("--- Lex Bubble Sorter ---\n");
    reset();
    yylex();
    return 0;
}
