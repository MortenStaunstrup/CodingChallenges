#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TABLE_SIZE 65000
#define MAX_PREFIX_LENGTH 250

typedef enum node_kind {
    LEAF_NODE,
    INTERNAL_NODE
} node_kind;

typedef struct Node {
    uint32_t codepoint;
    unsigned long freq;
    struct Node *next;

    // Used for creating prefixes
    int prefix[MAX_PREFIX_LENGTH];
    int prefixSize;
} Node;

typedef struct HuffmanNode {
    uint32_t codepoint;
    unsigned long freq;
    unsigned long weight;
    node_kind kind;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct HuffmanTree {
    HuffmanNode *root;
    unsigned long weight;
} HuffmanTree;

typedef struct PriorityQueue {
    HuffmanTree *items;
    int size;
} PriorityQueue;

Node *table[MAX_TABLE_SIZE];
long long nodeCount = 0;

static size_t hash(size_t codepoint) { return (codepoint * 2654435761) % MAX_TABLE_SIZE; }

char *handle_file(char *fileName) {
    FILE *f = fopen(fileName, "rb");
    if (f == NULL) {
        perror("fopen");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char *buffer = malloc(fsize + 1);
    if (buffer == NULL) {
        perror("malloc");
        exit(1);
    }
    fread(buffer, 1, fsize, f);
    buffer[fsize] = '\0';
    fclose(f);
    return buffer;
}

void increment_codepoint(uint32_t key) {
    size_t idx = hash(key);
    Node *n;
    for (n = table[idx]; n; n = n->next) {
        if (n->codepoint == key) {
            n->freq++;
            return;
        }
    }
    n = malloc(sizeof(Node));
    n->codepoint = key;
    n->next = table[idx];
    n->freq = 1;
    n->prefixSize = 0;
    table[idx] = n;
    nodeCount++;
}

void print_table(bool hasPrefixes) {
    if (hasPrefixes) {
        printf("Printing table:\n");
        for (int i = 0; i < MAX_TABLE_SIZE; i++) {
            for (Node *p = table[i]; p; p = p->next) {
                printf("Codepoint: U+%04X Freq: %lu Binary Prefix: ", p->codepoint, p->freq);
                for (int j = 0; j < p->prefixSize; j++) {
                    printf("%u", p->prefix[j]);
                }
                printf("\n");
            }
        }
        printf("Table count: %llu\n", nodeCount);
    } else {
        printf("Printing table:\n");
        for (int i = 0; i < MAX_TABLE_SIZE; i++) {
            for (Node *p = table[i]; p; p = p->next) {
                printf("Codepoint: U+%04X Freq: %lu\n", p->codepoint, p->freq);
            }
        }
        printf("Table count: %llu\n", nodeCount);
    }
}

HuffmanNode *init_huffmannode_container() {
    HuffmanNode *container = malloc(sizeof(HuffmanNode) * nodeCount);
    if (container == NULL) {
        perror("Failed to allocate memory HuffmanNode");
        exit(1);
    }
    int index = 0;
    for (size_t i = 0; i < MAX_TABLE_SIZE; i++) {
        for (Node *n = table[i]; n; n = n->next) {
            container[index].codepoint = n->codepoint;
            container[index].freq = n->freq;
            container[index].kind = LEAF_NODE;
            container[index].weight = n->freq;
            container[index].left = nullptr;
            container[index].right = nullptr;
            index++;
        }
    }
    return container;
}

void pq_swap(HuffmanTree *a, HuffmanTree *b) {
    HuffmanTree tmp = *a;
    *a = *b;
    *b = tmp;
}

// GeeksForGeeks code
void heapifyUp(PriorityQueue *pq, int index) {
    if (index
        && pq->items[(index - 1) / 2].weight > pq->items[index].weight) {
        pq_swap(&pq->items[(index - 1) / 2],
                &pq->items[index]);
        heapifyUp(pq, (index - 1) / 2);
    }
}

void enqueue(PriorityQueue *pq, HuffmanTree node) {
    if (pq->size == nodeCount) {
        perror("queue is full");
        exit(1);
    }
    pq->items[pq->size++] = node;
    heapifyUp(pq, pq->size - 1);
}

// GeeksForGeeks code
void heapifyDown(PriorityQueue *pq, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < pq->size
        && pq->items[left].weight < pq->items[smallest].weight)
        smallest = left;

    if (right < pq->size
        && pq->items[right].weight < pq->items[smallest].weight)
        smallest = right;

    if (smallest != index) {
        pq_swap(&pq->items[index], &pq->items[smallest]);
        heapifyDown(pq, smallest);
    }
}

// GeeksForGeeks code
HuffmanTree dequeue(PriorityQueue *pq) {
    if (!pq->size) {
        perror("queue is empty");
        exit(1);
    }
    HuffmanTree item = pq->items[0];
    pq->items[0] = pq->items[--pq->size];
    heapifyDown(pq, 0);
    return item;
}


void init_priority_queue(PriorityQueue *pq, HuffmanNode *container) {
    pq->items = malloc(sizeof(HuffmanTree) * nodeCount);
    pq->size = 0;
    for (int i = 0; i < nodeCount; i++) {
        HuffmanTree tree;
        tree.root = &container[i];
        tree.weight = container[i].weight;
        enqueue(pq, tree);
    }
}

void print_priority_queue(PriorityQueue *pq) {
    printf("Printing priority queue:\n");
    for (int i = 0; i < pq->size; i++) {
        printf("Codepoint: U+%04X, Weight: %lu\n", pq->items[i].root->codepoint, pq->items[i].weight);
    }
}

HuffmanTree create_huffman_tree(PriorityQueue *pq) {
    HuffmanTree tmp1, tmp2, tmp3;
    while (pq->size > 1) {
        tmp1 = dequeue(pq);

        tmp2 = dequeue(pq);

        tmp3.root = malloc(sizeof(HuffmanNode));

        tmp3.weight = tmp1.weight + tmp2.weight;
        tmp3.root->kind = INTERNAL_NODE;
        tmp3.root->left = tmp1.root;
        tmp3.root->right = tmp2.root;
        tmp3.root->codepoint = -1;
        tmp3.root->weight = tmp1.weight + tmp2.weight;
        tmp3.root->freq = -1;
        enqueue(pq, tmp3);
    }
    return dequeue(pq);
}

void print_huffman_tree(HuffmanNode *node, int depth) {
    if (node == nullptr) return;
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    if (node->kind == INTERNAL_NODE) {
        printf("Internal node, Weight: %lu\n", node->weight);
    }
    if (node->kind == LEAF_NODE) {
        printf("Leaf node, CodePoint: U+%04X Frequency: %lu\n", node->codepoint, node->freq);
    }
    print_huffman_tree(node->left, depth + 1);
    print_huffman_tree(node->right, depth + 1);
}

void create_code_prefixes(HuffmanNode *node, int prefix[], int prefixLength) {
    if (node == nullptr) return;

    if (node->kind == LEAF_NODE) {
        if (prefixLength > MAX_PREFIX_LENGTH) {
            perror("Too many different chars to decode prefix");
            exit(1);
        }
        size_t idx = hash(node->codepoint);
        for (Node *n = table[idx]; n; n = n->next) {
            if (n->codepoint == node->codepoint) {
                n->prefixSize = prefixLength == 0 ? 1 : prefixLength;
                memcpy(n->prefix, prefix, prefixLength * sizeof(int));
                if (prefixLength == 0) n->prefix[0] = 0;
                break;
            }
        }
    } else if (node->kind == INTERNAL_NODE) {
        if (prefixLength > MAX_PREFIX_LENGTH) {
            perror("Too many different chars to decode prefix");
            exit(1);
        }
        // Left child = 0
        prefix[prefixLength] = 0;
        create_code_prefixes(node->left, prefix, prefixLength + 1);
        // Right child = 1
        prefix[prefixLength] = 1;
        create_code_prefixes(node->right, prefix, prefixLength + 1);
    }
}

void init_create_code_prefixes(HuffmanTree tree) {
    int prefix[MAX_PREFIX_LENGTH] = {0};
    int prefixLength = 0;
    create_code_prefixes(tree.root, prefix, prefixLength);
}

char *concat(char *str1, char *str2) {
    char *result = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    result[strlen(str1) + strlen(str2)] = '\0';
    return result;
}


uint32_t decode_utf8_codepoint(char *p, int *len) {
    unsigned char c = (unsigned char) *p;
    uint32_t codepoint = 0;

    // AI code !!!
    // Read bits and decode UTF-8 chars
    if (c < 0x80) {
        codepoint = c;
        *len = 1;
    } else if ((c >> 5) == 0x6) {
        // 110xxxxx
        codepoint = ((c & 0x1F) << 6) | (p[1] & 0x3F);
        *len = 2;
    } else if ((c >> 4) == 0xE) {
        // 1110xxxx
        codepoint = ((c & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        *len = 3;
    } else if ((c >> 3) == 0x1E) {
        // 11110xxx
        codepoint = ((c & 0x07) << 18) | ((p[1] & 0x3F) << 12)
                    | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        *len = 4;
    } else {
        p++;
        codepoint = -1;
    }
    return codepoint;
}

void write_bit_to_file(FILE* outputFile, int bit, int* bitsInBuffer, unsigned char* byte) {
    if (bit & 1) {
        *byte |= (1 << (7 - *bitsInBuffer));
    }

    (*bitsInBuffer)++;

    if (*bitsInBuffer == 8) {
        // size used to be sizeof(byte) which was size of pointer (usually 8 bytes) which was obv wrong
        fwrite(byte, 1, 1, outputFile);
        *byte = 0;
        *bitsInBuffer = 0;
    }
}

void write_bytes_outputfile(FILE *outFile, char *filePointer) {
    unsigned char byte = 0;
    int bitsInBuffer = 0;

    int utf8len = 0;
    while (*filePointer) {
        uint32_t codepoint = decode_utf8_codepoint(filePointer, &utf8len);

        if (codepoint != -1) {
            size_t idx = hash(codepoint);
            for (Node *n = table[idx]; n; n = n->next) {
                if (n->codepoint == codepoint) {
                    for (int i = 0; i < n->prefixSize; i++) {
                        write_bit_to_file(outFile, n->prefix[i], &bitsInBuffer, &byte);
                    }
                    break;
                }
            }
        }
        filePointer += utf8len;
    }
}

char* remove_txt_file_extension(char *fileName) {
    char* result = malloc(strlen(fileName) - 4);
    strncpy(result, fileName, strlen(fileName) - 4);
    result[strlen(fileName) - 4] = '\0';
    return result;
}

int main(int argc, char *argv[]) {
    char *localeSet = setlocale(LC_ALL, "");
    if (localeSet == NULL) {
        perror("Could not read locale information");
        exit(1);
    }

    if (argc == 2) {
        char *file = handle_file(argv[1]);
        char *p = file;
        int len = 0;

        while (*p) {
            uint32_t codepoint = decode_utf8_codepoint(p, &len);
            if (codepoint != -1)
                increment_codepoint(codepoint);
            p += len;
        }

        print_table(false);

        // Create the huffmannode container
        HuffmanNode *container = init_huffmannode_container();

        // Create the priority_queue
        PriorityQueue pq;
        init_priority_queue(&pq, container);
        print_priority_queue(&pq);

        // Create the huffman_tree
        HuffmanTree rootTree = create_huffman_tree(&pq);
        print_huffman_tree(rootTree.root, 0);

        // Create the code prefixes
        init_create_code_prefixes(rootTree);

        print_table(true);

        char *fileNameNoExtension = remove_txt_file_extension(argv[1]);
        char *newName = concat(fileNameNoExtension, "-compressed.txt");
        printf("%s\n", newName);
        FILE *compressedFile = fopen(newName, "wb");
        free(newName);
        free(fileNameNoExtension);
        if (compressedFile == NULL) {
            perror("Could not write to file");
            exit(1);
        }

        // Write the amount of chars
        fprintf(compressedFile, "%lld\n", nodeCount);

        // Write the codepoints, prefix length, prefix then new line char

        for (int i = 0; i < MAX_TABLE_SIZE; i++) {
            for (Node *n = table[i]; n; n = n->next) {
                fprintf(compressedFile, "%u %lu %u ", n->codepoint, n->freq, n->prefixSize);
                for (int j = 0; j < n->prefixSize; j++) {
                    fprintf(compressedFile, "%u", n->prefix[j]);
                }
                fprintf(compressedFile, "\n");
            }
        }

        char *orgFilePointer = file;

        write_bytes_outputfile(compressedFile, orgFilePointer);

        fclose(compressedFile);

        // Free the char buffer of file
        free(file);
        // Free the Huffmannode Container
        free(container);
    } else if (argc == 3) {
        if (strcmp(argv[2], "-o") == 0) {
            printf("Use %s -h for help", argv[0]);
            return 0;
        }
    } else if (argc == 4) {
        if (strcmp(argv[2], "-o") == 0) {
            // TODO same but with new namef
        }
    } else {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    return 0;
}