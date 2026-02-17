#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Sorting algorithms implemented with AI

typedef enum sortingAlgorithm {
    QUICK_SORT,
    MERGE_SORT,
    RADIX_SORT,
    HEAP_SORT,
    NOT_FOUND,
    FAIL
} sortingAlgorithm;

typedef struct node {
    char* word;
    struct node* next;
} node;

typedef struct sortingResult {
    sortingAlgorithm algorithm;
    int index;
} sortingResult;

#define MAX_WORDS 100000
#define MAX_WORDS_LENGTH 100

node* uniqStringTable[MAX_WORDS];

static int hash(char* word) {
    int sum = 0;
    while (*word) {
        sum += (int) *word++;
    }
    if (sum < 0) {
        //printf("Sum is less than 0 error. Word is nothing. Should be discarded\n");
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

void quick_sort(const char* arr[], int n) {
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

void print_words(char** arr, int non_duplicate_words) {
    for (int i = 0; i < non_duplicate_words; i++) {
        printf("%s\n", arr[i]);
    }
}

int find_unique_param(char* argv[], int argc){
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0){
            return i;
        }
    }
    return -1;
}

void merge(char **array, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    char **L = malloc(n1 * sizeof(char*));
    char **R = malloc(n2 * sizeof(char*));

    for (int i = 0; i < n1; i++)
        L[i] = array[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = array[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (strcmp(L[i], R[j]) <= 0)
            array[k++] = L[i++];
        else
            array[k++] = R[j++];
    }

    while (i < n1)
        array[k++] = L[i++];
    while (j < n2)
        array[k++] = R[j++];

    free(L);
    free(R);
}

// Recursive merge sort function
void merge_sort(char **array, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(array, left, mid);
        merge_sort(array, mid + 1, right);
        merge(array, left, mid, right);
    }
}

#define R 256 // Number of possible characters (extended ASCII)

void msdRadixSort(char **array, char **aux, int left, int right, int d) {
    if (right <= left)
        return;

    int count[R + 2] = {0};

    // Compute frequency counts
    for (int i = left; i <= right; i++) {
        int c = (strlen(array[i]) > d) ? (unsigned char)array[i][d] + 1 : 0; // 0 for end-of-string
        count[c + 1]++;
    }

    // Transform counts to indices
    for (int r = 0; r < R + 1; r++)
        count[r + 1] += count[r];

    // Distribute
    for (int i = left; i <= right; i++) {
        int c = (strlen(array[i]) > d) ? (unsigned char)array[i][d] + 1 : 0;
        aux[count[c]++] = array[i];
    }

    // Copy back
    for (int i = left; i <= right; i++)
        array[i] = aux[i - left];

    // Recursively sort for each character value
    for (int r = 0; r < R; r++) {
        msdRadixSort(array, aux, left + count[r], left + count[r + 1] - 1, d + 1);
    }
}

void radix_sort(char **array, int n) {
    char **aux = (char **)malloc(n * sizeof(char*));
    msdRadixSort(array, aux, 0, n - 1, 0);
    free(aux);
}

// Swap two string pointers
void swap(char **a, char **b) {
    char *tmp = *a;
    *a = *b;
    *b = tmp;
}

// Heapify (percolate down) for a subtree rooted with index i
void heapify(char **arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    // See if left child exists and is greater than root
    if (left < n && strcmp(arr[left], arr[largest]) > 0)
        largest = left;

    // See if right child exists and is greater than largest so far
    if (right < n && strcmp(arr[right], arr[largest]) > 0)
        largest = right;

    // If largest is not root
    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest); // Recursively heapify the affected subtree
    }
}

void heap_sort(char **arr, int n) {
    // Build heap (rearrange array)
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    // One by one extract elements from heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        swap(&arr[0], &arr[i]);
        // call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

void sort(char** words, int count, sortingAlgorithm algorithm) {
    if (algorithm == NOT_FOUND || algorithm == QUICK_SORT) {
        quick_sort(words, count);
    } else if (algorithm == MERGE_SORT) {
        printf("Using mergesort:\n");
        merge_sort(words, 0, count - 1);
    } else if (algorithm == RADIX_SORT) {
        printf("Using radixsort:\n");
        radix_sort(words, count);
    } else if (algorithm == HEAP_SORT) {
        printf("Using heapsort:\n");
        heap_sort(words, count);
    } else {
        printf("Algorithm not found\n");
        exit(1);
    }
}

sortingAlgorithm find_sorting_algorithm(char* algorithm) {
    if (strcmp(algorithm, "quicksort") == 0) {
        return QUICK_SORT;
    } else if (strcmp(algorithm, "mergesort") == 0) {
        return MERGE_SORT;
    } else if (strcmp(algorithm, "radixsort") == 0) {
        return RADIX_SORT;
    } else if (strcmp(algorithm, "heapsort") == 0) {
        return HEAP_SORT;
    }
    return FAIL;
}

sortingResult find_sorting_param(char* argv[], int argc){
    sortingResult result;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-sorting") == 0){
            if (i == argc - 1) {
                result.algorithm = FAIL;
                result.index = -1;
                return result;
            }
            result.index = i;
            i++;
            result.algorithm = find_sorting_algorithm(argv[i]);
            return result;
        }
    }
    result.algorithm = NOT_FOUND;
    result.index = -1;
    return result;
}

void help() {
    printf("Sorts words from newline seperated text file using quicksort\n");
    printf("Usage: [optional parameters] <filename>\n");
    printf("-u only prints unique words\n");
    printf("-sorting [sorting algorithm] uses specific sorting algo to sort words\n");
    printf("sorting algos: quicksort, mergesort, radixsort, heapsort\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        help();
        exit(0);
    }

    if (strcmp(argv[1], "-help") == 0) {
        help();
        exit(0);
    }

    FILE* flpt = fopen(argv[argc - 1], "r");

    if (!flpt) {
        printf("File Not Found\n");
        exit(1);
    }

    int unique_param_index = find_unique_param(argv, argc);
    sortingResult sorting = find_sorting_param(argv, argc);
    if (sorting.algorithm == FAIL) {
        printf("Algorithm not found\n");
        printf("Sorting options: quicksort, mergesort, radixsort, heapsort\n");
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

    if (unique_param_index != -1) {
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

        char** unique_words = malloc(non_duplicate_words * sizeof(char*));
        insert_into_new_stringtable(unique_words, non_duplicate_words);

        sort(unique_words, non_duplicate_words, sorting.algorithm);
        print_words(unique_words, non_duplicate_words);

    } else {
        sort(words, word_count, sorting.algorithm);
        print_words(words, word_count);
    }

    return 0;
}