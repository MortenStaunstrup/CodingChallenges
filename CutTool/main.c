#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct arrayContainer {
    int columns[50];
    int length;
} arrayContainer;

char* handle_file(const char* fileArg) {
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

char* handle_stdin_file() {
    FILE* file = stdin;
    if (!file) {
        printf("Error opening file from stdin!\n");
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

arrayContainer doubleQuotes(const char* arg, const int initNumber) {
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

arrayContainer checkForF(const char* arg) {
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

    if (!dash || !f) {
        container.length = -1;
        return container;
    }

    container.columns[commas] = number;

    container.length = commas + 1;
    return container;
}

char checkForD(const char* arg) {
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

int index_of_f(char* argv[], const int argc) {
    for (int i = 0; i < argc; i++) {
        arrayContainer container = checkForF(argv[i]);
        if (container.length != -1) {
            for (int j = 0; j < container.length; j++) {
                if (container.columns[j] <= 0) {
                    return -2;
                }
            }
            return i;
        }
    }
    return -1;
}

int index_of_d(char* argv[], const int argc) {
    for (int i = 0; i < argc; i++) {
        char res = checkForD(argv[i]);
        if (res != -1) {
            return i;
        }
    }
    return -1;
}

int check_if_stdin(char* arg) {
    int count = 0;
    int dash = 0;
    while (*arg != '\0') {
        if (*arg == '-') {
            dash = 1;
            count++;
        }
        arg++;
    }
    if (dash && count == 1) {
        return 1;
    }
    return -1;
}

void help() {
    printf("Use cutem to print specific fields to the terminal, from value seperated file (standard being tabs)\n\n");
    printf("Usage: cutem -f[1..*] [optional parameter] <filename> or cutem -f[1..*] (file read from stdin)\n");
    printf("-f\tspecifies fields to print\t usages: -f2 only field 2 -f2,5 field 2 and 5 -f\"2 6 7\" field 2,6 and 7\n");
    printf("-d\tspecifies delimiter in file\t usages: -d, specifies , as delimiter -d! specifies ! as delimiter\n");
}

int main(const int argc, char* argv[]) {

    if (argc == 1) {
        help();
    }

    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0) {
            help();
            return 0;
        }
        // Check if -f exists
        int indx_f = index_of_f(argv, argc);
        if (indx_f == -1) {
            printf("Missing or invalid -f argument, see --help for help");
            return 1;
        } else if (indx_f == -2) {
            printf("Can't access field of 0");
            return 1;
        }

        arrayContainer container = checkForF(argv[indx_f]);
        int length = container.length;

        char* file = handle_stdin_file();
        if (file == NULL)
            return 1;

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
    }

    if (argc == 3) {
        // Check if -f exists
        int indx_f = index_of_f(argv, argc);
        if (indx_f == -1) {
            printf("Missing or invalid -f argument, see --help for help");
            return 1;
        } else if (indx_f == -2) {
            printf("Can't access field of 0");
            return 1;
        }

        arrayContainer container = checkForF(argv[indx_f]);
        int length = container.length;

        int indx_d = index_of_d(argv, argc);
        char delimiter = -1;
        if (indx_d != -1) {
            delimiter = checkForD(argv[indx_d]);
        }

        char* file;
        // If no file argument is given
        if (indx_f == argc - 1 || indx_d == argc - 1) {
            file = handle_stdin_file();
        } else {
            file = handle_file(argv[argc - 1]);
        }
        if (file == NULL)
            return 1;

        char* p = file;

        int currCol = 1;

        if (delimiter == -1) {
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
        } else {
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
        }

        free(file);
    }

    if (argc == 4) {
        // Check if -f exists
        int indx_f = index_of_f(argv, argc);
        if (indx_f == -1) {
            printf("Missing or invalid -f argument, see --help for help");
            return 1;
        } else if (indx_f == -2) {
            printf("Can't access field of 0");
            return 1;
        }

        arrayContainer container = checkForF(argv[indx_f]);
        int length = container.length;


        int indx_d = index_of_d(argv, argc);
        char delimiter = -1;
        if (indx_d == -1) {
            printf("Missing or invalid -d argument, see --help for help");
            return 1;
        }
        // Check if -d exists, both must exist with 4 arguments given
        delimiter = checkForD(argv[indx_d]);

        // Check if is '-' stdin input
        char* file;
        int isStdin = check_if_stdin(argv[argc - 1]);
        if (isStdin == 1) {
            file = handle_stdin_file();
        } else {
            file = handle_file(argv[argc - 1]);
        }
        if (file == NULL) {
            return 1;
        }

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

    if (argc >= 5) {
        printf("Too many arguments given, see --help for help");
    }
    return 0;
}