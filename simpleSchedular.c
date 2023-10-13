#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h> 
#include <sys/shm.h>
#include<sys/mman.h>
#include<stdbool.h>
#include<semaphore.h>

typedef struct Process
{
    int pid;
    char name[100];
    char state[100];
    int wait;
    int execution_time;
} Process;

// ------shared memory
int SIZE = 4096;
char* name = "shell to scheduler";
int shm_fd;
sem_t* sema;

typedef struct Node
{
    Process* data;
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

Node* createNode(Process* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void enqueue(Queue* queue, Process* data) {
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

Process* front(Queue* queue) {
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

void printQueue(Queue* queue) {
    Node* current = queue->front;

    if (isQueueEmpty(queue)) {
        printf("Queue is empty.\n");
        return;
    }

    printf("Queue Contents:\n");
    while (current != NULL) {
        printf("PID: %d, Name: %s, State: %s, Wait: %d, Execution Time: %d\n",
               current->data->pid, current->data->name, current->data->state,
               current->data->wait, current->data->execution_time);
        current = current->next;
    }
}
Queue* q;

int main() {
    printf("hi");
    fflush(stdout);
    q=(Queue*) malloc(sizeof(Queue));
    
    Process* ptr=(Process*)malloc(sizeof(Process));
    // sleep(10);
    sema = sem_open("a", O_CREAT, 0666,0);
    sem_wait(sema);
    printf("hi2\n");
    fflush(stdout);
    shm_fd = shm_open(name, O_RDWR, 0666);
    q = (Queue*)mmap(0, sizeof(Queue), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
    // printf("amdhjksahdjksa");
    printf("sizejadfjkafjkahsjkf %d\n",getSize(q));
    fflush(stdout);
    
    // printf("size %d\n",getSize(q));
    // fflush(stdout);
    printQueue(q);
    fflush(stdout);
    // printf("ptr schedular %p\n",ptr);
    // // printf("ptr->p schedular %p\n",ptr->p);
    // printf("%p \n",(void *)ptr->name);
    // fflush(stdout);
    // // char* s=strcpy(s, ptr->state);
    // printf("%s \n",ptr->name);
    // fflush(stdout);
    // char *a=ptr->p->name;
    // puts(a);
    
    // while (1)
    // {
    //     /* code */


    // }
    

    return 0;
}
