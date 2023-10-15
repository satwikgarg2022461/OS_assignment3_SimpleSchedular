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
#define SHM_SIZE 1024  // Adjust this as needed
#define MAX_PROCESSES 100

int termination_check=0;
int running=1;

void my_handler(int signum){
    if (signum==SIGCHLD){
        termination_check=1;
    }
    if (signum==SIGINT){
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

Process process_info[MAX_PROCESSES];
Process history[MAX_PROCESSES];
int no_of_processes=0;
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
    // printf("hi\n");
    // printf("%d\n",atoi(argv[1]));
    // printf("%d\n",atoi(argv[2]));
    signal(SIGCHLD,my_handler);
    signal(SIGINT,my_handler);
    int shm_fd1 = shm_open(l, O_RDWR | O_CREAT, 0666);
    ftruncate(shm_fd1, SIZE);
    if (shm_fd1 == -1) {
        perror("shm_open");
        return 1;
    }

    // Map the shared memory segment to the address space of the process
    int* shared_memory1 = (int*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    if (shared_memory1 == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Read the integer from shared memory
    // printf("Value in shared memory1: %d\n", *shared_memory1);

    // Unmap the shared memory segment
    

    // Close the shared memory object
    


    int NCPU = atoi(argv[1]);
    int TSLICE = atoi(argv[2])/1000;
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    Queue* shared_memory = (Queue*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    while (running)
    {
        while (shared_memory->size>0) {
            // printf("shared size %d\n",shared_memory->size);
            int n;
            if (shared_memory->size<NCPU){
                n=shared_memory->size;
            }
            else{
                n=NCPU;
            }
            sleep(TSLICE);
            Process pro[n];
            for (int i=0;i<n;i++){
                // sem_wait(&shared_memory->sem2);
                Process p = dequeue(shared_memory);
                if(check_array(p))
                {
                    history[no_of_history] = p;
                    no_of_history++;
                }
                // sem_post(&shared_memory->sem);
                if (strcmp(p.state,"Ready")==0){
                    // int pid=p.pid+1;
                    // printf("\n");
                    // fflush(stdout);
                    // printf("Name: %s\n", p.name);
                    // printf("scheduler pid %d\n",p.pid);
                    // strcpy(p.state,"Running");
                    kill(p.pid,SIGCONT);
                    p.execution_time+=TSLICE;
                    for(int j=0;j<no_of_history;j++)
                    {
                        if(history[j].pid == p.pid)
                        {
                            history[j].execution_time +=TSLICE; 
                            // printf("Execution Time: %d\n\n", history[j].execution_time);
                        }
                    }
                    pro[i]=p;
                }

            }
            for (int i=0;i<n;i++){
                // int status;
                // int result = waitpid(pro[i].pid, &status, WNOHANG);
                // // int result =
                // // char command[256];
                // printf("kill pid %d\n",pro[i].pid);
                // // sprintf(command, "ps -p %d", pro[i].pid);
                // printf("kill status %d\n",result);
                int result = *shared_memory1;
                if (result==0){
                    // printf("hi\n");
                    strcpy(pro[i].state,"Ready");
                    kill(pro[i].pid,SIGSTOP);
                    // sem_wait(&shared_memory->sem);
                    enqueue(shared_memory,&pro[i]);
                    // printf("shared memory size enquueue%d\n",shared_memory->size);
                    *shared_memory1=1;
                    
                    // sem_post(&shared_memory->sem2);
                }
                else{
                    // kill(pro[i].pid,SIGINT);
                    // gettimeofday(&(pro[i].end_time),NULL);
                    
                    // process_info[no_of_processes]=pro[i];

                    for(int j=0;j<no_of_history;j++)
                    {
                        if(history[j].pid == pro[i].pid)
                        {
                            // history[j].execution_time +=TSLICE;
                            gettimeofday(&(history[j].end_time),NULL); 
                        }
                    }

                    // no_of_processes++;

                }
            }
                // printf("term %d\n",termination_check);
        }
            // printf("size of queue scheduler %d\n",shared_memory->size);





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

    printf("\nShell terminated, printing info of all submitted child processes...\n");
    printf("PID\tName\tExecution Time\tWait Time(microsecond)\n");
    // for (int i = 0; i < no_of_processes; i++) 
    // {
    //     long long start = process_info[i].start_time.tv_sec*1000000LL + process_info[i].start_time.tv_usec;
    //     long long end = process_info[i].end_time.tv_sec*1000000LL + process_info[i].end_time.tv_usec;
    //     long long total_time= end-start;
    //     int total_time_sec=total_time/1000000;
    //     process_info[i].wait=total_time_sec-process_info[i].execution_time;
    //     printf("%d\t%s\t\t%d\t%d\n",
    //         process_info[i].pid,process_info[i].name,process_info[i].execution_time,process_info[i].wait);
    // }

    for(int i=0;i<no_of_history;i++)
    {
        long long start = history[i].start_time.tv_sec*1000000LL + history[i].start_time.tv_usec;
        long long end = history[i].end_time.tv_sec*1000000LL + history[i].end_time.tv_usec;
        
        long long total_time= end-start;
        int total_time_sec=total_time;
        history[i].wait=total_time_sec-(history[i].execution_time)*1000000;
        printf("%d\t%s\t\t%d\t%lld\n",
        history[i].pid,history[i].name,history[i].execution_time,history[i].wait);
        // printf("start time %lld\n",start);
        // printf("end time %lld\n",end);
        // printf("total time%lld \n",total_time);
        // printf("total time sec %d \n",total_time_sec);
        // printf("wait time %lld",history[i].wait," mircosecond");
    }

        // sem_wait(&shared_memory->sem);

    sem_destroy(&shared_memory->sem);
    
    munmap(shared_memory1, SIZE);
    unlink(l);

    close(shm_fd);

    return 0;
}
