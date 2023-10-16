#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h> 
#include <sys/mman.h>
#include<semaphore.h>
int dummy_main(int argc, char **argv);
int main(int argc, char **argv) {
const char* l = "/my_shared_memory";
    const int SIZE = sizeof(int);

    //..............Shared memory with simplescheduler
    int shm_fd1 = shm_open(l, O_CREAT | O_RDWR, 0666);
    if (shm_fd1== -1) {
        perror("shm_open");
        return 1;
    }

    int* shared_memory1 = (int*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    if (shared_memory1 == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    *shared_memory1 = 42;
    
int check = fork();
if(check <0)
{
    printf("error");
    exit(0);
}
else if (check>0)
{
    int status;
    wait(&status);
    *shared_memory1 = WEXITSTATUS(status);
    munmap(shared_memory1,SIZE);
    unlink(l);
    close(shm_fd1);
    return 0;
} 


int ret = dummy_main(argc, argv);
return ret;

}
#define main dummy_main
