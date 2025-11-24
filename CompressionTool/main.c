#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 65536
#define MAX_BIT_LENGTH 10000

typedef enum kind {
    LEAF_NODE,
    INTERNAL_NODE
} kind;

typedef struct BitCode {
    uint8_t bits[MAX_BIT_LENGTH];
    int bitLength;
} BitCode;

typedef struct DecodeTable {
    uint32_t key;
    BitCode bits;
} DecodeTable;

typedef struct Node {
    uint32_t key;
    unsigned long freq;
    BitCode bits;
    struct Node* next;
} Node;

// The hash map
// Each index can be a linked list to handle hash collisions (called buckets)
Node* table[TABLE_SIZE];
long long tableCount = 0;

typedef struct HuffmanNode {
    uint32_t key;
    unsigned long freq;
    unsigned long weight;
    enum kind kind;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct HuffmanTree {
    HuffmanNode *root;
    unsigned long weight;
} HuffmanTree;

typedef struct PriorityQueue {
    int size;
    HuffmanTree *trees;
} PriorityQueue;

char* handle_file(const char* text);
void incr(uint32_t key);
void print_map(void);
int utf8_next(const char **p, uint32_t *out);
HuffmanNode* create_huffman_node_array();
void print_huffman_node_array(const HuffmanNode* array);
void sort_huffman_node_array(HuffmanNode *array);
void init_priorityqueue(PriorityQueue *pq, HuffmanNode *sortedArray);
void add_to_queue(PriorityQueue *pq, HuffmanTree tree);
void swap_queue(HuffmanTree *a, HuffmanTree *b);
void display_queue(PriorityQueue *pq);
HuffmanTree delete_from_queue(PriorityQueue *pq);
int isEmpty(PriorityQueue *pq);
HuffmanTree create_huffman_tree(PriorityQueue *pq);
void print_huffman_tree(const HuffmanNode *node, int depth);
void create_codePrefixes(HuffmanTree *tree);
void traverse_tree(HuffmanNode *curr, BitCode *currPrefix);
char* concat(const char *s1, const char *s2);
char* copy_file_name(const char *fileName);
void bitcode_to_string(const BitCode *bc, char *s);
void compress_and_write(FILE *out, const char *text);
void help_function();
void print_decodeTable(DecodeTable *dTable, int codePointCount);
void bitcode_push(BitCode *bc, uint8_t bit);

// Knuth's hash function
// Returns index
static inline size_t hash32(uint32_t x) {return (x * 2654435761u) % TABLE_SIZE;}

int main(const int argc, char* argv[]) {

    if (argc == 1) {
        printf("Usage: <exe file> <txt file> <optional parameters>\n");
        printf("Use '-h' for help");
    }

    if (argc == 2 || argc == 4) {

        if (strcmp(argv[1], "-h") == 0) {
            help_function();
            return 0;
        }

        if (argc == 4 && strcmp(argv[2], "-o") != 0) {
            printf("Command not known");
            return 1;
        }

        char* text = handle_file(argv[1]);
        if (text == NULL) {
            printf("error: file not found");
            return 1;
        }
        const char *p = text;
        uint32_t cp;

        printf("Creating Hash table with frequencies...\n");
        // Main loop to increase frequency of codepoint
        while (1) {
            int r = utf8_next(&p, &cp);
            if (r == 1) break;
            if (r == -1) { p++; continue; } // skip invalid byte
            incr(cp);
        }
        print_map();

        printf("Creating Huffman node array...\n");
        HuffmanNode *array = create_huffman_node_array();
        print_huffman_node_array(array);

        printf("Sorting Huffman node array...\n");
        sort_huffman_node_array(array);
        print_huffman_node_array(array);

        printf("Creating Priority Queue...\n");
        PriorityQueue pq;
        init_priorityqueue(&pq, array);
        display_queue(&pq);

        printf("Creating Huffman tree...\n");
        HuffmanTree tree = create_huffman_tree(&pq);
        print_huffman_tree(tree.root, 0);

        printf("Creating Code Prefixes...\n");
        create_codePrefixes(&tree);

        printf("Printing hash-table...\n");
        print_map();

        // Create the new compressed file and make the header
        FILE *outputFile;
        if (argc == 4) {
            char* fileName = concat(argv[3], ".txt");
            if (fileName == NULL) {
                printf("Filename could not be made");
                return 1;
            }
            outputFile = fopen(fileName, "wb");
        } else {
            char* fileName = copy_file_name(argv[1]);
            printf("Name: %s\n", fileName);
            if (fileName == NULL) {
                printf("Filename could not be made");
                return 1;
            }
            outputFile = fopen(fileName, "wb");
        }
        fprintf(outputFile, "%llu\n", tableCount);
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            for (Node *n = table[i]; n; n = n->next) {
                char codeStr[MAX_BIT_LENGTH + 1];
                bitcode_to_string(&n->bits, codeStr);
                fprintf(outputFile, "%u %s\n", n->key, codeStr);
            }
        }

        compress_and_write(outputFile, text);
        fclose(outputFile);

        printf("Freeing memory...\n");
        free(array);
        free(text);
        free(pq.trees);
        printf("Success\n");
    }

    if (argc == 3) {
        if (strcmp(argv[2], "-o") == 0) {
            printf("Usage of -o needs parameter: -o <new-filename>");
            return 0;
        }
        if (strcmp(argv[2], "-h") == 0) {
            help_function();
        }
        if (strcmp(argv[2], "-d") == 0) {
            char* compressedText = handle_file(argv[1]);
            char* p = compressedText;

            // Find the code point count
            int codePointCount = 0;
            while (*p != '\n') {
                codePointCount = codePointCount * 10 + (*p++ - '0');
            }
            if (codePointCount == 0) {
                printf("error: compressed file has no content");
                return -1;
            }

            // skip the newline char
            p++;

            DecodeTable* dTable = malloc(sizeof(DecodeTable) * codePointCount);

            // Find the code points and the bits to decode them
            for (int i = 0; i < codePointCount; i++) {
                uint32_t cp = 0;
                while (*p != ' ') {
                    cp = cp * 10 + (*p - '0');
                    p++;
                }
                p++;
                dTable[i].key = cp;

                BitCode* b = &dTable[i].bits;
                b->bitLength = 0;
                while (*p != '\n') {
                    if (*p == '0') {
                        bitcode_push(b, 0);
                    } else if (*p == '1') {
                        bitcode_push(b, 1);
                    } else {
                        perror("Wrong syntax in bitCode for code point");
                        free(dTable);
                        return -1;
                    }
                    p++;
                }
                p++; // skip '\n'
            }

            print_decodeTable(dTable, codePointCount);

            free(dTable);
        }
    }

    if (argc > 4) {
        printf("Too many arguments");
    }

    return 0;
}

void print_decodeTable(DecodeTable *dTable, int codePointCount) {
    for (size_t i = 0; i < codePointCount; i++) {
        char codeStr[MAX_BIT_LENGTH + 1];
        bitcode_to_string(&dTable[i].bits, codeStr);
        printf("Codepoint: %u BitString: %s\n", dTable[i].key, codeStr);
    }
}

void help_function() {
    printf("Usage: <exe file> <txt file> <optional parameters>\n");
    printf("Optional parameters:\n");
    printf("-o <new-filename> Used to give the new compressed file a name\n");
    printf("<compressed file> -d Used to decompress compressed file\n");
}

void compress_and_write(FILE *out, const char *text) {
    unsigned char buffer = 0;
    int bit_count = 0;

    const char *p = text;
    uint32_t cp;
    while (1) {
        int r = utf8_next(&p, &cp);
        if (r == 1) break;
        if (r == -1) { p++; continue; }

        // Find code for cp
        size_t idx = hash32(cp);
        Node *found = NULL;
        for (Node *n = table[idx]; n; n = n->next) {
            if (n->key == cp) { found = n; break; }
        }
        if (!found) continue; // (should not happen...)

        // Write each bit
        for (int i = 0; i < found->bits.bitLength; ++i) {
            buffer <<= 1;
            if (found->bits.bits[i]) buffer |= 1;
            bit_count++;

            if (bit_count == 8) {
                fputc(buffer, out);
                buffer = 0;
                bit_count = 0;
            }
        }
    }

    // Flush incomplete byte
    if (bit_count > 0) {
        buffer <<= (8 - bit_count);
        fputc(buffer, out);
    }
}

char* copy_file_name(const char *fileName) {
    char *firstPart = malloc(strlen(fileName) -3);
    if (firstPart == NULL) {
        return nullptr;
    }
    strncpy(firstPart, fileName, strlen(fileName) -4);
    firstPart[strlen(fileName) - 4] = '\0';
    char *trueResult = concat(firstPart,"-compressed.txt");
    free(firstPart);
    return trueResult;
}

char* concat(const char *s1, const char *s2){
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    if (result == NULL) {
        return nullptr;
    }
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

// s must have enough space, at least MAX_BIT_LENGTH+1
void bitcode_to_string(const BitCode *bc, char *s) {
    for (int i = 0; i < bc->bitLength; i++) {
        s[i] = bc->bits[i] ? '1' : '0';
    }
    s[bc->bitLength] = '\0';
}

void bitcode_push(BitCode *bc, uint8_t bit) {
    if (bc->bitLength < MAX_BIT_LENGTH) {
        bc->bits[bc->bitLength++] = bit;
    } else {
        fprintf(stderr, "BitCode overflow\n");
        exit(1);
    }
}


void bitcode_pop(BitCode *bc) {
    if (bc->bitLength > 0) bc->bitLength--;
}

void traverse_tree(HuffmanNode *curr, BitCode *currPrefix) {
    if (!curr) return;

    if (curr->kind == LEAF_NODE) {
        size_t idx = hash32(curr->key);
        for (Node *n = table[idx]; n; n = n->next) {
            if (n->key == curr->key) {
                n->bits = *currPrefix; // store current bit pattern
            }
        }
    } else {
        // Go left with 0
        bitcode_push(currPrefix, 0);
        traverse_tree(curr->left, currPrefix);
        bitcode_pop(currPrefix);
        // Go right with 1
        bitcode_push(currPrefix, 1);
        traverse_tree(curr->right, currPrefix);
        bitcode_pop(currPrefix);
    }
}

void create_codePrefixes(HuffmanTree *tree) {
    BitCode currPrefix;
    currPrefix.bitLength = 0;
    traverse_tree(tree->root, &currPrefix);
}

void print_huffman_tree(const HuffmanNode *node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; ++i)
        printf("  "); // Indent for tree visualization

    if (node->kind == LEAF_NODE) {
        printf("Leaf: key=U+%04X, val=%lu, weight=%lu\n", node->key, node->freq, node->weight);
    } else {
        printf("Internal: weight=%lu\n", node->weight);
        print_huffman_tree(node->left, depth+1);
        print_huffman_tree(node->right, depth+1);
    }
}

HuffmanTree create_huffman_tree(PriorityQueue *pq) {
    HuffmanTree tmp1, tmp2, tmp3;

    while (pq->size > 1) {
        tmp1 = delete_from_queue(pq);
        tmp2 = delete_from_queue(pq);

        tmp3.root = malloc(sizeof(HuffmanNode));
        tmp3.root->key = -1;
        tmp3.root->freq = 0;
        tmp3.root->weight = tmp1.weight + tmp2.weight;
        tmp3.root->kind = INTERNAL_NODE;
        tmp3.root->left = tmp1.root; tmp3.root->right = tmp2.root;
        tmp3.weight = tmp1.weight + tmp2.weight;
        add_to_queue(pq, tmp3);
    }
    return delete_from_queue(pq);
}

int isEmpty(PriorityQueue *pq) {
    return pq->size == 0;
}

HuffmanTree delete_from_queue(PriorityQueue *pq){
    if (isEmpty(pq)) {
        printf("Queue is empty\n");
        exit(-1);
    }
    HuffmanTree minItem = pq->trees[0];
    // Set the last element to be the first, and decrement the size
    pq->trees[0] = pq->trees[--pq->size];

    int i = 0;
    while (true) {
        int left = 2 * i + 1, right = 2 * i + 2, smallest = i;
        // Check if the left or right node is smaller than the current node, then swap to keep integrity of min-heap
        if (left < pq->size && pq->trees[left].weight < pq->trees[smallest].weight)
            smallest = left;
        if (right < pq->size && pq->trees[right].weight < pq->trees[smallest].weight)
            smallest = right;
        if (smallest != i) {
            swap_queue(&pq->trees[i], &pq->trees[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
    return minItem;
}

void display_queue(PriorityQueue *pq) {
    printf("Current queue size: %u\n", pq->size);
    for (int i = 0; i < pq->size; i++) {
        printf("Codepoint: %d, Weight: %lu\n", pq->trees[i].root->key, pq->trees[i].weight);
    }
}

void swap_queue(HuffmanTree *a, HuffmanTree *b) {
    HuffmanTree tmp = *a;
    *a = *b;
    *b = tmp;
}

void init_priorityqueue(PriorityQueue *pq, HuffmanNode *sortedArray) {
    pq->size = 0;
    pq->trees = malloc(tableCount * sizeof(HuffmanTree));
    // All single Huffman nodes start out being trees, finally being merged to be 1 tree later
    for (int i = 0; i < tableCount; i++) {
        HuffmanTree tree;
        tree.weight = sortedArray[i].weight;
        tree.root = &sortedArray[i];
        add_to_queue(pq, tree);
    }
}

void add_to_queue(PriorityQueue *pq, HuffmanTree tree) {
    if (pq->size >= tableCount) {
        printf("Queue is full\n");
        return;
    }
    // Add the tree to the end of the queue
    int i = pq->size++;
    pq->trees[i] = tree;

    // While the added tree is less than the parent, swap them, so the smallest is at the front
    while (i > 0) {
        int parent = (i - 1) / 2; // Calculation for the parent node
        if (pq->trees[i].weight < pq->trees[parent].weight) {
            swap_queue(&pq->trees[i], &pq->trees[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

int compare(const void* a, const void* b) {
    const HuffmanNode* nodeA = (const HuffmanNode*)a;
    const HuffmanNode* nodeB = (const HuffmanNode*)b;
    if (nodeA->weight < nodeB->weight) return -1;
    if (nodeA->weight > nodeB->weight) return 1;
    return 0;
}

void sort_huffman_node_array(HuffmanNode *array) {
    qsort(array, tableCount, sizeof(HuffmanNode), compare);
}

HuffmanNode* create_huffman_node_array() {
    HuffmanNode* array = malloc(sizeof(HuffmanNode) * tableCount);
    if (array == NULL) {
        perror("Creating node array failed");
        exit(EXIT_FAILURE);
    }
    int index = 0;
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        for (Node *n = table[i]; n; n = n->next) {
            array[index].key = n->key;
            array[index].freq = n->freq;
            array[index].kind = LEAF_NODE;
            array[index].weight = n->freq;
            array[index].left = nullptr;
            array[index].right = nullptr;
            index++;
        }
    }
    return array;
}

void print_huffman_node_array(const HuffmanNode* array) {
    for (size_t i = 0; i < tableCount; i++) {
        printf("Index: %llu, key: %u, frequency: %lu\n", i, array[i].key, array[i].freq);
    }
    printf("Count: %lld\n", tableCount);
}

char* handle_file(const char* text) {
    FILE* f = fopen(text, "rb");
    if (!f) {
        perror("Couldn't open file");
        return nullptr;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char* buffer = malloc(fsize + 1);
    buffer[fsize] = '\0';
    fread(buffer, 1, fsize, f);
    fclose(f);
    return buffer;
}


void incr(uint32_t key) {
    size_t idx = hash32(key);
    // Finds the key in the hashmap, and increments the value
    for (Node *n = table[idx]; n; n=n->next) {
        if (n->key == key) { n->freq++; return;}
    }
    // If the key does not exist, create it in the linked list at the head, in the index
    Node *n = malloc(sizeof *n);
    if (!n) exit(1);

    n->bits.bitLength = 0;
    n -> key = key; n -> freq = 1; n -> next = table[idx]; table[idx] = n;
    tableCount++;
}

void print_map(void) {
    printf("Table Chars and frequency:\n");
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        for (Node *n = table[i]; n; n = n->next) {
            char str[MAX_BIT_LENGTH + 1];
            bitcode_to_string(&n->bits, str);
            printf("U+%04X : %lu CodePrefix: %s\n", n->key, n->freq, str);
        }
    }
    printf("Table Count: %llu\n", tableCount);
}


// 0 = Decoded a code piont and advanced the pointer
// 1 = Reached end of input (null terminator)
// -1 = Invalid UTF-8 sequence, skip the byte and try the next one
int utf8_next(const char **p, uint32_t *out) {
    const unsigned char *s = (const unsigned char *) *p;
    // If null terminator
    if (*s == 0) return 1;
    // If highest bit is 0 (one byte, value 0..127) then this is ASCII. Just return the code point and advance to next char
    if (*s < 0x80) { *out = *s++; *p = (const char*)s; return 0;}

    int len = 0;
    uint32_t cp = 0;
    // Determine the length of the UTF-8 char in bytes
    if ((s[0] & 0xC0) != 0x80) { len = 2; cp = s[0] & 0x1F; }
    else if ((s[0] & 0xF0) == 0xE0) { len = 3; cp = s[0] & 0x0F; }
    else if ((s[0] & 0xF8) == 0xF0) { len = 4; cp = s[0] & 0x07; }
    // If the 3 conditions are not met, invalid UTF-8, return -1
    else return -1;
    // Parse continuation bytes, assign cp the codepoint
    for (int i = 1; i < len; ++i) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    // Success, advance pointer by length of continuation bytes, and assign code point
    *p = (const char*)(s + len);
    *out = cp;
    return 0;
}