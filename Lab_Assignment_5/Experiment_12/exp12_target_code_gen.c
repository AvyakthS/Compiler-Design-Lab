#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CODE 50
#define NUM_REG 4

typedef struct {
    char label[10];
    char op[10];
    char arg1[20];
    char arg2[20];
    char res[20];
} TAC;

TAC tac_list[MAX_CODE];
int tac_count = 0;

// Simple round-robin register allocator
int reg_turn = 0;
int allocate_reg() {
    int r = reg_turn;
    reg_turn = (reg_turn + 1) % NUM_REG;
    return r;
}

void get_branch_mnemonic(const char *relop, char *mnemonic) {
    if (strcmp(relop, "==") == 0) strcpy(mnemonic, "JE");
    else if (strcmp(relop, "!=") == 0) strcpy(mnemonic, "JNE");
    else if (strcmp(relop, "<") == 0)  strcpy(mnemonic, "JL");
    else if (strcmp(relop, "<=") == 0) strcpy(mnemonic, "JLE");
    else if (strcmp(relop, ">") == 0)  strcpy(mnemonic, "JG");
    else if (strcmp(relop, ">=") == 0) strcpy(mnemonic, "JGE");
    else strcpy(mnemonic, "JNE");
}

void generate_target_code() {
    printf("\n======================= TARGET ASSEMBLY CODE GENERATION =======================\n");
    printf("%-20s | %-25s | %-25s\n", "INPUT TAC", "ASSEMBLY INSTRUCTION", "ACTION / EXPLANATION");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < tac_count; i++) {
        TAC t = tac_list[i];

        // 1. Label definition
        if (strlen(t.label) > 0) {
            printf("%-20s | %-25s | %-25s\n", "", t.label, "Declare label target");
        }

        // 2. Unconditional Jump: goto L
        if (strcmp(t.op, "goto") == 0) {
            char asm_code[35];
            snprintf(asm_code, sizeof(asm_code), "JMP %s", t.arg1);
            printf("%-20s | %-25s | Direct jump to label\n", "goto", asm_code);
            continue;
        }

        // 3. Conditional Jump: if arg1 relop arg2 goto L
        if (strcmp(t.op, "if") == 0) {
            int r = allocate_reg();
            char tac_repr[30];
            snprintf(tac_repr, sizeof(tac_repr), "if %s %s %s goto %s", t.arg1, t.op, t.arg2, t.res);

            char move_cmd[35], cmp_cmd[35], jmp_cmd[35], jmp_mne[10];
            snprintf(move_cmd, sizeof(move_cmd), "MOV R%d, %s", r, t.arg1);
            snprintf(cmp_cmd, sizeof(cmp_cmd), "CMP R%d, %s", r, t.arg2);
            get_branch_mnemonic(t.op, jmp_mne);
            snprintf(jmp_cmd, sizeof(jmp_cmd), "%s %s", jmp_mne, t.res);

            printf("%-20s | %-25s | Load operand into register\n", tac_repr, move_cmd);
            printf("%-20s | %-25s | Compare register with operand\n", "", cmp_cmd);
            printf("%-20s | %-25s | Conditional branch on flag\n", "", jmp_cmd);
            continue;
        }

        // 4. Simple Copy Assignment: res = arg1
        if (strlen(t.op) == 0 || strcmp(t.op, "=") == 0) {
            int r = allocate_reg();
            char tac_repr[30];
            snprintf(tac_repr, sizeof(tac_repr), "%s = %s", t.res, t.arg1);

            char move1[35], move2[35];
            snprintf(move1, sizeof(move1), "MOV R%d, %s", r, t.arg1);
            snprintf(move2, sizeof(move2), "MOV %s, R%d", t.res, r);

            printf("%-20s | %-25s | Load source to register\n", tac_repr, move1);
            printf("%-20s | %-25s | Store register to target memory\n", "", move2);
            continue;
        }

        // 5. Binary Arithmetic: res = arg1 op arg2
        int r = allocate_reg();
        char tac_repr[30];
        snprintf(tac_repr, sizeof(tac_repr), "%s = %s %s %s", t.res, t.arg1, t.op, t.arg2);

        char move1[35], arith[35], move2[35];
        snprintf(move1, sizeof(move1), "MOV R%d, %s", r, t.arg1);

        char op_code[10];
        if (strcmp(t.op, "+") == 0) strcpy(op_code, "ADD");
        else if (strcmp(t.op, "-") == 0) strcpy(op_code, "SUB");
        else if (strcmp(t.op, "*") == 0) strcpy(op_code, "MUL");
        else if (strcmp(t.op, "/") == 0) strcpy(op_code, "DIV");
        else strcpy(op_code, "OP");

        snprintf(arith, sizeof(arith), "%s R%d, %s", op_code, r, t.arg2);
        snprintf(move2, sizeof(move2), "MOV %s, R%d", t.res, r);

        printf("%-20s | %-25s | Load first operand to register\n", tac_repr, move1);
        printf("%-20s | %-25s | Perform arithmetic operation\n", "", arith);
        printf("%-20s | %-25s | Store computation into memory\n", "", move2);
    }
    printf("================================================================================\n");
}

void load_preset_program() {
    tac_count = 0;

    // t1 = a + b
    strcpy(tac_list[tac_count].label, "");
    strcpy(tac_list[tac_count].op, "+");
    strcpy(tac_list[tac_count].arg1, "a");
    strcpy(tac_list[tac_count].arg2, "b");
    strcpy(tac_list[tac_count].res, "t1");
    tac_count++;

    // t2 = t1 * c
    strcpy(tac_list[tac_count].label, "");
    strcpy(tac_list[tac_count].op, "*");
    strcpy(tac_list[tac_count].arg1, "t1");
    strcpy(tac_list[tac_count].arg2, "c");
    strcpy(tac_list[tac_count].res, "t2");
    tac_count++;

    // x = t2
    strcpy(tac_list[tac_count].label, "");
    strcpy(tac_list[tac_count].op, "=");
    strcpy(tac_list[tac_count].arg1, "t2");
    strcpy(tac_list[tac_count].arg2, "");
    strcpy(tac_list[tac_count].res, "x");
    tac_count++;

    // if x > 10 goto L1
    strcpy(tac_list[tac_count].label, "");
    strcpy(tac_list[tac_count].op, ">");
    strcpy(tac_list[tac_count].arg1, "x");
    strcpy(tac_list[tac_count].arg2, "10");
    strcpy(tac_list[tac_count].res, "L1");
    tac_count++;

    // y = 0
    strcpy(tac_list[tac_count].label, "");
    strcpy(tac_list[tac_count].op, "=");
    strcpy(tac_list[tac_count].arg1, "0");
    strcpy(tac_list[tac_count].arg2, "");
    strcpy(tac_list[tac_count].res, "y");
    tac_count++;

    // goto L2
    strcpy(tac_list[tac_count].label, "");
    strcpy(tac_list[tac_count].op, "goto");
    strcpy(tac_list[tac_count].arg1, "L2");
    strcpy(tac_list[tac_count].arg2, "");
    strcpy(tac_list[tac_count].res, "");
    tac_count++;

    // L1: y = 1
    strcpy(tac_list[tac_count].label, "L1:");
    strcpy(tac_list[tac_count].op, "=");
    strcpy(tac_list[tac_count].arg1, "1");
    strcpy(tac_list[tac_count].arg2, "");
    strcpy(tac_list[tac_count].res, "y");
    tac_count++;

    // L2:
    strcpy(tac_list[tac_count].label, "L2:");
    strcpy(tac_list[tac_count].op, "");
    strcpy(tac_list[tac_count].arg1, "");
    strcpy(tac_list[tac_count].arg2, "");
    strcpy(tac_list[tac_count].res, "");
    tac_count++;
}

int main() {
    int choice;
    printf("=====================================================\n");
    printf("    EXPERIMENT 12: TARGET MACHINE CODE GENERATOR     \n");
    printf(" Target Architecture: Register Machine (R0, R1, R2, R3)\n");
    printf("=====================================================\n");

    printf("\nChoose Input Mode:\n");
    printf("1. Load standard TAC program with arithmetic and branches\n");
    printf("2. Input custom TAC lines interactively\n");
    printf("Enter choice (1 or 2): ");
    if (scanf("%d", &choice) != 1) choice = 1;

    if (choice == 2) {
        printf("Enter number of instructions: ");
        if (scanf("%d", &tac_count) != 1) return 0;
        printf("Enter TAC format: <res> = <arg1> <op> <arg2>\n");
        printf("(For copies: x = y - - ; For goto: - goto L1 - -)\n");
        for (int i = 0; i < tac_count; i++) {
            char eq[4];
            scanf("%s %s %s %s %s",
                  tac_list[i].res, eq,
                  tac_list[i].arg1, tac_list[i].op, tac_list[i].arg2);
            tac_list[i].label[0] = '\0';
            if (strcmp(tac_list[i].op, "-") == 0) tac_list[i].op[0] = '\0';
            if (strcmp(tac_list[i].arg2, "-") == 0) tac_list[i].arg2[0] = '\0';
        }
    } else {
        load_preset_program();
    }

    generate_target_code();
    return 0;
}
