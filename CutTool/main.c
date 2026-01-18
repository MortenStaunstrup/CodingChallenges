#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* handle_file(char* fileArg) {
    FILE* file = fopen(fileArg, "r");
    if (!file) {
        printf("Error opening file!\n");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Buffer could not be created");
        return NULL;
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

int checkForF(char* arg) {
    int dash = 0;
    int f = 0;
    int number = 0;
    while (*arg != '\0') {
        if (!dash && !f && !number && *arg == '-') {
            dash = 1;
        }
        else if (dash && !f && !number && *arg == 'f') {
            f = 1;
        } else if (dash && f && isdigit(*arg)) {
            number = number * 10 + (*arg - '0');
        } else {
            return -1;
        }
        arg++;
    }
    return number;
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        int col = checkForF(argv[1]);
        if (col == -1) {
            printf("-f invalid, see --help for more");
            return 0;
        }

        char* file = handle_file(argv[2]);

        char* p = file;

        int currCol = 1;

        while (*p != '\0') {
            if (*p == '\n') {
                currCol = 1;
                printf("\n");
            }
            if (currCol == col) {
                printf("%c", *p);
            }
            if (*p == '\t') {
                currCol++;
            }

            p++;
        }

        free(file);
    }
    return 0;
}
