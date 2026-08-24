#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char stack[100];
int top = -1;
char input[100];
int ip = 0;

void push(char c) {
    stack[++top] = c;
    stack[top + 1] = '\0';
}

void pop() {
    if (top >= 0) {
        stack[top] = '\0';
        top--;
    }
}

void print_step(const char *action) {
    printf("%-20s %-20s %-25s\n", stack, &input[ip], action);
}

// Checks stack top for reducible patterns: id, E+E, E*E, (E)
int reduce() {
    // Check for "id"
    if (top >= 1 && stack[top - 1] == 'i' && stack[top] == 'd') {
        pop(); // remove 'd'
        pop(); // remove 'i'
        push('E');
        print_step("Reduce: E -> id");
        return 1;
    }
    // Check for "E+E"
    if (top >= 2 && stack[top - 2] == 'E' && stack[top - 1] == '+' && stack[top] == 'E') {
        pop(); // remove 'E'
        pop(); // remove '+'
        pop(); // remove 'E'
        push('E');
        print_step("Reduce: E -> E+E");
        return 1;
    }
    // Check for "E*E"
    if (top >= 2 && stack[top - 2] == 'E' && stack[top - 1] == '*' && stack[top] == 'E') {
        pop(); // remove 'E'
        pop(); // remove '*'
        pop(); // remove 'E'
        push('E');
        print_step("Reduce: E -> E*E");
        return 1;
    }
    // Check for "(E)"
    if (top >= 2 && stack[top - 2] == '(' && stack[top - 1] == 'E' && stack[top] == ')') {
        pop(); // remove ')'
        pop(); // remove 'E'
        pop(); // remove '('
        push('E');
        print_step("Reduce: E -> (E)");
        return 1;
    }
    return 0;
}

int main() {
    char raw_input[100];
    printf("=====================================================\n");
    printf("        EXPERIMENT 5: SHIFT-REDUCE PARSER            \n");
    printf(" Grammar:                                            \n");
    printf("   1. E -> E + E                                     \n");
    printf("   2. E -> E * E                                     \n");
    printf("   3. E -> (E)                                       \n");
    printf("   4. E -> id                                        \n");
    printf("=====================================================\n");
    printf("Enter input string (e.g., id+id*id, (id+id), id*id): ");
    if (scanf("%99s", raw_input) != 1) return 0;

    // Append end-marker '$'
    snprintf(input, sizeof(input), "%s$", raw_input);

    stack[0] = '\0';
    top = -1;
    ip = 0;

    printf("\n%-20s %-20s %-25s\n", "STACK", "INPUT BUFFER", "ACTION");
    printf("-----------------------------------------------------------------\n");

    while (1) {
        // Shift step if not at end of input
        if (input[ip] != '$') {
            if (input[ip] == 'i' && input[ip + 1] == 'd') {
                push(input[ip++]);
                push(input[ip++]);
                print_step("Shift (id)");
            } else {
                char ch = input[ip++];
                push(ch);
                char action[30];
                snprintf(action, sizeof(action), "Shift (%c)", ch);
                print_step(action);
            }
        }

        // Apply reductions greedily
        while (reduce());

        // Acceptance condition: Stack has only 'E' and input is at '$'
        if (top == 0 && stack[0] == 'E' && input[ip] == '$') {
            printf("%-20s %-20s %-25s\n", stack, "$", "ACCEPT");
            printf("\n>> SUCCESS: The string \"%s\" is VALID and ACCEPTED.\n", raw_input);
            break;
        }

        // Error / Rejection condition
        if (input[ip] == '$' && !(top == 0 && stack[0] == 'E')) {
            printf("\n>> ERROR: The string \"%s\" is INVALID and REJECTED.\n", raw_input);
            break;
        }
    }

    return 0;
}
