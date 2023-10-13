#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 1024  // Adjust this as needed

typedef struct Process
{
    int pid;
    char name[100];
    char state[100];
    int wait;
    int execution_time;
} Process;

typedef struct Queue
{
    int front, rear, size;
    unsigned capacity;
    Process array[0]; // Flexible array member
} Queue;

Queue* initQueue(unsigned capacity) {
    Queue* queue = malloc(sizeof(Queue) + sizeof(Process) * capacity);
    queue->capacity = capacity;
    queue->front = queue->rear = -1;
    queue->size = 0;
    return queue;
}

int isFull(Queue* queue) {
    return queue->size == queue->capacity;
}

int isEmpty(Queue* queue) {
    return queue->size == 0;
}

void enqueue(Queue* queue, Process data) {
    if (isFull(queue)) {
        printf("Queue is full. Cannot enqueue.\n");
        return;
    }

    if (queue->rear == -1)
        queue->front = queue->rear = 0;
    else
        queue->rear = (queue->rear + 1) % queue->capacity;

    queue->array[queue->rear] = data;
    queue->size++;
}

Process dequeue(Queue* queue) {
    Process empty = {0, "", "", 0, 0}; // You might want to define a specific "empty" state.
    if (isEmpty(queue))
        return empty;

    Process data = queue->array[queue->front];
    if (queue->front == queue->rear)
        queue->front = queue->rear = -1;
    else
        queue->front = (queue->front + 1) % queue->capacity;

    queue->size--;
    return data;
}

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    Queue* shared_memory = (Queue*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    while (!isEmpty(shared_memory)) {
        Process p = dequeue(shared_memory);
        printf("Process ID: %d\n", p.pid);
        printf("Name: %s\n", p.name);
        printf("State: %s\n", p.state);
        printf("Wait Time: %d\n", p.wait);
        printf("Execution Time: %d\n\n", p.execution_time);
    }

    close(shm_fd);

    return 0;
}
