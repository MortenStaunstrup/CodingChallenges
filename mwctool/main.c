#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdbool.h>


void errOpenFile();
void errReadFile();
void notACommand(const char *argv);
void mwcHelp();

int getChars(const char *filename);
int getWords(const char *filename);
int getBytes(const char *filename);
int getLines(const char *filename);

int getCharsNoFile();
int getWordsNoFile();
int getBytesNoFile();
int getLinesNoFile();

int main(int argc, char* argv[])
{

    // No arguments given

    if (argc == 1) {
        printf("Type -h or --h for help");
        return 0;
    }

    // command given

    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--h") == 0) {
            mwcHelp();
            return 0;
        }
        if (strcmp(argv[1], "-c") == 0) {
            int result = getBytesNoFile();

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            if (result == -2) {
                errOpenFile();
                return -1;
            }

            printf("%i \r\n", result);
            return 0;

        }
        if (strcmp(argv[1], "-l") == 0) {
            int result = getLinesNoFile();

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            printf("%i \r\n", result);
            return 0;

        }
        if (strcmp(argv[1], "-w") == 0) {
            int result = getWordsNoFile();

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            printf("%i \r\n", result);
            return 0;

        }
        if (strcmp(argv[1], "-m") == 0) {
            int result = getCharsNoFile();

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            printf("%i \r\n", result);
            return 0;
        }

        int byteResult = getBytes(argv[1]);
        int lineResult = getLines(argv[1]);
        int wordResult = getWords(argv[1]);

        if (byteResult == -1 || lineResult == -1 || wordResult == -1) {
            errReadFile();
            return -1;
        }
        if (byteResult == -2) {
            errOpenFile();
            return -1;
        }

        printf("%i %i %i %s \r\n", lineResult, wordResult, byteResult, argv[1]);
        return 0;
    }

    // command and potential file given

    if (argc == 3) {
        if (strcmp(argv[1], "-c") == 0) {

            int result = getBytes(argv[2]);

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            if (result == -2) {
                errReadFile();
                return -1;
            }

            printf("%i %s \r\n", result, argv[2]);
            return 0;
        }
        if (strcmp(argv[1], "-l") == 0) {

            int result = getLines(argv[2]);

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            printf("%i %s \r\n", result, argv[2]);
            return 0;

        }
        if (strcmp(argv[1], "-w") == 0) {

            int result = getWords(argv[2]);

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            printf("%i %s \r\n", result, argv[2]);
            return 0;

        }
        if (strcmp(argv[1], "-m") == 0) {

            int result = getChars(argv[2]);

            if (result == -1) {
                errOpenFile();
                return -1;
            }

            printf("%i %s \r\n", result, argv[2]);
            return 0;

        }
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--h") == 0) {
            mwcHelp();
            return 0;
        }

        notACommand(argv[1]);
        return 0;

    }

    if (argc > 3 ) {
        printf("Too many parameters given\r\n");
        return 0;
    }

    return 0;
}

void errOpenFile() {
    perror("Could not open file\r\n");
}

void errReadFile() {
    perror("Could not read file bytes\r\n");
}

void notACommand(const char *argv) {
    printf("'%s' is not a command, mwc -h for help\r\n", argv);
}

void mwcHelp() {
    printf("<filename> || outputs the bytes, lines and words of a file\r\n"
           "-c <filename> || outputs the bytes of the file\r\n"
           "-l <filename> || outputs the lines of a file\r\n"
           "-w <filename> || outputs the words of a file\r\n"
           "-m <filename> || outputs the characters of a file (only works with UTF-8 encoding)\r\n"
           "All args work in a pipeline on a file eg 'cat <filename> | mcw -l'");
}

int getChars(const char *filename) {

    // Virker kun med UTF-8

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) return -1;
    int count = 0;
    int c;
    while ((c = getc(fp)) != EOF) {
        // Count only bytes that are not continuation bytes in UTF-8
        if ((c & 0xC0) != 0x80) {
            count++;
        }
    }
    fclose(fp);
    return count;
}

int getWords(const char *filename) {
    FILE *fp = fopen(filename, "r");

    int counter = 0;
    char c;
    bool whitespace = true;

    if (fp == NULL) {
        return -1;
    }

    for (c = getc(fp); c != EOF; c = fgetc(fp)) {
        if (whitespace && !isspace(c)) {
            counter++;
            whitespace = false;
        } else if (!whitespace && isspace(c)) {
            whitespace = true;
        }
    }
    fclose(fp);
    return counter;
}

int getBytes(const char *filename) {

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    if (size < 0) {
        return -2;
    }
    return size;
}

int getLines(const char *filename) {

    FILE *fp = fopen(filename, "r");

    int counter = 0;
    char c;

    if (!fp) {
        return -1;
    }

    for (c = getc(fp); c != EOF; c = getc(fp)) {
        if (c == '\n') {
            counter++;
        }
    }

    fclose(fp);
    return counter;
}

int getCharsNoFile() {
    FILE *fp;

    fp = stdin;
    if (fp == NULL) {
        return -1;
    }

    int count = 0;
    int c;
    while ((c = getc(fp)) != EOF) {
        // Count only bytes that are not continuation bytes in UTF-8
        if ((c & 0xC0) != 0x80) {
            count++;
        }
    }
    fclose(fp);
    return count;

}
int getWordsNoFile() {
    FILE *fp;

    fp = stdin;
    int counter = 0;
    char c;
    bool whitespace = true;

    if (fp == NULL) {
        return -1;
    }

    for (c = getc(fp); c != EOF; c = fgetc(fp)) {
        if (whitespace && !isspace(c)) {
            counter++;
            whitespace = false;
        } else if (!whitespace && isspace(c)) {
            whitespace = true;
        }
    }
    fclose(fp);
    return counter;
}
int getBytesNoFile() {
    FILE *fp;

    fp = stdin;
    if (!fp) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    if (size < 0) {
        return -2;
    }
    return size;
}
int getLinesNoFile() {
    FILE *fp;

    fp = stdin;
    int counter = 0;
    char c;

    if (!fp) {
        return -1;
    }

    for (c = getc(fp); c != EOF; c = getc(fp)) {
        if (c == '\n') {
            counter++;
        }
    }

    fclose(fp);
    return counter;
}