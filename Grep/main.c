#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: <reg-ex> <text-file>\n");
        exit(0);
    }

    FILE *f = fopen(argv[argc - 1], "r");
    if (!f) {
        printf("Can't open file %s\n", argv[argc - 1]);
        exit(0);
    }

    char* line;
    int lineSize = 500;
    line = malloc(lineSize * sizeof(char));
    if (strcmp(argv[1], "") == 0) {
        while (fgets(line, lineSize, f) != NULL) {
            line[strcspn(line, "\r\n")] = '\0';
            printf("%s\n", line);
        }
    }

    fclose(f);
    return 0;
}
