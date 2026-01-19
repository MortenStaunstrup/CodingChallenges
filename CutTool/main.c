#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct arrayContainer {
    int columns[50];
    int length;
} arrayContainer;

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

arrayContainer doubleQuotes(char* arg, int initNumber) {
    arrayContainer container;
    int number = 0;
    int spaces = 0;
    container.columns[spaces++] = initNumber;

    arg++;
    while (*arg != '\0') {
        if (isdigit(*arg)) {
            number = 10 * number + (*arg - '0');
        } else if (*arg == ' ') {
            container.columns[spaces] = number;
            number = 0;
            spaces++;
        } else {
            container.length = -1;
            return container;
        }
        arg++;
    }


    container.columns[spaces] = number;
    container.length = spaces + 1;
    return container;
}

arrayContainer checkForF(char* arg) {
    arrayContainer container;
    int dash = 0;
    int f = 0;
    int number = 0;

    int commas = 0;

    while (*arg != '\0') {
        if (!dash && !f && !number && *arg == '-') {
            dash = 1;
        } else if (dash && !f && !number && *arg == 'f') {
            f = 1;
        } else if (dash && f && *arg == ' ' && !commas && number) {
          return doubleQuotes(arg, number);
        } else if (dash && f && isdigit(*arg)) {
            number = number * 10 + (*arg - '0');
        } else if (dash && f && number && *arg == ',') {
            container.columns[commas] = number;
            number = 0;
            commas++;
        }else {
            container.length = -1;
            return container;
        }
        arg++;
    }

    container.columns[commas] = number;

    container.length = commas + 1;
    return container;
}

char checkForD(char* arg) {
    int dash = 0;
    int d = 0;
    char otherChar = -1;

    while (*arg != '\0') {
        if (!dash && !d && *arg == '-') {
            dash = 1;
        }
        else if (dash && !d && *arg == 'd') {
            d = 1;
        } else if (dash && d) {
            otherChar = *arg;
        } else {
            return -1;
        }
        arg++;
    }
    return otherChar;
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        arrayContainer container = checkForF(argv[1]);
        int length = container.length;

        if (length == -1) {
            printf("'%s' invalid form of -f, see --help for more", argv[1]);
            return 0;
        }

        char* file = handle_file(argv[2]);
        char* p = file;

        int currCol = 1;

        while (*p != '\0') {
            if (*p == '\n') {
                currCol = 1;
                printf("\n");
            } else if (*p == '\t') {
                currCol++;
            } else {
                for (int i = 0; i < length; i++) {
                    if (currCol == container.columns[i] && *p != '\t') {
                        while (*p != '\t') {
                            printf("%c", *p);
                            p++;
                        }
                        if (i != length - 1) {
                            printf("\t");
                        }
                        currCol++;
                    }
                }
            }
            p++;
        }

        free(file);
    } else if (argc == 4) {
        arrayContainer container = checkForF(argv[1]);
        int length = container.length;
        if (length == -1) {
            printf("'%s' invalid form of -f, see --help for more", argv[1]);
            return 0;
        }

        char delimiter = checkForD(argv[2]);
        if (delimiter == -1) {
            printf("'%s' invalid form of -d, see --help for more", argv[2]);
            return 0;
        }

        char* file = handle_file(argv[3]);
        char* p = file;

        int currCol = 1;

        while (*p != '\0') {
            if (*p == '\n') {
                currCol = 1;
                printf("\n");
            } else if (*p == delimiter) {
                currCol++;
            } else {
                for (int i = 0; i < length; i++) {
                    if (currCol == container.columns[i] && *p != delimiter) {
                        while (*p != delimiter) {
                            printf("%c", *p);
                            p++;
                        }
                        if (i != length - 1) {
                            printf("%c", delimiter);
                        }
                        currCol++;
                    }
                }
            }
            p++;
        }
        free(file);

    }
    return 0;
}
