#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ITEMS 30
#define MAX_STATES 30
#define MAX_PROD 10

typedef struct {
    char lhs;
    char rhs[10];
    int dot;
} Item;

typedef struct {
    Item items[MAX_ITEMS];
    int count;
} State;

typedef struct {
    char lhs;
    char rhs[10];
} Production;

// Strictly LR(0) Prefix Expression Grammar
Production prods[MAX_PROD] = {
    {'E', "+EE"}, // 0
    {'E', "*EE"}, // 1
    {'E', "(E)"}, // 2
    {'E', "i"}    // 3
};
int num_prods = 4;

State states[MAX_STATES];
int num_states = 0;

char terminals[] = {'+', '*', '(', ')', 'i', '$'};
int num_terminals = 6;
char non_terminals[] = {'E'};
int num_non_terminals = 1;

char action_type[MAX_STATES][10];
int action_val[MAX_STATES][10];
int goto_table[MAX_STATES][10];

int item_exists(State *s, Item it) {
    for (int i = 0; i < s->count; i++) {
        if (s->items[i].lhs == it.lhs &&
            strcmp(s->items[i].rhs, it.rhs) == 0 &&
            s->items[i].dot == it.dot)
            return 1;
    }
    return 0;
}

void add_item(State *s, Item it) {
    if (!item_exists(s, it)) {
        s->items[s->count++] = it;
    }
}

void closure(State *s) {
    int added = 1;
    while (added) {
        added = 0;
        int current_count = s->count;
        for (int i = 0; i < current_count; i++) {
            Item it = s->items[i];
            if (it.dot < (int)strlen(it.rhs)) {
                char symbol = it.rhs[it.dot];
                for (int j = 0; j < num_prods; j++) {
                    if (prods[j].lhs == symbol) {
                        Item new_it = {prods[j].lhs, "", 0};
                        strcpy(new_it.rhs, prods[j].rhs);
                        if (!item_exists(s, new_it)) {
                            add_item(s, new_it);
                            added = 1;
                        }
                    }
                }
            }
        }
    }
}

State find_goto(State *s, char symbol) {
    State res;
    res.count = 0;
    for (int i = 0; i < s->count; i++) {
        Item it = s->items[i];
        if (it.dot < (int)strlen(it.rhs) && it.rhs[it.dot] == symbol) {
            Item next_it = it;
            next_it.dot++;
            add_item(&res, next_it);
        }
    }
    if (res.count > 0) closure(&res);
    return res;
}

int find_state_index(State *s) {
    for (int i = 0; i < num_states; i++) {
        if (states[i].count == s->count) {
            int match = 1;
            for (int j = 0; j < s->count; j++) {
                if (!item_exists(&states[i], s->items[j])) {
                    match = 0;
                    break;
                }
            }
            if (match) return i;
        }
    }
    return -1;
}

int get_term_idx(char c) {
    for (int i = 0; i < num_terminals; i++)
        if (terminals[i] == c) return i;
    return -1;
}

int get_nterm_idx(char c) {
    for (int i = 0; i < num_non_terminals; i++)
        if (non_terminals[i] == c) return i;
    return -1;
}

void build_table() {
    State s0;
    s0.count = 0;
    Item init_item = {'Z', "E", 0}; // Z = E' (Augmented)
    add_item(&s0, init_item);
    closure(&s0);
    states[num_states++] = s0;

    for (int i = 0; i < num_states; i++) {
        // Shift transitions
        for (int t = 0; t < num_terminals - 1; t++) {
            char sym = terminals[t];
            State next_s = find_goto(&states[i], sym);
            if (next_s.count > 0) {
                int idx = find_state_index(&next_s);
                if (idx == -1) {
                    idx = num_states;
                    states[num_states++] = next_s;
                }
                action_type[i][t] = 's';
                action_val[i][t] = idx;
            }
        }

        // Goto transitions
        for (int nt = 0; nt < num_non_terminals; nt++) {
            char sym = non_terminals[nt];
            State next_s = find_goto(&states[i], sym);
            if (next_s.count > 0) {
                int idx = find_state_index(&next_s);
                if (idx == -1) {
                    idx = num_states;
                    states[num_states++] = next_s;
                }
                goto_table[i][nt] = idx;
            } else {
                goto_table[i][nt] = -1;
            }
        }

        // LR(0) Reduce and Accept transitions
        for (int j = 0; j < states[i].count; j++) {
            Item it = states[i].items[j];
            if (it.lhs == 'Z' && it.dot == (int)strlen(it.rhs)) {
                int dollar_idx = get_term_idx('$');
                action_type[i][dollar_idx] = 'a';
            } else if (it.dot == (int)strlen(it.rhs)) {
                int p_idx = -1;
                for (int k = 0; k < num_prods; k++) {
                    if (prods[k].lhs == it.lhs && strcmp(prods[k].rhs, it.rhs) == 0) {
                        p_idx = k;
                        break;
                    }
                }
                // LR(0): Place reduce across every terminal
                for (int t = 0; t < num_terminals; t++) {
                    action_type[i][t] = 'r';
                    action_val[i][t] = p_idx;
                }
            }
        }
    }
}

void print_table() {
    printf("\n=============================== LR(0) PARSING TABLE ===============================\n");
    printf("%-7s | %-35s | %-10s\n", "STATE", "ACTION", "GOTO");
    printf("        |");
    for (int i = 0; i < num_terminals; i++) printf(" %-5c", terminals[i]);
    printf(" |");
    for (int i = 0; i < num_non_terminals; i++) printf(" %-5c", non_terminals[i]);
    printf("\n------------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_states; i++) {
        printf("I%-6d |", i);
        for (int t = 0; t < num_terminals; t++) {
            if (action_type[i][t] == 's') {
                char buf[10];
                snprintf(buf, sizeof(buf), "s%d", action_val[i][t]);
                printf(" %-5s", buf);
            } else if (action_type[i][t] == 'r') {
                char buf[10];
                snprintf(buf, sizeof(buf), "r%d", action_val[i][t]);
                printf(" %-5s", buf);
            } else if (action_type[i][t] == 'a') {
                printf(" %-5s", "acc");
            } else {
                printf(" %-5s", ".");
            }
        }
        printf(" |");
        for (int nt = 0; nt < num_non_terminals; nt++) {
            if (goto_table[i][nt] != -1) {
                printf(" %-5d", goto_table[i][nt]);
            } else {
                printf(" %-5s", ".");
            }
        }
        printf("\n");
    }
    printf("====================================================================================\n");
}

void parse_string(const char *raw_str) {
    char input[100];
    snprintf(input, sizeof(input), "%s$", raw_str);

    int state_stack[100];
    char sym_stack[100];
    int top = 0;

    state_stack[0] = 0;
    sym_stack[0] = '$';
    sym_stack[1] = '\0';

    int ip = 0;

    printf("\n%-25s %-15s %-20s %-20s\n", "STATE STACK", "SYMBOL STACK", "INPUT", "ACTION");
    printf("------------------------------------------------------------------------------------\n");

    while (1) {
        int s = state_stack[top];
        char a = input[ip];
        int t_idx = get_term_idx(a);

        if (t_idx == -1) {
            printf("\n>> ERROR: Invalid token '%c' in input stream.\n", a);
            return;
        }

        char state_str[50] = "";
        for (int i = 0; i <= top; i++) {
            char buf[10];
            snprintf(buf, sizeof(buf), "%d ", state_stack[i]);
            strcat(state_str, buf);
        }

        char action_str[50] = "";
        char act = action_type[s][t_idx];
        int val = action_val[s][t_idx];

        if (act == 's') {
            snprintf(action_str, sizeof(action_str), "Shift -> I%d", val);
            printf("%-25s %-15s %-20s %-20s\n", state_str, sym_stack, &input[ip], action_str);
            top++;
            state_stack[top] = val;
            sym_stack[top] = a;
            sym_stack[top + 1] = '\0';
            ip++;
        } else if (act == 'r') {
            snprintf(action_str, sizeof(action_str), "Reduce: %c -> %s", prods[val].lhs, prods[val].rhs);
            printf("%-25s %-15s %-20s %-20s\n", state_str, sym_stack, &input[ip], action_str);
            int len = strlen(prods[val].rhs);
            top -= len;
            int prev_state = state_stack[top];
            char lhs = prods[val].lhs;
            int nt_idx = get_nterm_idx(lhs);
            int next_state = goto_table[prev_state][nt_idx];

            top++;
            state_stack[top] = next_state;
            sym_stack[top] = lhs;
            sym_stack[top + 1] = '\0';
        } else if (act == 'a') {
            printf("%-25s %-15s %-20s %-20s\n", state_str, sym_stack, &input[ip], "Accept");
            printf("\n>> SUCCESS: String \"%s\" successfully parsed and accepted!\n", raw_str);
            return;
        } else {
            printf("%-25s %-15s %-20s %-20s\n", state_str, sym_stack, &input[ip], "Error");
            printf("\n>> ERROR: Parsing failed. Syntax error at '%c'.\n", a);
            return;
        }
    }
}

int main() {
    printf("=====================================================\n");
    printf("     EXPERIMENT 6: LR(0) PREFIX EXPRESSION PARSER    \n");
    printf(" Grammar:                                            \n");
    printf("   0. E -> + E E                                     \n");
    printf("   1. E -> * E E                                     \n");
    printf("   2. E -> ( E )                                     \n");
    printf("   3. E -> i                                         \n");
    printf("=====================================================\n");

    build_table();
    print_table();

    char test_input[100];
    printf("\nEnter prefix expression to parse (e.g., +ii, *+iii, +*(i)ii): ");
    if (scanf("%99s", test_input) == 1) {
        parse_string(test_input);
    }
    return 0;
}
