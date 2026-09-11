#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROD 20
#define MAX_SYMBOLS 50
#define MAX_LEN 20

typedef struct {
    char lhs[MAX_LEN];
    char rhs[MAX_PROD][MAX_LEN];
    int rhs_count;
} Production;

Production prods[MAX_PROD];
int prod_count = 0;

char non_terminals[MAX_SYMBOLS][MAX_LEN];
int nt_count = 0;

char terminals[MAX_SYMBOLS][MAX_LEN];
int t_count = 0;

int first_set[MAX_SYMBOLS][MAX_SYMBOLS];  
int follow_set[MAX_SYMBOLS][MAX_SYMBOLS]; 

int get_nt_index(char *str) {
    for (int i = 0; i < nt_count; i++) {
        if (strcmp(non_terminals[i], str) == 0) return i;
    }
    return -1;
}

int get_t_index(char *str) {
    for (int i = 0; i < t_count; i++) {
        if (strcmp(terminals[i], str) == 0) return i;
    }
    return -1;
}

void add_nt(char *str) {
    if (get_nt_index(str) == -1 && strlen(str) > 0) {
        strcpy(non_terminals[nt_count++], str);
    }
}

void add_t(char *str) {
    if (get_nt_index(str) == -1 && get_t_index(str) == -1 && strlen(str) > 0) {
        strcpy(terminals[t_count++], str);
    }
}

int is_nt(char *str) {
    return get_nt_index(str) != -1;
}

int tokenize_rhs(char *rhs_raw, char tokens[MAX_PROD][MAX_LEN]) {
    int t_idx = 0;
    int i = 0;
    int len = strlen(rhs_raw);
    
    while (i < len) {
        if (rhs_raw[i] == 'i' && i + 1 < len && rhs_raw[i+1] == 'd') {
            strcpy(tokens[t_idx++], "id");
            i += 2;
        } else if (i + 1 < len && rhs_raw[i+1] == '\'') {
            tokens[t_idx][0] = rhs_raw[i];
            tokens[t_idx][1] = '\'';
            tokens[t_idx][2] = '\0';
            t_idx++;
            i += 2;
        } else {
            tokens[t_idx][0] = rhs_raw[i];
            tokens[t_idx][1] = '\0';
            t_idx++;
            i++;
        }
    }
    return t_idx;
}

void compute_first() {
    int change = 1;
    while (change) {
        change = 0;
        for (int i = 0; i < prod_count; i++) {
            int lhs_idx = get_nt_index(prods[i].lhs);
            
            for (int j = 0; j < prods[i].rhs_count; j++) {
                char tokens[MAX_PROD][MAX_LEN];
                int token_count = tokenize_rhs(prods[i].rhs[j], tokens);
                
                if (token_count == 1 && strcmp(tokens[0], "e") == 0) {
                    int eps_idx = get_t_index("e");
                    if (!first_set[lhs_idx][eps_idx]) {
                        first_set[lhs_idx][eps_idx] = 1;
                        change = 1;
                    }
                    continue;
                }
                
                int k = 0;
                int continue_loop = 1;
                while (k < token_count && continue_loop) {
                    continue_loop = 0;
                    char *sym = tokens[k];
                    
                    if (!is_nt(sym)) {
                        int t_idx = get_t_index(sym);
                        if (!first_set[lhs_idx][t_idx]) {
                            first_set[lhs_idx][t_idx] = 1;
                            change = 1;
                        }
                    } else {
                        int sym_nt_idx = get_nt_index(sym);
                        for (int t = 0; t < t_count; t++) {
                            if (strcmp(terminals[t], "e") != 0 && first_set[sym_nt_idx][t]) {
                                if (!first_set[lhs_idx][t]) {
                                    first_set[lhs_idx][t] = 1;
                                    change = 1;
                                }
                            }
                        }
                        if (first_set[sym_nt_idx][get_t_index("e")]) {
                            continue_loop = 1;
                        }
                    }
                    k++;
                }
                if (k == token_count && continue_loop) {
                    int eps_idx = get_t_index("e");
                    if (!first_set[lhs_idx][eps_idx]) {
                        first_set[lhs_idx][eps_idx] = 1;
                        change = 1;
                    }
                }
            }
        }
    }
}

void compute_follow() {
    follow_set[get_nt_index(prods[0].lhs)][get_t_index("$")] = 1;
    
    int change = 1;
    while (change) {
        change = 0;
        for (int i = 0; i < prod_count; i++) {
            int lhs_idx = get_nt_index(prods[i].lhs);
            
            for (int j = 0; j < prods[i].rhs_count; j++) {
                char tokens[MAX_PROD][MAX_LEN];
                int token_count = tokenize_rhs(prods[i].rhs[j], tokens);
                
                for (int k = 0; k < token_count; k++) {
                    char *sym = tokens[k];
                    if (!is_nt(sym)) continue;
                    int sym_nt_idx = get_nt_index(sym);
                    
                    int next_pos = k + 1;
                    int continue_loop = 1;
                    
                    while (next_pos < token_count && continue_loop) {
                        continue_loop = 0;
                        char *next_sym = tokens[next_pos];
                        
                        if (!is_nt(next_sym)) {
                            int t_idx = get_t_index(next_sym);
                            if (!follow_set[sym_nt_idx][t_idx]) {
                                follow_set[sym_nt_idx][t_idx] = 1;
                                change = 1;
                            }
                        } else {
                            int next_nt_idx = get_nt_index(next_sym);
                            for (int t = 0; t < t_count; t++) {
                                if (strcmp(terminals[t], "e") != 0 && first_set[next_nt_idx][t]) {
                                    if (!follow_set[sym_nt_idx][t]) {
                                        follow_set[sym_nt_idx][t] = 1;
                                        change = 1;
                                    }
                                }
                            }
                            if (first_set[next_nt_idx][get_t_index("e")]) {
                                continue_loop = 1;
                            }
                        }
                        next_pos++;
                    }
                    
                    if (next_pos == token_count && continue_loop) {
                        for (int t = 0; t < t_count; t++) {
                            if (follow_set[lhs_idx][t]) {
                                if (!follow_set[sym_nt_idx][t]) {
                                    follow_set[sym_nt_idx][t] = 1;
                                    change = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Custom sort function that handles special terminal priorities perfectly
int compare_terminals(const void *a, const void *b) {
    char *strA = (char *)a;
    char *strB = (char *)b;
    
    // Always force epsilon 'e' to the end of FIRST sets
    if (strcmp(strA, "e") == 0) return 1;
    if (strcmp(strB, "e") == 0) return -1;
    
    // Always force end-marker '$' to the end of FOLLOW sets
    if (strcmp(strA, "$") == 0) return 1;
    if (strcmp(strB, "$") == 0) return -1;
    
    return strcmp(strA, strB);
}

void print_set(int set_matrix[MAX_SYMBOLS][MAX_SYMBOLS], int nt_idx) {
    printf("{");
    
    // Create a temporary sorted array of matching items for the current row
    char matches[MAX_SYMBOLS][MAX_LEN];
    int m_count = 0;
    
    for (int t = 0; t < t_count; t++) {
        if (set_matrix[nt_idx][t]) {
            strcpy(matches[m_count++], terminals[t]);
        }
    }
    
    // Apply order sort
    qsort(matches, m_count, MAX_LEN, compare_terminals);
    
    for (int i = 0; i < m_count; i++) {
        printf("%s", matches[i]);
        if (i < m_count - 1) printf(",");
    }
    printf("}");
}

int main() {
    int n;
    printf("Enter the no. of production:");
    if (scanf("%d", &n) != 1) return 1;
    prod_count = n;

    char raw_line[MAX_LEN * 3];
    for (int i = 0; i < n; i++) {
        scanf("%s", raw_line);
        
        char *arrow = strstr(raw_line, "->");
        *arrow = '\0';
        strcpy(prods[i].lhs, raw_line);
        add_nt(prods[i].lhs);
        
        char *rhs_part = arrow + 2;
        char *token = strtok(rhs_part, "/");
        prods[i].rhs_count = 0;
        while (token != NULL) {
            strcpy(prods[i].rhs[prods[i].rhs_count++], token);
            token = strtok(NULL, "/");
        }
    }

    // Dynamic extraction phase for terminal matching sets
    for (int i = 0; i < prod_count; i++) {
        for (int j = 0; j < prods[i].rhs_count; j++) {
            char tokens[MAX_PROD][MAX_LEN];
            int token_count = tokenize_rhs(prods[i].rhs[j], tokens);
            for (int k = 0; k < token_count; k++) {
                if (strcmp(tokens[k], "e") != 0) {
                    add_t(tokens[k]);
                }
            }
        }
    }
    add_t("e");
    add_t("$");

    compute_first();
    compute_follow();

    printf("\nFirst\n");
    for (int i = 0; i < nt_count; i++) {
        printf("First(%s)=", non_terminals[i]);
        print_set(first_set, i);
        printf("\n");
    }

    printf("\nFollow\n");
    for (int i = 0; i < nt_count; i++) {
        printf("Follow(%s)=", non_terminals[i]);
        print_set(follow_set, i);
        printf("\n");
    }

    return 0;
}
