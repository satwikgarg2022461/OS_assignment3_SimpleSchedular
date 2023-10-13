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
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);

    Queue* queue = initQueue((SHM_SIZE - sizeof(Queue)) / sizeof(Process));

    Process p1 = {1, "Process 1", "Ready", 0, 10};
    Process p2 = {2, "Process 2", "Running", 5, 20};

    enqueue(queue, p1);
    enqueue(queue, p2);

    Queue* shared_memory = (Queue*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    memcpy(shared_memory, queue, sizeof(Queue) + sizeof(Process) * queue->capacity);

    if(fork()==0)
    {
        execl("./consumer","consumer",NULL);
    }

    close(shm_fd);

    return 0;
}
