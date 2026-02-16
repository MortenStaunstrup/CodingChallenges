#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char* word;
    struct node* next;
} node;


#define MAX_WORDS 100000
#define MAX_WORDS_LENGTH 100

node* uniqStringTable[MAX_WORDS];

static int hash(char* word) {
    int sum = 0;
    while (*word) {
        sum += (int) *word++;
    }
    if (sum < 0) {
        printf("Sum is less than 0 error. Word is nothing. Should be discarded\n");
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

void traverse_hash_table() {
    printf("Printing nodetable words:\n");
    for (int i = 0; i < MAX_WORDS; i++) {
        node* curr = uniqStringTable[i];
        while (curr != NULL) {
            printf("%s\n", curr->word);
            curr = curr->next;
        }
    }
    printf("Done printing nodetable words\n");
}

void insert_into_new_stringtable(char** table, int non_duplicate_words) {
    int i = 0;
    while (i != non_duplicate_words) {
        for (int j = 0; j < MAX_WORDS; j++) {
            node* curr = uniqStringTable[j];
            while (curr != NULL) {
                table[i] = malloc(sizeof(char) * (strlen(curr->word) + 1));
                strcpy(table[i], curr->word);
                curr = curr->next;
                i++;
            }
        }
    }
}

int insert_into_table(char* word, int index) {
    node* curr = uniqStringTable[index];
    while (curr != NULL) {
        if (strcmp(curr->word, word) == 0) {
            return 0;
        }
        curr = curr->next;
    }
    node* newWord = malloc(sizeof(node));
    newWord->word = malloc(strlen(word) + 1);
    strcpy(newWord->word, word);
    newWord->next = uniqStringTable[index];
    uniqStringTable[index] = newWord;
    return 1;
}

void traverse_unique_string_array(char** arr, int non_duplicate_words) {
    printf("Printing unique string array:\n");
    for (int i = 0; i < non_duplicate_words; i++) {
        printf("%s\n", arr[i]);
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
            for (int i = 0; i < MAX_WORDS; i++) {
                uniqStringTable[i] = NULL;
            }

            int non_duplicate_words = 0;
            for (int i = 0; i < word_count; i++) {
                int idx = hash(words[i]);
                if (idx < 0)
                    continue;
                int result = insert_into_table(words[i], idx);
                non_duplicate_words += result;
            }
            traverse_hash_table();

            char** unique_words = malloc(non_duplicate_words * sizeof(char*));
            insert_into_new_stringtable(unique_words, non_duplicate_words);

            sort(unique_words, non_duplicate_words);
            traverse_unique_string_array(unique_words, non_duplicate_words);

        } else {
            sort(words, word_count);

            for (int i = 0; i < word_count; i++) {
                printf("%s\n", words[i]);
            }
        }
    }

    return 0;
}