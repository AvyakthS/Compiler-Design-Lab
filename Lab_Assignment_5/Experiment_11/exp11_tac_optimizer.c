#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INSTR 50

typedef struct {
    char res[16];
    char arg1[16];
    char op[8];
    char arg2[16];
    int is_eliminated;
} Instruction;

Instruction code[MAX_INSTR];
int n_instr = 0;

int is_number(const char *s) {
    if (s == NULL || *s == '\0') return 0;
    int i = 0;
    if (s[0] == '-' || s[0] == '+') i++;
    for (; s[i] != '\0'; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

int evaluate(int a, const char *op, int b) {
    if (strcmp(op, "+") == 0) return a + b;
    if (strcmp(op, "-") == 0) return a - b;
    if (strcmp(op, "*") == 0) return a * b;
    if (strcmp(op, "/") == 0 && b != 0) return a / b;
    return 0;
}

void constant_folding_and_propagation() {
    for (int i = 0; i < n_instr; i++) {
        if (code[i].is_eliminated) continue;

        // Propagate known constant values from earlier assignments
        for (int j = 0; j < i; j++) {
            if (!code[j].is_eliminated && strlen(code[j].op) == 0 && is_number(code[j].arg1)) {
                if (strcmp(code[i].arg1, code[j].res) == 0) {
                    strcpy(code[i].arg1, code[j].arg1);
                }
                if (strcmp(code[i].arg2, code[j].res) == 0) {
                    strcpy(code[i].arg2, code[j].arg1);
                }
            }
        }

        // Fold binary operations if both operands are constants
        if (strlen(code[i].op) > 0 && is_number(code[i].arg1) && is_number(code[i].arg2)) {
            int val1 = atoi(code[i].arg1);
            int val2 = atoi(code[i].arg2);
            int res_val = evaluate(val1, code[i].op, val2);
            snprintf(code[i].arg1, sizeof(code[i].arg1), "%d", res_val);
            code[i].op[0] = '\0';
            code[i].arg2[0] = '\0';
        }
    }
}

void common_subexpression_elimination() {
    for (int i = 0; i < n_instr; i++) {
        if (code[i].is_eliminated || strlen(code[i].op) == 0) continue;

        for (int j = 0; j < i; j++) {
            if (code[j].is_eliminated || strlen(code[j].op) == 0) continue;

            if (strcmp(code[i].op, code[j].op) == 0 &&
                strcmp(code[i].arg1, code[j].arg1) == 0 &&
                strcmp(code[i].arg2, code[j].arg2) == 0) {
                // Check if any operand was re-assigned between j and i
                int modified = 0;
                for (int k = j + 1; k < i; k++) {
                    if (strcmp(code[k].res, code[i].arg1) == 0 ||
                        strcmp(code[k].res, code[i].arg2) == 0) {
                        modified = 1;
                        break;
                    }
                }
                if (!modified) {
                    strcpy(code[i].arg1, code[j].res);
                    code[i].op[0] = '\0';
                    code[i].arg2[0] = '\0';
                    break;
                }
            }
        }
    }
}

void dead_code_elimination() {
    // A temporary (starting with 't') is dead if never used as an argument downstream
    for (int i = 0; i < n_instr; i++) {
        if (code[i].is_eliminated) continue;

        if (code[i].res[0] == 't') {
            int is_used = 0;
            for (int j = i + 1; j < n_instr; j++) {
                if (code[j].is_eliminated) continue;
                if (strcmp(code[j].arg1, code[i].res) == 0 ||
                    strcmp(code[j].arg2, code[i].res) == 0) {
                    is_used = 1;
                    break;
                }
            }
            if (!is_used) {
                code[i].is_eliminated = 1;
            }
        }
    }
}

void add_instruction(const char *res, const char *arg1, const char *op, const char *arg2) {
    strcpy(code[n_instr].res, res);
    strcpy(code[n_instr].arg1, arg1);
    strcpy(code[n_instr].op, op ? op : "");
    strcpy(code[n_instr].arg2, arg2 ? arg2 : "");
    code[n_instr].is_eliminated = 0;
    n_instr++;
}

void print_tac_state(const char *title) {
    printf("\n--- %s ---\n", title);
    printf("%-5s | %-25s\n", "LINE", "INSTRUCTION");
    printf("------------------------------------\n");
    int count = 1;
    for (int i = 0; i < n_instr; i++) {
        if (!code[i].is_eliminated) {
            if (strlen(code[i].op) > 0) {
                printf("%-5d | %s = %s %s %s\n", count++, code[i].res, code[i].arg1, code[i].op, code[i].arg2);
            } else {
                printf("%-5d | %s = %s\n", count++, code[i].res, code[i].arg1);
            }
        }
    }
}

void load_preset_tac() {
    n_instr = 0;
    add_instruction("t1", "4", "+", "6");     // Constant folding: t1 = 10
    add_instruction("t2", "t1", "*", "2");    // Constant propagation & folding: t2 = 20
    add_instruction("t3", "a", "+", "b");     // Original expression
    add_instruction("t4", "a", "+", "b");     // Common subexpression: t4 = t3
    add_instruction("t5", "t3", "+", "t4");   // Uses t3 and t4
    add_instruction("t6", "5", "*", "3");     // Unused dead code: t6 = 15
    add_instruction("ans", "t5", "+", "t2");  // Final output
}

int main() {
    int choice;
    printf("=====================================================\n");
    printf("      EXPERIMENT 11: TAC CODE OPTIMIZATION           \n");
    printf(" Optimizations Applied:                              \n");
    printf("   1. Constant Folding                               \n");
    printf("   2. Constant Propagation                           \n");
    printf("   3. Common Subexpression Elimination (CSE)         \n");
    printf("   4. Dead Code Elimination (DCE)                    \n");
    printf("=====================================================\n");

    printf("\nChoose Input Mode:\n");
    printf("1. Run built-in standard TAC benchmark\n");
    printf("2. Enter custom TAC statements manually\n");
    printf("Enter choice (1 or 2): ");
    if (scanf("%d", &choice) != 1) choice = 1;

    if (choice == 2) {
        printf("Enter number of statements: ");
        int count;
        if (scanf("%d", &count) != 1) return 0;
        n_instr = 0;
        printf("Enter statements in format: <res> = <arg1> [<op> <arg2>]\n");
        printf("(Use '-' if no op/arg2, e.g.: t1 = a + b  OR  x = 5 - -)\n");
        for (int i = 0; i < count; i++) {
            char r[16], eq[4], a1[16], op[8], a2[16];
            scanf("%s %s %s %s %s", r, eq, a1, op, a2);
            if (strcmp(op, "-") == 0) {
                add_instruction(r, a1, "", "");
            } else {
                add_instruction(r, a1, op, a2);
            }
        }
    } else {
        load_preset_tac();
    }

    print_tac_state("ORIGINAL THREE ADDRESS CODE");

    constant_folding_and_propagation();
    print_tac_state("AFTER CONSTANT FOLDING & PROPAGATION");

    common_subexpression_elimination();
    print_tac_state("AFTER COMMON SUBEXPRESSION ELIMINATION");

    dead_code_elimination();
    print_tac_state("FINAL OPTIMIZED CODE (AFTER DEAD CODE ELIMINATION)");

    return 0;
}
