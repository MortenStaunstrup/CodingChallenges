#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TABLE_SIZE 65000

typedef uint32_t u8char;

typedef struct Node {
    size_t codepoint;
    long long freq;
    struct Node* next;
} Node;

Node* table[MAX_TABLE_SIZE];

static size_t hash(size_t codepoint){ return (codepoint * 2654435761) % MAX_TABLE_SIZE; }

char* handle_file(char* fileName) {
    FILE *f = fopen(fileName, "rb");
    if (f == NULL) {
        perror("fopen");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char* buffer = malloc(fsize + 1);
    if (buffer == NULL) {
        perror("malloc");
        exit(1);
    }
    fread(buffer, 1, fsize, f);
    buffer[fsize] = '\0';
    fclose(f);
    return buffer;
}



int main(int argc, char *argv[]) {
    char* localeSet = setlocale(LC_ALL, "");
    if (localeSet == NULL) {
        perror("Could not read locale information");
        exit(1);
    }

    if (argc == 2) {
        FILE *fil = fopen(argv[1], "rb");
        if (fil == NULL) {
            perror("Could not open file");
            exit(1);
        }


        int c;
        while ((c = fgetc(fil)) != EOF) {
            printf("%u\n", c);
        }

        fclose(fil);
    }
    return 0;
}