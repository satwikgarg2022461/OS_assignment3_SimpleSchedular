# OS_assignment3_SimpleSchedular

Github LINK - https://github.com/satwikgarg2022461/OS_assignment3_SimpleSchedular.git

# Contribution  
This code is made by Satwik Garg and Saurav Haldar 

# Implementation  
1. We fork the simple schedular in simple shell as deamon  
2. We create a function named submit which fork the execution process then parent send the queue to simple schedular using shared memory created using shm_open  
3. Simple schedular received queue from Simple shell using shm_open  
4. An infinite while(1) loop is used to keep the simple schedualer daemon   
5. Then we continue the process submitted by the user using kill(pid, SIGCONT)  
6. Simple schedualer then sleep for TSLICE   
7. Then it stop all processes using kill(pid, SIGSTOP)  
8. A dummmy_main.h header is created for sending the termination details from the user input file to simple scheduler  
9. We also created a shared memory in dummy_main.h for sending the data of termination to the simple Schedular  

# How to run the code
1. Add the line --> #include "dummy_main.h" in your code after adding the adding the other headers.   
2. Then type the make in the terminal  
3. Use sumbit for executing the process in simple shell     
4. For exiting use ctrl+c or exit  

# Limitations
1. User should only terminate Simple Shell after all the processes are complete.  
2. If it is terminated before then simple Shell will not work in the next execution.  
3. user is not allowed to submit any program that has blocking calls, e.g., scanf, sleep, etc., and the user program cannot have any command line parameters
