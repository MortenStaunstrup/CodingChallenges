#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char* word;
    struct node* next;
} node;


#define MAX_WORDS 100000
#define MAX_WORDS_LENGTH 100

static int hash(char* word) {
    int sum = 0;
    while (*word) {
        sum += (int) *word++;
    }
    return sum % MAX_WORDS;
}

char* handle_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("File Not Found\n");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char* buffer = (char*)malloc(length + 1);
    if (buffer == NULL) {
        printf("Memory Allocation Failed\n");
        return NULL;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int name_comparer(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void sort(const char* arr[], int n) {
    qsort(arr, n, sizeof(const char*), name_comparer);
}

int insert_into_table(node** table, int index, char* word) {
    node* curr = table[index];
    while (curr != NULL) {
        // If word already exist in the table, return prematurely
        if (strcmp(curr->word, word) == 0) {
            return 0;
        }
        curr = curr->next;
    }
    node* newNode = malloc(sizeof(node));
    if (newNode == NULL) {
        printf("Memory Allocation Failed\n");
        exit(1);
    }
    newNode->word = strdup(word);
    if (newNode->word == NULL) {
        printf("Memory Allocation Failed\n");
        exit(1);
    }
    newNode->next = table[index];
    table[index] = newNode;
    return 1;
}

void insert_into_table_array(char** arr, node** table, int non_duplicate_words) {
    int n = 0;
    int arrIdx = 0;
    while (n < MAX_WORDS) {
        node* curr = table[n];
        while (curr != NULL) {
            arr[arrIdx++] = curr->word;
            curr = curr->next;
        }
        n++;
    }
    if (arrIdx != non_duplicate_words) {
        printf("Error with inserting non_duplicate_words into new word arr\n");
        printf("Non duplicate word: %u\n", non_duplicate_words);
        printf("Index of last element in new array: %u\n", arrIdx);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Missing args\n");
        exit(1);
    }

    if (argc == 2 || argc == 3) {
        FILE* flpt = fopen(argv[1], "r");
        if (!flpt) {
            printf("File Not Found\n");
            exit(1);
        }


        char** words = malloc(MAX_WORDS * sizeof(char*));
        for (int i = 0; i < MAX_WORDS; i++) {
            words[i] = malloc(MAX_WORDS_LENGTH);
        }

        int word_count = 0;

        while (word_count < MAX_WORDS && fscanf(flpt, "%s", words[word_count]) == 1) {
            word_count++;
        }
        fclose(flpt);

        if (argc == 3 && strcmp(argv[2], "-u") == 0) {
            node** table = calloc(MAX_WORDS , sizeof(node));
            if (table == NULL) {
                printf("Memory Allocation Failed\n");
                exit(1);
            }
            int non_duplicate_words = 0;
            for (int i = 0; i < word_count; i++) {
                int idx = hash(words[i]);
                int result = insert_into_table(table, idx, words[i]);
                non_duplicate_words += result;
                printf("Result: %d\n", result);
                free(words[i]);
            }
            free(words);

            char** tableWords = malloc(non_duplicate_words * sizeof(char*));
            insert_into_table_array(tableWords, table, non_duplicate_words);

        } else {
            sort(words, word_count);

            for (int i = 0; i < word_count; i++) {
                printf("%s\n", words[i]);
            }
        }
    }

    return 0;
}