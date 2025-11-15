#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TABLE_SIZE 65536

typedef enum NODE_KIND {
    LEAF_NODE,
    INTERNAL_NODE
} NODE_KIND;


typedef struct Node {
    uint32_t key;
    unsigned long val;
    struct Node* next;
} Node;

typedef struct HuffBaseNode {
    unsigned long val;
    NODE_KIND kind;
    uint32_t key;
    unsigned long weight;
    struct HuffBaseNode *left, *right;
} HuffBaseNode;

typedef struct HuffTree {
    struct HuffBaseNode root;
    unsigned long weight;
} HuffTree;

typedef struct PriorityQueue {
    HuffTree* trees;
    int size;
} PriorityQueue;

// The hash map
// Each index can be a linked list to handle hash collisions (called buckets)
Node* table[TABLE_SIZE];
long long tableCount = 0;


int isEmpty(PriorityQueue *pq) {
    return pq->size == 0;
}

void swap(HuffTree *a, HuffTree *b) {
    HuffTree temp = *a;
    *a = *b;
    *b = temp;
}

void insert(PriorityQueue *pq, HuffTree tree) {
    if (pq->size >= tableCount) {
        printf("Queue is full\n");
        return;
    }
    int i = pq->size++;
    pq->trees[i] = tree;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->trees[i].weight < pq->trees[parent].weight) {
            swap(&pq->trees[i], &pq->trees[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

HuffTree delete(PriorityQueue *pq){
    if (isEmpty(pq)) {
        printf("Queue is empty\n");
        exit(-1);
    }
    HuffTree minItem = pq->trees[0];
    pq->trees[0] = pq->trees[--pq->size];

    int i = 0;
    while (true) {
        int left = 2 * i + 1, right = 2 * i + 2, smallest = i;
        if (left < pq->size && pq->trees[left].weight < pq->trees[smallest].weight)
            smallest = left;
        if (right < pq->size && pq->trees[right].weight < pq->trees[smallest].weight)
            smallest = right;
        if (smallest != i) {
            swap(&pq->trees[i], &pq->trees[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
    return minItem;
}

HuffTree peek(PriorityQueue *pq) {
    if (isEmpty(pq)) {
        printf("Queue is empty!\n");
        exit(EXIT_FAILURE);
    }
    return pq->trees[0];
}

void display(PriorityQueue *pq) {
    printf("Displaying min heap priority queue\n");
    printf("Current queue size: %u\n", pq->size);
    for (int i = 0; i < pq->size; i++) {
        printf("Data: %d, Priority: %lu\n", pq->trees[i].root.key, pq->trees[i].weight);
    }
}

void PrioQueueInit(PriorityQueue *pq, HuffBaseNode *array) {
    pq->size = 0;
    pq->trees = malloc(sizeof(HuffTree) * tableCount);
    for (int i = 0; i < tableCount; i++) {
        HuffTree tree;
        tree.weight = array[i].val;
        tree.root = array[i];
        insert(pq, tree);
    }
}

// TODO Create binary tree

static HuffTree build_tree(HuffBaseNode* array) {
    HuffTree tmp1, tmp2, tmp3;

    // Keep going until there is only 1 or no more left
    HuffBaseNode* cur = array;
    long index = 0;
    while (index < tableCount - 1) {
        tmp1.root = array[index];
        tmp1.weight = array[index].weight;
        tmp1.root.kind = LEAF_NODE;
        index++;

        tmp2.root = array[index];
        tmp1.weight = array[index].weight;
        tmp1.root.kind = LEAF_NODE;
        index++;

    }
}

// Knuth's hash function
// Returns index
static inline size_t hash32(uint32_t x) {return (x * 2654435761u) % TABLE_SIZE;}

// If collisions happen (more than one key hashes to the same index) they are both stored in the same bucket (linked list)
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

// Simple print, prints all the values
// Traverses the linked list in the index
void print_map(void) {
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        for (Node *n = table[i]; n; n = n->next) {
            printf("U+%04X : %lu\n", n->key, n->val);
        }
    }
}

char* Handle_file(char* text) {
    FILE *file = fopen(text, "rb");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    char *buffer = malloc(size + 1);
    buffer[size] = '\0';
    fread(buffer, 1, size, file);
    fclose(file);
    return buffer;
}

int utf8_next(const char **p, uint32_t *out) {
    const unsigned char *s = (const unsigned char *) *p;
    if (*s == 0) return 1;
    if (*s < 0x80) { *out = *s++; *p = (const char*)s; return 0;}
    int len = 0;
    uint32_t cp = 0;
    if ((s[0] & 0xC0) != 0x80) { len = 2; cp = s[0] & 0x1F; }
    else if ((s[0] & 0xF0) == 0xE0) { len = 3; cp = s[0] & 0x0F; }
    else if ((s[0] & 0xF8) == 0xF0) { len = 4; cp = s[0] & 0x07; }
    else return -1;
    for (int i = 1; i < len; ++i) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    *p = (const char*)(s + len);
    *out = cp;
    return 0;
}

HuffBaseNode* create_huffnode_array() {
    HuffBaseNode* huff_buffer = malloc(tableCount * sizeof(HuffBaseNode));
    if (!huff_buffer) return NULL;

    long bufferIndex = 0;
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        for (Node *n = table[i]; n; n = n->next) {
            huff_buffer[bufferIndex].key = n -> key; huff_buffer[bufferIndex].val = n -> val; huff_buffer[bufferIndex].weight = n -> val;
            huff_buffer[bufferIndex].kind = LEAF_NODE;
            bufferIndex++;
        }
    }

    printf("Huff node array created\n");
    for (size_t b = 0; b < tableCount; b++) {
        printf("Key: %u Value: %lu\n", huff_buffer[b].key, huff_buffer[b].val);
    }
    return huff_buffer;
}


int compare(const void* a, const void* b) {
    const HuffBaseNode* nodeA = (const HuffBaseNode*)a;
    const HuffBaseNode* nodeB = (const HuffBaseNode*)b;
    if (nodeA->weight < nodeB->weight) return -1;
    if (nodeA->weight > nodeB->weight) return 1;
    return 0;
}

void sort_huffnode_array(HuffBaseNode* buffer) {
    qsort(buffer, tableCount, sizeof(HuffBaseNode), compare);
    HuffBaseNode* p = buffer;

    printf("Huff node array sorted\n");
    for (size_t i = 0; i < tableCount; i++) {
        printf("Node: %i Value: %lu\n", p -> key, p -> val);
        p++;
    }
}

int main(const int argc, char* argv[]) {

    if (argc < 2) {
        printf("Usage: %s [FILE]\n", argv[0]);
    } else {
        char* result = Handle_file(argv[1]);
        const char *p = result;
        uint32_t cp;
        // Main loop to increase frequency of char
        while (1) {
            int r = utf8_next(&p, &cp);
            if (r == 1) break;
            if (r == -1) { p++; continue; } // skip invalid byte
            incr(cp);
        }
        print_map();
        printf("Frequency table count:%lld\n", tableCount);

        // Create the array with key to char, and frequency as val and weight
        HuffBaseNode* nodeArray = create_huffnode_array();
        if (nodeArray == NULL) {
            perror("Could not create huffnode array");
            return -1;
        }

        // Sort the values
        sort_huffnode_array(nodeArray);

        for (size_t i = 0; i < tableCount; i++) {
            printf("%lu ", nodeArray[i].weight);
        }

        // Create the min heap priority queue
        PriorityQueue pq;
        PrioQueueInit(&pq, nodeArray);
        display(&pq);



        free(pq.trees);
        free(nodeArray);
        free(result);
    }


    return 0;
}