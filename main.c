#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <stdbool.h>

// Struct definitions for the graph
typedef struct NODE {
    int v;            // Destination vertex of the edge
    float w;          // Weight of the edge
    struct NODE *next; // Pointer to the next node
} NODE;

typedef struct VERTEX {
    int index;       // Index of the vertex
    float key;       // Key value used in the priority queue
    struct VERTEX *pi; // Predecessor vertex in the shortest path
    NODE *adj;       // Adjacency list
} VERTEX;

typedef struct GRAPH {
    int num_vertices; // Number of vertices in the graph
    VERTEX *V;        // Array of vertices
} GRAPH;

// Function prototypes
GRAPH* createGraph(int n);
void addEdge(GRAPH *G, int u, int v, float w);
GRAPH* readGraph(const char *filename, int flag);
void dijkstra(GRAPH *G, VERTEX *s);
void printAdj(GRAPH *G);
void freeGraph(GRAPH *G);
void handleCommands(GRAPH *G);

// Function to create a graph with n vertices
GRAPH* createGraph(int n) {
    GRAPH *G = (GRAPH *)malloc(sizeof(GRAPH));
    G->num_vertices = n;
    G->V = (VERTEX *)malloc(n * sizeof(VERTEX));
    for (int i = 0; i < n; i++) {
        G->V[i].index = i;
        G->V[i].key = FLT_MAX;
        G->V[i].pi = NULL;
        G->V[i].adj = NULL;
    }
    return G;
}

// Function to add an edge from u to v with weight w
void addEdge(GRAPH *G, int u, int v, float w) {
    NODE *newNode = (NODE *)malloc(sizeof(NODE));
    newNode->v = v;
    newNode->w = w;
    newNode->next = G->V[u].adj;
    G->V[u].adj = newNode;
}

// Function to add an edge from u to v with weight w, appending to the end of the list
void addEdgeReverse(GRAPH *G, int u, int v, float w) {
    NODE *newNode = (NODE *)malloc(sizeof(NODE));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->v = v;
    newNode->w = w;
    newNode->next = NULL; // Set newNode's next pointer to NULL initially

    // If adj list is empty, make newNode the head
    if (G->V[u].adj == NULL) {
        G->V[u].adj = newNode;
    } else {
        // Traverse to the end of the adjacency list for vertex u
        NODE *current = G->V[u].adj;
        while (current->next != NULL) {
            current = current->next;
        }
        // Append newNode at the end of the list
        current->next = newNode;
    }
}

// Function to read the graph from a file
GRAPH* readGraph(const char *filename, int flag) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    int n, m;
    fscanf(file, "%d %d", &n, &m);
    GRAPH *G = createGraph(n);
    for (int i = 0; i < m; i++) {
        int ignore;
        int u, v;
        float w;
        fscanf(file, "%d %d %d %f",&ignore, &u, &v, &w);
        if(flag==1) {
            addEdge(G, u - 1, v - 1, w); // Adjust for 0-based indexing in the graph
        }
        else {
            addEdgeReverse(G, u-1,v-1,w);
        }
    }
    fclose(file);
    return G;
}

// Structure for the min heap
typedef struct {
    VERTEX **data;
    int size;
    int capacity;
} MIN_HEAP;

// Function prototypes for heap operations
MIN_HEAP* createMinHeap(int capacity);
void swap(VERTEX **a, VERTEX **b);
void minHeapify(MIN_HEAP *heap, int idx);
void insertMinHeap(MIN_HEAP *heap, VERTEX *v);
VERTEX* extractMin(MIN_HEAP *heap);
int isEmpty(MIN_HEAP *heap);

// Function to create a minimum heap with given capacity
MIN_HEAP* createMinHeap(int capacity) {
    MIN_HEAP *heap = (MIN_HEAP *)malloc(sizeof(MIN_HEAP));
    heap->data = (VERTEX **)malloc(capacity * sizeof(VERTEX *));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// Function to swap two vertices in the heap
void swap(VERTEX **a, VERTEX **b) {
    VERTEX *temp = *a;
    *a = *b;
    *b = temp;
}

// Function to heapify the heap at given index
void minHeapify(MIN_HEAP *heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    if (left < heap->size && heap->data[left]->key < heap->data[smallest]->key)
        smallest = left;
    if (right < heap->size && heap->data[right]->key < heap->data[smallest]->key)
        smallest = right;
    if (smallest != idx) {
        swap(&heap->data[smallest], &heap->data[idx]);
        minHeapify(heap, smallest);
    }
}

// Function to insert a vertex into the heap
void insertMinHeap(MIN_HEAP *heap, VERTEX *v) {
    heap->size++;
    int i = heap->size - 1;
    heap->data[i] = v;
    while (i != 0 && heap->data[(i - 1) / 2]->key >= heap->data[i]->key) {
        swap(&heap->data[i], &heap->data[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

// Function to extract the minimum vertex from the heap
VERTEX* extractMin(MIN_HEAP *heap) {
    if (heap->size == 0)
        return NULL;
    VERTEX *root = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    minHeapify(heap, 0);
    return root;
}

int isEmpty(MIN_HEAP *heap) {
    return heap->size == 0;
}

void initSingleSource(GRAPH *G, VERTEX *s) {
    for (int i = 0; i < G->num_vertices; i++) {
        G->V[i].key = FLT_MAX;
        G->V[i].pi = NULL;
    }
    s->key = 0;
}

void relax(VERTEX *u, VERTEX *v, float w) {
    if (v->key >= u->key + w) {
        v->key = u->key + w;
        v->pi = u;
    }
}

void dijkstra(GRAPH *G, VERTEX *s) {
    initSingleSource(G, s);
    printf("Starting Dijkstra's algorithm from vertex %d\n", s->index + 1); // +1 for 1-based index
    MIN_HEAP *Q = createMinHeap(G->num_vertices);
    for (int i = 0; i < G->num_vertices; i++) {
        insertMinHeap(Q, &G->V[i]);
    }
    while (!isEmpty(Q)) {
        VERTEX *u = extractMin(Q);
        NODE *adj = u->adj;
        while (adj != NULL) {
            relax(u, &G->V[adj->v], adj->w);
            adj = adj->next;
        }
    }
    free(Q->data);
    free(Q);
}

void printAdj(GRAPH *G) {
    for (int i = 0; i < G->num_vertices; i++) {
        NODE *adj = G->V[i].adj;
        printf("ADJ[%d]:-->", i + 1); // Assuming 1-based indexing
        while (adj != NULL) {
            printf("[%d %d: %.2f]", i + 1, adj->v + 1, adj->w); // Assuming 1-based indexing
            adj = adj->next;
            if (adj != NULL) {
                printf("-->");
            }
        }
        printf("\n");
    }
}

// Function to free the memory allocated for the graph
void freeGraph(GRAPH *G) {
    for (int i = 0; i < G->num_vertices; i++) {
        NODE *adj = G->V[i].adj;
        while (adj != NULL) {
            NODE *temp = adj;
            adj = adj->next;
            free(temp);
        }
    }
    free(G->V);
    free(G);
}

// Function to print the menu for user commands
void printMenu() {
    printf("Homework 5 - CSE 310\n");
    printf("1. Read the graph from the file (enter the file name, D or UD and the flag)\n");
    printf("2. Print ADJ\n");
    printf("3. Single Source, enter the starting node number\n");
    printf("4. Single Pair, enter the starting node and ending node numbers\n");
    printf("5. Print Path, enter the starting node and ending node numbers\n");
    printf("6. Print Length, enter the starting node and ending node numbers\n");
    printf("7. Print Path, enter the starting node and ending node numbers\n");
    printf("8. Stop\n");
    printf("Enter options 1 - 8:\n");
}

void printPath(VERTEX *v) {
    if (v->pi != NULL) {
        printPath(v->pi);
        printf("-->");
    }
    printf("[%d: %.2f]", v->index + 1, v->key);
}

// Function to handle user commands for graph operations
void handleCommands(GRAPH *G) {
    int option;
    int s, t;
    char filename[50], type[3];
    int flag;
    while (1) {
        printMenu();
        scanf("%d", &option);

        switch (option) {
            case 1:
                printf("Enter the file name, type (D/UD), and flag (1/2): ");
                scanf("%s %s %d", filename, type, &flag);
                G = readGraph(filename, flag);
                break;
            case 2:
                if (G == NULL) {
                    printf("Error: Graph not initialized. Please read the graph first (option 1).\n");
                } else {
                    printAdj(G);
                }
                break;
            case 3:
                printf("Enter the starting node number: ");
                scanf("%d", &s);
                dijkstra(G, &G->V[s - 1]);
                break;
            case 4:
                printf("Enter the starting node and ending node numbers: ");
                scanf("%d %d", &s, &t);
                dijkstra(G, &G->V[s - 1]);
                printf("Shortest path length: %.2f\n", G->V[t - 1].key);
                break;
            case 5:
                printf("Enter the starting node and ending node numbers: ");
                scanf("%d %d", &s, &t);
                printPath(&G->V[t-1]);
                printf("\n");
                break;
            case 6:
                printf("Enter the starting node and ending node numbers: ");
                scanf("%d %d", &s, &t);
                printf("%.2f\n", G->V[t - 1].key);
                break;
            case 7:
                printf("Enter the starting node and ending node numbers: ");
                scanf("%d %d", &s, &t);
                printPath(&G->V[t - 1]);
                printf("\n");
                break;
            case 8:
                freeGraph(G);
                exit(0);
            default:
                printf("Invalid option.\n");
        }
    }
}

// Main function to initialize graph and handle commands
int main() {
    GRAPH *G = NULL;
    handleCommands(G);
    return 0;
}

