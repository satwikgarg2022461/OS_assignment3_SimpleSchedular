#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

typedef struct {
    int arr[MAX_SIZE_QUEUE];
    int front;
    int rear;
} Queue;

void initializeQueue(Queue* q) {
    q->front = -1;
    q->rear = -1;
}

int isFull(Queue* q) {
    return (q->rear == MAX_SIZE - 1);
}

int isEmpty(Queue* q) {
    return (q->front == -1);
}

void enqueue(Queue* q, int value) {
    if (isFull(q)) {
        printf("Queue is full. Cannot enqueue.\n");
        return;
    }

    if (isEmpty(q)) {
        q->front = 0;
    }

    q->rear++;
    q->arr[q->rear] = value;
}

int dequeue(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty. Cannot dequeue.\n");
        return -1; // Return a sentinel value indicating error
    }

    int value = q->arr[q->front];

    if (q->front == q->rear) {
        q->front = q->rear = -1; // Reset queue when last element is dequeued
    } else {
        q->front++;
    }

    return value;
}

int main() {
    Queue myQueue;
    initializeQueue(&myQueue);

    enqueue(&myQueue, 10);
    enqueue(&myQueue, 20);
    enqueue(&myQueue, 30);

    printf("Dequeued element: %d\n", dequeue(&myQueue));
    printf("Dequeued element: %d\n", dequeue(&myQueue));
    printf("Dequeued element: %d\n", dequeue(&myQueue));
    printf("Dequeued element: %d\n", dequeue(&myQueue)); // Trying to dequeue from an empty queue

    return 0;
}
