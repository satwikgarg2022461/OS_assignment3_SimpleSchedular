#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

// ------ defining all variables used
#define MAX_SIZE 1024
#define MAX_SIZE_QUEUE 100


// --------sturct for history cmd
typedef struct History {
    char* name;
    pid_t pid;
    struct timeval start_time, end_time;
    int execution_time;
} History;

typedef struct Process
{
    pid_t pid;
    int execution_time;
    char name[MAX_SIZE];
}Process;

History* history[MAX_SIZE];
int count_history = 0;

struct timeval start_time, end_time;



typedef struct {
    Process arr[MAX_SIZE_QUEUE];
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
    q->arr[q->rear].pid = value;
}

int dequeue(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty. Cannot dequeue.\n");
        return -1; // Return a sentinel value indicating error
    }

    int value = q->arr[q->front].pid;

    if (q->front == q->rear) {
        q->front = q->rear = -1; // Reset queue when last element is dequeued
    } else {
        q->front++;
    }

    return value;
}

int getSize(Queue* q) {
    if (isEmpty(q)) {
        return 0;
    }
    return (q->rear - q->front + 1);
}

// declaring global queue
Queue* q;
// initializeQueue(&q);



// -------recording value in history array
void record_process_history(pid_t pid, char* cmd, struct timeval start_time, struct timeval end_time) {
    history[count_history] = malloc(sizeof(History));
    if (history[count_history] == NULL) {
        perror("Error allocating memory for history");
        exit(1);
    }
    history[count_history]->pid = pid;
    history[count_history]->start_time = start_time;
    history[count_history]->end_time = end_time;
    history[count_history]->name = strdup(cmd);
    history[count_history]->execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000;
    count_history++;
}


// printing history array value by cmd history, on exit, on ctrl+c
void print_history() {
    printf("\nShell terminated, printing info of all child processes...\n");
    printf("Name\tPID\tStart Time\t\tEnd Time\t\tExecution Time\n");
    for (int i = 0; i < count_history; i++) {
        printf("%s\t%d\t%ld\t%ld\t%d\n",
               history[i]->name,
               history[i]->pid,
               history[i]->start_time.tv_sec,
               history[i]->end_time.tv_sec,
               history[i]->execution_time);
    }
}

// print shell line
void print_display() {
    char path[MAX_SIZE];
    if (getcwd(path, sizeof(path)) != NULL) {
        printf("%s  ", path);
    } else {
        perror("Error in printing simple shell line");
        exit(1);
    }
}

// free the value of history array
void free_history()
{
    for(int i=0;i<count_history;i++)
    {
        free(history[i]->name);
        free(history[i]);
    }
}


// signal function
static void my_handler(int signum) {
    if(signum == SIGINT) {
        // print_process_info();
        print_history();
        free_history();
        exit(0);
    }
}

// taking input from the user
char* user_input() {
    char input[MAX_SIZE];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        return strdup(input);
    } else {
        // error checking for input
        perror("error in taking input");
        exit(1);
    }
}


// creating child process and run the give cmd
void launch_process(char* cmd) {

    if (strcmp(cmd, "history") == 0) {
        // handling history cmd
        if (count_history == 0) {
            printf("No history value\n");
            return;
        }
        print_history();
        return;
    }
    
    gettimeofday(&start_time, NULL); // getting start_time

    if (strcmp(cmd, "exit") == 0) {
        // handling exit cmd
        if (count_history == 0) {
            printf("No history value\n");
            exit(0);
        }
        print_history();
        free_history();
        exit(0);
    }

    pid_t pid = fork();//forking

    if (pid < 0) {
        // error checking in fork
        perror("Error in forking process");
        exit(1);
    } 
    else if (pid == 0) {
        // child process
        char* args[MAX_SIZE];
        int num_args = 0;

        char* token = strtok(cmd, " ");
        while (token != NULL) {
            args[num_args] = token;
            num_args++;
            token = strtok(NULL, " ");
        }

        args[num_args] = NULL;

        if (args[0] == NULL) {
            // error ching in args
            printf("Invalid command\n");
            exit(1);
        }

        int check = execvp(args[0], args);
        if (check == -1) {
            // check for execvp
            perror("Error in execvp");
            exit(1);
        }
        exit(0);
    } 
    else {
        // ---parent process
        wait(NULL);
        gettimeofday(&end_time, NULL); // getting ending time
        record_process_history(pid, cmd, start_time, end_time); // recording the cmd
    }
}

// launching process
int launch(char* cmd) {
    if (cmd == NULL) {
        // checking for NULL in cmd
        return 0;
    }
    launch_process(cmd);
    return 1;
}

// loop for shell
void shellloop() {
    char* cmd;
    int status;

    do {
        print_display();
        cmd = user_input();
        status = launch(cmd);

        free(cmd);// free cmd command

    } while (status);

    free_history();// free history array
}

void SimpleShechular()
{
    
    while(1)
    {

        while(isEmpty(q))
        {


        }
    }
}

int main(int argc, char* argv[]) {
    if(argc != 3)
    {
        perror("Wrong input");
        // exit(1);
        return 1;
    }


    // setup the signal ctrl+c
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);



    shellloop();
    return 0;
}
