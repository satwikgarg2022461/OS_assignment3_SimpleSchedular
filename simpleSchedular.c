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
#define l "/my_shared_memory"
#define SIZE 100
#define SHM_SIZE 1024  
#define MAX_PROCESSES 100

int running =1;

//..............Signal handler for scheduler
void sig_handler(int sig){
    if (sig==SIGINT){
        running=0;
    }
}


typedef struct Process
{
    int pid;
    char name[100];
    char state[100];
    struct timeval start_time;
    struct timeval end_time;
    long long wait;
    int execution_time;
} Process;


//............Structure and functions of Queue
typedef struct Queue
{
    int front, rear, size;
    unsigned capacity;
    Process array[0]; 
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

    queue->array[queue->rear] = *data; 
    queue->size++;
}

Process dequeue(Queue* queue) {
    Process empty = {0, "", "", 0, 0}; 
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

sem_t s;

//.........keeping track of completed processes
Process history[MAX_PROCESSES];
int no_of_history=0;

bool check_array(Process p)
{
    if(no_of_history == 0)
    {
        return true;
    }
    for(int i=0;i<no_of_history;i++)
    {
        if(p.pid == history[i].pid)
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    signal(SIGINT,sig_handler);
    sem_init(&s,1,1);

    //...........Shared memory with dummy_main.h
    int shm_fd1 = shm_open(l, O_RDWR | O_CREAT, 0666);
    if (shm_fd1 == -1) {
        perror("shm_open error\n");
        return 1;
    }
    ftruncate(shm_fd1, SIZE);
    int* shared_memory1 = (int*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    if (shared_memory1 == MAP_FAILED) {
        perror("mmap error\n");
        return 1;
    }
    *shared_memory1 = 100;
    int NCPU = atoi(argv[1]);
    int TSLICE = atoi(argv[2])/1000;
    //............Shared memory with simpleshell
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open error\n");
        return 1;
    }
    Queue* shared_memory = (Queue*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap error\n");
        return 1;
    }
    //...............Main scheduler algorithm
    while (running)
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
                sem_wait(&s);
                Process p = dequeue(shared_memory);
                sem_post(&s);
                if(check_array(p))
                {
                    history[no_of_history] = p;
                    no_of_history++;
                }
                if (strcmp(p.state,"Ready")==0){
                    
                    kill(p.pid,SIGCONT);
                    p.execution_time+=TSLICE;
                    for(int j=0;j<no_of_history;j++)
                    {
                        if(history[j].pid == p.pid)
                        {
                            history[j].execution_time +=TSLICE; 
                        }
                    }
                    pro[i]=p;
                }

            }
            
            sleep(TSLICE);
            int result = *shared_memory1;
            for (int i=0;i<n;i++){
                if (result!=0){                    
                    strcpy(pro[i].state,"Ready");
                    kill(pro[i].pid,SIGSTOP);   
                    sem_wait(&s);                 
                    enqueue(shared_memory,&pro[i]);
                    sem_post(&s);                 
                    // printf("%d\n",pro[i].pid);
                }
                else{
                    *shared_memory1=1;
                    for(int j=0;j<no_of_history;j++)
                    {
                        if(history[j].pid == pro[i].pid)
                        {
                            gettimeofday(&(history[j].end_time),NULL); 
                        }
                    }
                }
            }
        }      
    }

    printf("\nScheduler terminated, printing info of all submitted child processes...\n");
    printf("PID\tName\tExecution Time\tWait Time(microsecond)\n");
    

    for(int i=0;i<no_of_history;i++)
    {
        long long start = history[i].start_time.tv_sec*1000000LL + history[i].start_time.tv_usec;
        long long end = history[i].end_time.tv_sec*1000000LL + history[i].end_time.tv_usec;
        
        long long total_time= end-start;
        int total_time_sec=total_time;
        history[i].wait=total_time_sec-(history[i].execution_time)*1000000;
        printf("%d\t%s\t\t%d\t%lld\n",
        history[i].pid,history[i].name,history[i].execution_time,history[i].wait);
        
    }
    
    munmap(shared_memory1, SIZE);
    unlink(l);
    close(shm_fd1);
    munmap(shared_memory,SHM_SIZE);
    unlink(SHM_NAME);
    close(shm_fd);
    sem_destroy(&s);

    return 0;
}
