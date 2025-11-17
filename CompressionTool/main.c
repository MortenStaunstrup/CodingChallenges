#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TABLE_SIZE 65536


typedef enum kind {
    LEAF_NODE,
    INTERNAL_NODE
} kind;

typedef struct Node {
    uint32_t key;
    unsigned long val;
    struct Node* next;
} Node;

// The hash map
// Each index can be a linked list to handle hash collisions (called buckets)
Node* table[TABLE_SIZE];
long long tableCount = 0;

typedef struct HuffmanNode {
    uint32_t key;
    unsigned long val;
    unsigned long weight;
    enum kind kind;
    struct HuffmanNode* left, *right;
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
HuffmanTree delete(PriorityQueue *pq);
int isEmpty(PriorityQueue *pq);

// Knuth's hash function
// Returns index
static inline size_t hash32(uint32_t x) {return (x * 2654435761u) % TABLE_SIZE;}

int main(const int argc, char* argv[]) {

    if (argc == 1) {
        printf("Usage: <exe file> <txt file>");
    }

    if (argc == 2) {
        char* text = handle_file(argv[1]);
        const char *p = text;
        uint32_t cp;

        // Main loop to increase frequency of codepoint
        while (1) {
            int r = utf8_next(&p, &cp);
            if (r == 1) break;
            if (r == -1) { p++; continue; } // skip invalid byte
            incr(cp);
        }
        print_map();
        printf("Frequency table count:%lld\n", tableCount);

        HuffmanNode *array = create_huffman_node_array();
        print_huffman_node_array(array);

        sort_huffman_node_array(array);
        print_huffman_node_array(array);

        PriorityQueue pq;
        init_priorityqueue(&pq, array);
        display_queue(&pq);

        free(array);
    }

    return 0;
}

int isEmpty(PriorityQueue *pq) {
    return pq->size == 0;
}

HuffmanTree delete(PriorityQueue *pq){
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
        // Check if the left or right node is smaller than than the current node, then swap to keep integrity of min-heap
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
    printf("Displaying priority queue\n");
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
            array[index].val = n->val;
            array[index].kind = LEAF_NODE;
            array[index].weight = n->val;
            array[index].left = nullptr;
            array[index].right = nullptr;
            index++;
        }
    }
    return array;
}

void print_huffman_node_array(const HuffmanNode* array) {
    for (size_t i = 0; i < tableCount; i++) {
        printf("Index: %llu, key: %u, value: %lu\n", i, array[i].key, array[i].val);
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
    char* buffer = (char*)malloc(fsize + 1);
    buffer[fsize] = '\0';
    fread(buffer, 1, fsize, f);
    fclose(f);
    return buffer;
}


void incr(uint32_t key) {
    size_t idx = hash32(key);
    // Finds the key in the hashmap, and increments the value
    for (Node *n = table[idx]; n; n=n->next) {
        if (n->key == key) { n->val++; return;}
    }
    // If the key does not exist, create it in the linked list at the head, in the index
    Node *n = malloc(sizeof *n);
    if (!n) exit(1);

    n -> key = key; n -> val = 1; n -> next = table[idx]; table[idx] = n;
    tableCount++;
}

void print_map(void) {
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        for (Node *n = table[i]; n; n = n->next) {
            printf("U+%04X : %lu\n", n->key, n->val);
        }
    }
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