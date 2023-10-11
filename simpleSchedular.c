#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Process
{
    int pid;
    char* name;
    char* state;
    int wait;
    int execution_time;
} Process;

typedef struct Node
{
    Process data;
    struct Node* next;
} Node;

typedef struct Queue
{
    Node* front;
    Node* rear;
    int size;
} Queue;

void initializeQueue(Queue* queue) {
    queue->front = queue->rear = NULL;
    queue->size = 0;
}

bool isQueueEmpty(Queue* queue) {
    return (queue->front == NULL);
}

Node* createNode(Process data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void enqueue(Queue* queue, Process data) {
    Node* newNode = createNode(data);

    if (isQueueEmpty(queue)) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }

    queue->size++;
}

void dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) {
        printf("Queue is empty, cannot dequeue.\n");
        return;
    }

    Node* temp = queue->front;

    if (queue->front == queue->rear) {
        queue->front = queue->rear = NULL;
    } else {
        queue->front = queue->front->next;
    }

    free(temp);
    queue->size--;
}

Process front(Queue* queue) {
    if (isQueueEmpty(queue)) {
        printf("Queue is empty, cannot get front element.\n");
        exit(EXIT_FAILURE);
    }
    return queue->front->data;
}

int getSize(Queue* queue) {
    return queue->size;
}

void freeQueue(Queue* queue) {
    while (!isQueueEmpty(queue)) {
        dequeue(queue);
    }
}

int main() {
    while (1)
    {
        /* code */
    }
    

    return 0;
}
