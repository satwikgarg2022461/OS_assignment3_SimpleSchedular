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

#define SHM_NAME "a"
#define SHM_SIZE 1024  // Adjust this as needed

int termination_check=0;

void my_handler(int signum){
    if (signum==SIGCHLD){
        termination_check=1;
    }
}

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
    sem_t sem,sem2;
} Queue;

void initQueue(Queue* q, unsigned capacity) {
    q->capacity = capacity;
    q->front = q->rear = -1;
    q->size = 0;
}

int isFull(Queue* queue) {
    return queue->size == queue->capacity;
}

int isEmpty(Queue* queue) {
    return queue->size == 0;
}

void enqueue(Queue* queue, Process* data) {
    if (isFull(queue)) {
        printf("Queue is full. Cannot enqueue.\n");
        return;
    }

    if (queue->rear == -1)
        queue->front = queue->rear = 0;
    else
        queue->rear = (queue->rear + 1) % queue->capacity;

    queue->array[queue->rear] = *data; // Copy the Process data into the queue
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
sem_t* sema;
int main(int argc, char **argv) {
    // printf("hi\n");
    // printf("%d\n",atoi(argv[1]));
    // printf("%d\n",atoi(argv[2]));
    signal(SIGCHLD,my_handler);
    int NCPU = atoi(argv[1]);
    int TSLICE = atoi(argv[2])/1000;
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    Queue* shared_memory = (Queue*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    while (1)
    {
        while (shared_memory->size>0) {
            int n;
            if (shared_memory->size<NCPU){
                n=shared_memory->size;
            }
            else{
                n=NCPU;
            }
            Process pro[n];
            for (int i=0;i<n;i++){
                Process p = dequeue(shared_memory);
                if (strcmp(p.state,"Ready")){
                    // int pid=p.pid+1;
                    printf("%s is running\n",p.name);
                    printf("scheduler pid %d\n",p.pid);
                    strcpy(p.state,"Running");
                    kill(p.pid,SIGCONT);
                    p.execution_time+=TSLICE;
                    pro[i]=p;
                }
                sleep(TSLICE);
                for (int i=0;i<n;i++){
                     int status;
                    int result = waitpid(pro[i].pid, &status, WNOHANG);
                    printf("kill pid %d\n",pro[i].pid);
                    printf("kill status %d\n",result);
                    if (result==0){
                        kill(pro[i].pid,SIGSTOP);
                        enqueue(shared_memory,&pro[i]);
                    }
                    // else{
                    //     kill(pro[i].pid,SIGINT);
                    // }
                }
                printf("term %d\n",termination_check);
            }
            printf("size of queue scheduler %d\n",shared_memory->size);





            // sem_wait(&shared_memory->sem);
            // printf("shared mem size schedular %d\n",shared_memory->size);
            // // sem_wait(&shared_memory->sem);
            // Process p = dequeue(shared_memory);
            // // sem_wait(&shared_memory->sem);
            // printf("Name: %s\n", p.name);
            // printf("State: %s\n", p.state);
            // printf("Wait Time: %d\n", p.wait);
            // printf("Execution Time: %d\n\n", p.execution_time);
            // printf("Process ID: %d\n", p.pid);
            // kill(getpid(),SIGINT);
        // fflush(stdout);
        // sem_wait(&shared_memory->sem);
        }
        // sem_wait(&shared_memory->sem);
    }

    sem_destroy(&shared_memory->sem);
    
    

    close(shm_fd);

    return 0;
}
