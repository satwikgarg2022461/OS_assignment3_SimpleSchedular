#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h> 
#define true 1
#define false 0

typedef int bool; 

// -------defining all varibles used 
#define MAX_INPUT_SIZE 1024
#define MAX_HISTORY_SIZE 100
#define MAX_PROCESSES 100
#define MAX_DIRECTORY_SIZE 1024

// -----------global varible for checking background process
int back_ground_check=0;


// -------------------------making queue


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

Queue q;

typedef struct ChildProcessInfo
{
    int pid;
    struct timeval start_time;
    struct timeval end_time;
}processinfo;

processinfo processes[MAX_PROCESSES];
int no_processes=0;


void record_process_info(pid_t pid, struct timeval start_time, struct timeval end_time) 
{
    if (no_processes < MAX_PROCESSES) {
        processes[no_processes].pid = pid;
        processes[no_processes].start_time = start_time;
        processes[no_processes].end_time = end_time;
        no_processes++;
    } else {
        fprintf(stderr, "Too many processes to track.\n");
    }
}

void print_process_info()
{
    printf("\nShell terminated, printing info of all child processes...\n");
    printf("PID\tStart Time\t\tEnd Time\t\tExecution Time\n");
    for (int i = 0; i < no_processes; i++) 
    {
        long int execution_time_sec = processes[i].end_time.tv_sec - processes[i].start_time.tv_sec;
        long int execution_time_usec = processes[i].end_time.tv_usec - processes[i].start_time.tv_usec;
        if (execution_time_usec < 0) 
        {
            execution_time_sec--;
            execution_time_usec += 1000000;
        }
        printf("%d\t%ld.%06ld\t%ld.%06ld\t%ld.%06ld\n",
            processes[i].pid,
            processes[i].start_time.tv_sec, processes[i].start_time.tv_usec,
            processes[i].end_time.tv_sec, processes[i].end_time.tv_usec,
            execution_time_sec, execution_time_usec);
    }
}

typedef struct CmdHistory
{
    char history[MAX_INPUT_SIZE];
}cmdHistory;

cmdHistory history[MAX_HISTORY_SIZE];
int no_history=0;

void addHistory(char *command)
{
    if (no_history < MAX_HISTORY_SIZE) 
    {
        strcpy(history[no_history].history, command);
        no_history++;
    } 
    else 
    {
        // If history is full, shift commands to make room for the new one
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) 
        {
            strcpy(history[i].history, history[i + 1].history);
        }
        strcpy(history[MAX_HISTORY_SIZE - 1].history, command);
    }
}



void print_history() 
{
    printf("Command History:\n");
    for (int i = 0; i < no_history; i++) 
    {
        printf("%d: %s\n", i + 1, history[i].history);
    }
}


static void my_handler(int signum) {
    if(signum == SIGINT) {
        print_process_info();
        exit(0);
    }
}



void print_statement() 
{
    char path[MAX_DIRECTORY_SIZE];
    if (getcwd(path, sizeof(path)) != NULL)
    {
        printf("SimpleShell:~%s$ ", path);

    } else 
    {
        perror("Error in getting path of the current directory");
    }
}

char* read_user_input() 
{
    

    char input[MAX_INPUT_SIZE];
    if (fgets(input, sizeof(input), stdin) == NULL) 
    {
        perror("Error in taking input");
        return NULL;
    }
    if(input[strlen(input) - 2] == '&')
    {
        // printf("hi");
        back_ground_check=1;
        input[strlen(input) - 2] = '\0';
    }
    input[strcspn(input, "\n")] = 0;
    return strdup(input);
}



int create_process_run(char *cmd)
{

    if (strcmp(cmd, "exit") == 0) 
    {
        print_process_info();
        return 0;
    }

    char* token = strtok(cmd, "|");
    int num_commands = 0;
    char* commands[MAX_INPUT_SIZE];

    
    while (token != NULL) 
    {
        commands[num_commands] = token;
        num_commands++;
        token = strtok(NULL, "|");
    }

    if(num_commands > 0)
    {
        // handling process with | 
        int pipe_fd[2];
        int prev_pipe = 0;

        struct timeval start_time;
        gettimeofday(&start_time, NULL);

        for(int i = 0; i < num_commands; i++)
        {
            
            if (pipe(pipe_fd) == -1)
            {
                perror("pipe");
                exit(1);
            }
            // creating fork

            int pid = fork();
            if (pid == -1)
            {
                printf("fork error\n");
                exit(0);
            }
            else if (pid == 0) 
            {
                // Child process
                close(pipe_fd[0]); // Close read end of the pipe

                if (i != 0)
                {
                    dup2(prev_pipe, STDIN_FILENO);
                    close(prev_pipe);
                }

                if (i != num_commands - 1)
                {
                    dup2(pipe_fd[1], STDOUT_FILENO);
                }

                char* args[MAX_INPUT_SIZE];
                int num_args = 0;

                char* token = strtok(commands[i], " ");

                while (token != NULL) 
                {
                    args[num_args] = token;
                    num_args++;
                    token = strtok(NULL, " ");
                }

                if(num_args == MAX_INPUT_SIZE)
                {
                    perror("Error");
                    exit(1);
                }

                args[num_args] = NULL;

                
                int check = execvp(args[0],args);
                if (check==-1)
                {
                    printf("Error\n");
                    exit(1);
                }

            } 

            else 
            {
                // Parent process
                // close(pipe_)
                close(pipe_fd[1]); // Close write end of the pipe

                if (i != 0)
                {
                    close(prev_pipe);
                }

                
                if(back_ground_check==0)
                { 
                    waitpid(pid, NULL, 0);
                }

                else
                {
                    printf("Background process started with PID: %d\n", pid);
                    back_ground_check=0;
                }

                if(i == num_commands - 1)
                {
                    struct timeval end_time;
                    gettimeofday(&end_time, NULL);

                    record_process_info(pid, start_time, end_time);
                }

                else
                {
                    struct timeval end_time;
                    gettimeofday(&end_time, NULL);

                    record_process_info(pid, start_time, end_time);

                    struct timeval start_time;
                    gettimeofday(&start_time, NULL);

                }
            }

            prev_pipe = pipe_fd[0];
        }
        close(prev_pipe);
    }

    else
    {
        // handling processes without |
        struct timeval start_time;
        gettimeofday(&start_time, NULL);

        


        // creating fork
        int pid = fork();

        if (pid ==-1)
        {
            printf("fork error\n");
            exit(0);
        }

        else if (pid == 0) 
        {
            // child process
            char* args[MAX_INPUT_SIZE];
            int num_args = 0;

            char* token = strtok(cmd, " ");
            while (token != NULL) 
            {
                args[num_args] = token;
                num_args++;
                token = strtok(NULL, " ");
            }

            if(num_args==MAX_INPUT_SIZE)
            {
                perror("Error");
                exit(1);
            }

            args[num_args] = NULL;
            
            int check = execvp(args[0],args);
            if (check==-1){
                printf("Error\n");
                exit(1);
            }
            else
            {
                // parent process
                if(back_ground_check==1)
                {
                    printf("Background process started with PID: %d\n", pid);
                    back_ground_check=0;
                }
                else
                {
                    
                    waitpid(pid, NULL, 0);
                }

                struct timeval end_time;
                gettimeofday(&end_time, NULL);
                record_process_info(pid,start_time,end_time);

            }
            
        }


    }    
    return 1; 


}

int launch(char* cmd)
{
    if (cmd == NULL) {
        return 0;
    }
    int status = create_process_run(cmd);
    return status;
}

// ------bonus 2
int read_sh(char* cmd) 
{
    int status = 1;
    int fd;
    fd = open(cmd, O_RDONLY);
    if(fd==-1)
    {
        printf("error in reading the file\n");
        return 1;
    }
    off_t f_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char *memory = (char*)malloc(f_size);
    if(memory == NULL)
    {
        printf("error in allocating memory\n");
        return 1;
    }
    ssize_t readData = read(fd, memory, f_size);
    if(readData == -1)
    {
        printf("error\n");
        return 1;
    }
    close(fd);

    char* token = strtok(memory, "\n");

    char* command[MAX_INPUT_SIZE];
    int i = 0;

    while (token != NULL) {
        if (token[0] != '#') {
            command[i] = token;
            i++;
        }
        token = strtok(NULL, "\n");
    }

    for (int j = 0; j < i; j++) {
        status=launch(command[j]);
    }

    free(memory);

    return status;
}


int submit(char* cmd)
{
    Process* p = (Process*) malloc(sizeof(Process));
    p->name = cmd;
    int check=fork();
    if(check < 0)
    {
        printf("Error in fork for submit");
    }
    else if(check == 0)
    {
        // --child process
        p->pid = getpid();
        // printf("ppid %d\n",getppid());
        // printf("GETPID %d\n",getpid());
        p->state="Ready";
        p->execution_time=0;
        p->wait=0;
        execl(cmd,cmd,NULL);
    }
    else
    {
        
    }
}



void shell_loop()
{
    int status;
    char* cmd;
    do {
        print_statement();
        cmd = read_user_input();

        addHistory(cmd);

        

        if (strcmp(cmd,"history")==0)
        {
            print_history();
            continue;
        }

        if(cmd[0] == 114)
        {
            // ---------read shell script 
            char* args[MAX_INPUT_SIZE];
            int num_args = 0;
            int c=0;
            int flag=0;

            char* token = strtok(cmd, " ");
            while (token != NULL) 
            {
                c++;
                args[num_args] = token;
                num_args++;
                token = strtok(NULL, " ");
            }

            if(num_args==MAX_INPUT_SIZE)
            {
                perror("Error");
                exit(1);
            }

            args[num_args] = NULL;

            if(c>2)
            {
                printf("Error\n");
                flag=1;
            }

            if(flag==0)
            {
                status=read_sh(args[1]);
            }
            else
            {
                status=1;
            }

            
            cmd=NULL;
        }

        if(cmd[0] == 115)
        {
            // ------------------submit-------------------
            char* args[MAX_INPUT_SIZE];
            int num_args = 0;
            int c=0;
            int flag=0;

            char* token = strtok(cmd, " ");
            while (token != NULL) 
            {
                c++;
                args[num_args] = token;
                num_args++;
                token = strtok(NULL, " ");
            }

            if(num_args==MAX_INPUT_SIZE)
            {
                perror("Error");
                exit(1);
            }

            args[num_args] = NULL;

            if(c>2)
            {
                printf("Error\n");
                flag=1;
            }

            if(flag==0)
            {
                // status=read_sh(args[1]);

                status = submit(args[1]);

                //  add your add here


            }
            else
            {
                status=1;
            }

            
            cmd=NULL;
        }

        if (cmd != NULL) {
            status = launch(cmd);
            free(cmd); 
        }
        
    } while (status);
}

int main() 
{
    // signal for ctrl + c
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    initializeQueue(&q);


    int shedular = fork();
    if(shedular < 0)
    {
        printf("Error in forking schedular");
        exit(1);
    }
    else if(shedular == 0)
    {
        int check=execlp("./simpleSchedular","simpleSchedular", NULL);
        if(check == -1)
        {
            printf("error in running simple schedular");
            exit(1);
        }
    }
    else{
        shell_loop();
    }
    return 0;
}