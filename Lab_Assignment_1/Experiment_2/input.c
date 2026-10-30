/* Header comment test
   spanning multiple lines to ensure
   line numbers stay synchronized */
int global_flag = 1;
char grade = 'A';
int buffer[10];
float grid[4][8];

// Function declaration & definition
int calculate_score() {
    int a = 10, b = 20, sum;
    float factor = 1.75;
    double high_precision = 88.5;

    // Array indexing usage (MUST NOT corrupt buffer/grid dimensions in symTable)
    buffer[0] = 50;
    grid[1][2] = 3.14;

    if (a <= b) {
        // Nested block: tests Level 2 local scope
        int temp_delta = 5;
        sum = a + b + temp_delta;
    } else {
        sum = a - b;
    }

    return sum;
}
