#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <fenv.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include "carSemaphore.c"

void Finish(int fnum), deleteCar(), washCar(), controlC(), dequeue(int);
double  what_time_is_it();
double startTime;
float nextwashTime(float);
float timeInterval , washingAvgTime , runningTime ;
int Validation(int n, float interval, float avg, float runningTime);
int MemoryValidation(int a,int b,int c,int d,int e, int f,int g, int h, int i);
int ShmatValidation(int* a,int* b,int* c,int* d);
int countingSemaphoreMemory, countingQMemory, mutexMemory, mutexWashMemory, endControlCMemory;
int shmid, carCountShmid, carIndex, programRunning, numberOfWashing;
int* countingQ, endControlC, carQ, carCounter, nowIndex, p_programRunning, mutex, mutexWash, countingSemaphore;

int main(int argc,char *argv[]) {
    startTime = what_time_is_it();
        if ((argc!=5)) {
		    printf("Error in arguments \n");
		    exit(1);
	    }

    numberOfWashing = atoi(argv[1]);
    timeInterval = atof(argv[2]);
    washingAvgTime = atof(argv[3]);
    runningTime = atof(argv[4]);

    printf("The number of washing machine stations: \t %d\n",numberOfWashing);
    printf("The time between each car: \t %.3f\n",timeInterval);
    printf("The average time for  each car wash: \t %.3f\n",washingAvgTime);
    printf("The program simulation time: \t %.3f\n\n",runningTime);
   
    if(!Validation(numberOfWashing,timeInterval,washingAvgTime,runningTime)) {
		exit(1);
    }
    pid_t pid;

    // unique keys
    key_t carCount_k = ftok(".",'a');
    key_t carQueue_k = ftok(".", 'b');
    key_t carIndex_k = ftok(".", 'c');
    key_t isProgramRunnin_k = ftok(".", 'd');
    key_t mutex_k = ftok(".", 'e');
    key_t countingSem_k = ftok(".", 'f');
    key_t endControlCMemory_k = ftok(".", 'g');
    key_t counting_k = ftok(".", 'h');
    key_t countingSemaphore_k = ftok(".", 'i');
    key_t mutexWash_k = ftok(".", 'j');
    key_t mutexNew_k = ftok(".", 'k');

    // Allocation of shared memory
    shmid = shmget(carQueue_k, sizeof(int) * (runningTime / timeInterval) * 2, IPC_CREAT|IPC_EXCL|0666);
    carCountShmid = shmget(carCount_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    carIndex = shmget(carIndex_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    programRunning = shmget(isProgramRunnin_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    endControlCMemory= shmget(endControlCMemory_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    mutexMemory= shmget(mutex_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    countingQMemory= shmget(counting_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    countingSemaphoreMemory= shmget(countingSemaphore_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    mutexWashMemory= shmget(mutexWash_k, sizeof(int), IPC_CREAT|IPC_EXCL|0666);

    if(!MemoryValidation(shmid,carCountShmid,carIndex,programRunning,endControlCMemory,countingQMemory,countingSemaphoreMemory,mutexMemory,mutexWashMemory))
        Finish(1);
    
    mutexWash = (int*)shmat(mutexWashMemory, 0, 0);
    mutex = (int*)shmat(mutexMemory, 0, 0);
    countingSemaphore = (int*)shmat(countingSemaphoreMemory, 0, 0);
    countingQ = (int*)shmat(countingQMemory, 0, 0);
    endControlC = (int*)shmat(endControlCMemory, 0, 0);
    carQ = (int*)shmat(shmid, 0, 0);
    carCounter = (int*)shmat(carCountShmid, 0, 0);
    nowIndex = (int*)shmat(carIndex, 0, 0);
    p_programRunning = (int*)shmat(programRunning, 0, 0);

    if(!ShmatValidation(carQ,carCounter,nowIndex,p_programRunning))
        Finish(2);
  
    *p_programRunning = 1;
    *endControlC=1;
    *countingQ=0;

    // Setting up semaphores
    *mutex = initsem(mutexNew_k, 1); 
	if(*mutex < 0) 
		Finish(3);

    *mutexWash = initsem(mutexWash_k, 1); 
	if(*mutexWash < 0) 
		Finish(8);

    *countingSemaphore = initsem(countingSem_k, numberOfWashing);
	if(*countingSemaphore < 0) 
		Finish(4);

    srand(time(0));

    // Signals
    signal(SIGUSR1, washCar);
    signal(SIGUSR2, deleteCar);
    signal(SIGINT, controlC);
 
    while( what_time_is_it() - startTime< runningTime && *p_programRunning == 1) 
    {
        float washTime = nextwashTime(1 / (float)timeInterval);
        
    
            if (washTime + what_time_is_it() - startTime <runningTime)
        {
        usleep(washTime*1000000);
        pid = fork();
        if(pid == 0) break;

        if (pid < 0) {
            printf("Error creating Child\n");
            exit(1);
        }
        p(*mutex);

        // critical section 
        if(*p_programRunning == 1) { 
            carQ[*nowIndex] = pid; // enter car to the queue
            (*nowIndex)++; 
              
            printf("The running time of the program is: %.6lf seconds\n", what_time_is_it() - startTime);
            printf("The car arrives at the facility and joins the queue\n");
            printf("The PID of the car is: %d\n\n",pid);

            kill(pid, SIGUSR1); 
        }
        v(*mutex);
    }
    }

    while(!(-1==wait(NULL)));

    if(pid > 0 && *endControlC!=0) {
        Finish(5);
        exit(1);
    }
    return 1;
}

float nextwashTime(float rateParameter) {
    float var=-logf(1.0 - (float)rand() / (RAND_MAX + 1.0))  / rateParameter;
    return var;
}

// Car entry washing
void washCar() {
    signal(SIGUSR1, washCar);
    p(*countingSemaphore);
    int pid = getpid();
    (*countingQ)++;
    
    printf("The running time of the program is: %.6lf seconds\n", what_time_is_it() - startTime);
    printf("The car enters a washing machine station\n");
    printf("The PID of the car is: %d\n\n",pid);

    (*carCounter)++; 
    float washingTime = nextwashTime(1 / (float)washingAvgTime);
    usleep(washingTime*1000000);
    kill(pid, SIGUSR2);
    v(*countingSemaphore);
}

// Car exit washing
void deleteCar() {
    signal(SIGUSR2, deleteCar);
    p(*mutexWash);
    (*countingQ)--;
    int pid = getpid();
    dequeue((int)getpid());
    
    printf("The running time of the program is: %.6lf seconds\n", what_time_is_it() - startTime);
    printf("The car has finished washing and is leaving the facility\n");
    printf("The PID of the car is: %d\n\n",pid);
    
    if(*nowIndex == 0 && *p_programRunning != 1 && *countingQ== 0) {
        Finish(6);
    }

    if( *endControlC==0 && *countingQ== 0  ) {
        Finish(7);
    }
    v(*mutexWash);
}
 
 // Taking a car out of queue
void dequeue(int car) {
    int* temp; 
    for(int i = 0; i < *nowIndex; i ++) {
        if(carQ[i] == car) {
            for(int j = i; j < *nowIndex - 1; j++) {
                carQ[j] = carQ[j + 1];
            }
            (*nowIndex)--;
            break;
        }
    }
}

// End of program
void Finish(int fnum) {
    if (*p_programRunning !=0) 
        *p_programRunning=0;
    shmctl(mutexWashMemory, IPC_RMID, 0);
    shmctl(mutexMemory, IPC_RMID, 0);
    shmctl(countingSemaphoreMemory, IPC_RMID, 0);
    shmctl(countingQMemory, IPC_RMID, 0);
    shmctl(endControlCMemory, IPC_RMID, 0);
    shmctl(shmid, IPC_RMID, 0);
    shmctl(carCountShmid, IPC_RMID, 0);
    shmctl(carIndex, IPC_RMID, 0);
    shmctl(programRunning, IPC_RMID, 0);
    
    for (int i = 0; i <*nowIndex; ++i)
		kill(carQ[i], SIGKILL);
    
    printf("The amount of cars that were in the washing machine is : %d \n", *carCounter);
    printf("The total time of program run is: %.6lf seconds\n", what_time_is_it() - startTime);
    exit(0);
}

int Validation(int n, float interval, float avg, float runningTime)
{
    if(n > 5 || n <= 0)
    {
        printf("error number of washing\n");
        return 0;
    }
    if(interval < 0){
        printf("error time interval between cars\n");
        return 0;
    }
    if(avg < 0){
        printf("error avarge time washing cars\n");
        return 0;
    }
    if(runningTime < 0){

        printf("error imaging time\n");
        return 0;
    }
    return 1;
}

int MemoryValidation(int a,int b,int c,int d,int e, int f,int g, int h, int i)
{
    if(a == -1 || b == -1 || c == -1 || d == -1 || e==-1 | f==-1 || g==-1 || h==-1  || i==-1 ){
        printf("Memory Problem :(\n");
        return 0;      
    }
    return 1;
}

int ShmatValidation(int* a,int* b,int* c,int* d){
    if(a == (int*)-1|| b == (int*)-1 || c == (int*)-1 || d == (int*)-1) {
        printf("Shmat Problem :(\n");
        return 0;      
    }
    return 1;
}

double what_time_is_it()
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec + now.tv_nsec*1e-9;
}

void controlC()
{
   *endControlC=0; 
   *p_programRunning=0;
}