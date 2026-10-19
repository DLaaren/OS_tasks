#include <stdio.h>

int main(void) {
    FILE *f = fopen("src/mul.c", "w");
    if (!f) {
        perror("src/mul.c");
        return 1;
    }
    fprintf(f, "#include \"mul.h\"\n");
    fprintf(f, "int mul(int a, int b) { return a * b; }\n");
    fclose(f);

    f = fopen("include/mul.h", "w");
    if (!f) {
        perror("include/mul.h");
        return 1;
    }
    fprintf(f, "#ifndef MUL_H\n");
    fprintf(f, "#define MUL_H\n");
    fprintf(f, "int mul(int a, int b);\n");
    fprintf(f, "#endif\n");
    fclose(f);

    return 0;
}
