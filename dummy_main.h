#include <stdio.h>
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
/* You can add any code here you want to support your SimpleScheduler implementation*/
const char* l = "/my_shared_memory";
    const int SIZE = sizeof(int);

    // Create and open the shared memory object
    int shm_fd1 = shm_open(l, O_CREAT | O_RDWR, 0666);
    if (shm_fd1== -1) {
        perror("shm_open");
        return 1;
    }

    // Set the size of the shared memory segment
    // ftruncate(shm_fd1, SIZE);

    // Map the shared memory segment to the address space of the process
    int* shared_memory1 = (int*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    if (shared_memory1 == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Write an integer to the shared memory
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
    wait(&status); // Wait for any child process to finish

    // if (WIFEXITED(status)) {
    //     printf("Child process exited with status %d\n", WEXITSTATUS(status));
    //     // Handle process termination as needed
    // } else {
    //     printf("Child process did not exit normally\n");
    //     // Handle abnormal exit if needed
    // }
    // Unmap the shared memory segment

    *shared_memory1 = WEXITSTATUS(status);
    // munmap(shared_memory1, SIZE);

    // Close the shared memory object
    // close(shm_fd1);
    
    // exit(0);
    return 0;
}
int ret = dummy_main(argc, argv);
return ret;
}
#define main dummy_main