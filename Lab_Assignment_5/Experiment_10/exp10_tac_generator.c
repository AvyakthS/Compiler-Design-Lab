#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_QUAD 50

typedef struct {
    char op[10];
    char arg1[20];
    char arg2[20];
    char res[20];
    char label[10];
    char explanation[60];
} Quadruple;

Quadruple quads[MAX_QUAD];
int quad_count = 0;
int temp_count = 1;
int label_count = 1;

void new_temp(char *t) {
    snprintf(t, 20, "t%d", temp_count++);
}

void new_label(char *l) {
    snprintf(l, 10, "L%d", label_count++);
}

void emit(const char *label, const char *op, const char *arg1, const char *arg2, const char *res, const char *expl) {
    strncpy(quads[quad_count].label, label, 9);
    strncpy(quads[quad_count].op, op, 9);
    strncpy(quads[quad_count].arg1, arg1, 19);
    strncpy(quads[quad_count].arg2, arg2, 19);
    strncpy(quads[quad_count].res, res, 19);
    strncpy(quads[quad_count].explanation, expl, 59);
    quad_count++;
}

void generate_detailed_tac(int col_size, int elem_size) {
    quad_count = 0;
    temp_count = 1;
    label_count = 1;

    char str_col[10], str_w[10];
    snprintf(str_col, sizeof(str_col), "%d", col_size);
    snprintf(str_w, sizeof(str_w), "%d", elem_size);

    char L_start[10], L_body[10], L_exit[10];
    new_label(L_start); // L1: Loop Start
    new_label(L_body);  // L2: Loop Body
    new_label(L_exit);  // L3: Loop Exit

    char t1[20], t2[20], t3[20], t4[20], t5[20];
    char t6[20], t7[20], t8[20], t9[20], t10[20], t11[20], t12[20], t13[20];

    // --- CONDITION 1: i < max ---
    emit(L_start, "if_false", "i < max", "-", L_exit, "Short-circuit: exit if i >= max");

    // --- CONDITION 2: matrix[i][j + 1] != 0 ---
    new_temp(t1);
    emit("", "+", "j", "1", t1, "Column offset: j + 1");

    new_temp(t2);
    emit("", "*", "i", str_col, t2, "Row offset: i * num_columns");

    new_temp(t3);
    emit("", "+", t2, t1, t3, "Linear index: (i * C) + (j + 1)");

    new_temp(t4);
    emit("", "*", t3, str_w, t4, "Byte offset: index * elem_size");

    new_temp(t5);
    char matrix_access[30];
    snprintf(matrix_access, sizeof(matrix_access), "matrix[%s]", t4);
    emit("", "=", matrix_access, "-", t5, "Load matrix[i][j + 1]");

    char cond2[30];
    snprintf(cond2, sizeof(cond2), "%s != 0", t5);
    emit("", "if_false", cond2, "-", L_exit, "Short-circuit: exit if matrix[i][j+1] == 0");

    // --- BODY: matrix[i][j] = matrix[i][j] * (x + y) - z ---
    new_temp(t6);
    emit(L_body, "*", "i", str_col, t6, "LHS/RHS Row offset: i * num_columns");

    new_temp(t7);
    emit("", "+", t6, "j", t7, "Linear index: (i * C) + j");

    new_temp(t8);
    emit("", "*", t7, str_w, t8, "Byte offset for matrix[i][j]");

    new_temp(t9);
    snprintf(matrix_access, sizeof(matrix_access), "matrix[%s]", t8);
    emit("", "=", matrix_access, "-", t9, "Load value of matrix[i][j]");

    new_temp(t10);
    emit("", "+", "x", "y", t10, "Compute sub-expression (x + y)");

    new_temp(t11);
    emit("", "*", t9, t10, t11, "Multiply: matrix[i][j] * (x + y)");

    new_temp(t12);
    emit("", "-", t11, "z", t12, "Subtract: t11 - z");

    // Store into matrix[i][j]
    emit("", "[]=", t12, "-", matrix_access, "Store result into matrix[i][j]");

    // --- BODY: i = i + 2 ---
    new_temp(t13);
    emit("", "+", "i", "2", t13, "Increment i by 2");
    emit("", "=", t13, "-", "i", "Update variable i");

    // Loop jump
    emit("", "goto", L_start, "-", "-", "Repeat while-loop");

    // Exit
    emit(L_exit, "noop", "-", "-", "-", "Loop termination target");
}

void print_linear_tac() {
    printf("\n============================ LINEAR THREE ADDRESS CODE ============================\n");
    for (int i = 0; i < quad_count; i++) {
        if (strlen(quads[i].label) > 0) {
            printf("%-6s: ", quads[i].label);
        } else {
            printf("        ");
        }

        if (strcmp(quads[i].op, "if_false") == 0) {
            printf("if_false %-15s goto %s\n", quads[i].arg1, quads[i].res);
        } else if (strcmp(quads[i].op, "goto") == 0) {
            printf("goto %s\n", quads[i].arg1);
        } else if (strcmp(quads[i].op, "noop") == 0) {
            printf("end_loop\n");
        } else if (strcmp(quads[i].op, "=") == 0) {
            printf("%-6s = %s\n", quads[i].res, quads[i].arg1);
        } else if (strcmp(quads[i].op, "[]=") == 0) {
            printf("%-6s = %s\n", quads[i].res, quads[i].arg1);
        } else {
            printf("%-6s = %s %s %s\n", quads[i].res, quads[i].arg1, quads[i].op, quads[i].arg2);
        }
    }
    printf("===================================================================================\n");
}

void print_quadruples_table() {
    printf("\n=============================== QUADRUPLES TABLE ==================================\n");
    printf("%-5s | %-10s | %-12s | %-12s | %-15s\n", "INDEX", "OPERATOR", "ARGUMENT 1", "ARGUMENT 2", "RESULT");
    printf("-----------------------------------------------------------------------------------\n");
    for (int i = 0; i < quad_count; i++) {
        printf("(%-3d) | %-10s | %-12s | %-12s | %-15s\n",
               i, quads[i].op, quads[i].arg1, quads[i].arg2, quads[i].res);
    }
    printf("===================================================================================\n");
}

int main() {
    int col_size = 10;
    int elem_size = 4;

    printf("===================================================================\n");
    printf("      EXPERIMENT 10: THREE ADDRESS CODE (TAC) GENERATION          \n");
    printf(" Source Pseudocode:                                                \n");
    printf("   while (i < max && matrix[i][j + 1] != 0) {                     \n");
    printf("       matrix[i][j] = matrix[i][j] * (x + y) - z;                  \n");
    printf("       i = i + 2;                                                 \n");
    printf("   }                                                              \n");
    printf("===================================================================\n");

    printf("\nEnter total columns of 'matrix' (e.g., 10 or 20): ");
    if (scanf("%d", &col_size) != 1) col_size = 10;

    printf("Enter element size in bytes (e.g., 4 for int, 8 for double): ");
    if (scanf("%d", &elem_size) != 1) elem_size = 4;

    generate_detailed_tac(col_size, elem_size);
    print_linear_tac();
    print_quadruples_table();

    return 0;
}
