#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include "queue.h"
#include "control.h"
#define key 7676767
#define MAXFORKS 100
#define NUMQUEUES 8


//max interval and other global vars
static unsigned int maxTimeBetweenProcsSecs=0, 
    maxTimeBetweenProcsNS=1E6,
    *seconds,
    *nseconds,
    *shmptrClock,
    numLaunched=0,
    numFinished=0,
    idleS,
    idleNS,
    avgTurnaroundS,
    avgTurnaroundNS,
    avgSleepS,
    avgSleepNS,
    avgCpuS,
    avgCpuNS,
    avgWaitS,
    avgWaitNS;
static int shmidCtrl, shmidClock;
static ctrlBlock * controlTable;
char *logfile = "log.out";
char line[200];
FILE *ofp;

//message queue stuff
struct mq_msgbuf {
   long mtype;
   char mtext[200];
};
struct mq_msgbuf msqMsg;
int msqid;

//return 0 if realtime or 1 if user process. Probablity is 10%
unsigned int userOrRealtime(){
    srand(rand()+*nseconds);
    int chance = rand() % 100;
    if (chance == chance % 10)
        return 0;
    return 1;
}

//get the next launch time by adding a random amount to last launch time
void getInterval(unsigned int*seconds, unsigned int*nseconds){
    srand(rand()+*nseconds);
    *seconds += rand() % (maxTimeBetweenProcsSecs+1);
    srand(rand()+*seconds);
    addTime(0, rand() % (maxTimeBetweenProcsNS+1), seconds, nseconds);
}

//see if a process's event has happened
int checkEvent(unsigned int fakePid){
    unsigned int eventS = controlTable[fakePid].blockS;
    unsigned int eventNS = controlTable[fakePid].blockNS;
    if (eventS < *seconds || (eventS <= *seconds && eventNS < *nseconds))
        return 1;
    return 0;
}

void alarm_handler(int);
void sigterm_handler(int);
void sigint_handler(int);

int main(int argc, char**argv){

    srand(time(0));
    unsigned int numCurrent=0,
        thisQuantum,
        linesWritten=0,
        procBitArray = 0,
        message,
        currentQueue,
        tempS,
        tempNS,
        nextS=0,
        nextNS=0,
        priority,
        fakePid,
        pid, 
        timeout=3,
        currentProc=0,
        help=0,
        level=0,
        turn = 0,
        terminateProb=2,
        blockProb=1,
        quantum = 200000,
        increment = 20000;

    //handle options
    int c;
    while ((c = getopt (argc, argv, "ht:q:i:s:n:x:b:")) != -1)
      switch (c){
        case 'h':
          help = 1;
          break;
        case 't':
          timeout = atoi(optarg);
          break;
        case 'q':
          quantum = atoi(optarg);
          break;
        case 'i':
          increment = atoi(optarg);
          break;
        case 's':
          maxTimeBetweenProcsSecs = atoi(optarg);
          break;
        case 'n':
          maxTimeBetweenProcsNS = atoi(optarg);
          break;
        case 'x':
          terminateProb = atoi(optarg);
          break;
        case 'b':
          blockProb = atoi(optarg);
          break;
        default:
          exit(1);
        }

    if (help){
        puts("Command line options:\n\n"
             "  -h : View this help message.\n"
             "  -i <nanoseconds> : Change time incrememt. Default is 20000.\n"
             "  -q <nanoseconds> : Change timeslice quantum. Default is 200000.\n"
             "  -t  <seconds> : Set the number of seconds until timeout. Default is 3.\n"
             "  -x  <number> : Set the termination probability (out of 1000) per quantum. Default is 2.\n"
             "  -b  <number> : Set the block probability (out of 1000) per quantum. Default is 1.\n"
             "  -s  <seconds> : Set the max seconds between launch. Default is 0.\n"
             "  -n  <seconds> : Set the max nanoseconds between launch. Default is a million.\n");
        exit(0);
    }

    if ((ofp = fopen(logfile, "w")) == NULL){ perror("Bad output file"); exit(1); }
    unsigned int timeSlices[4] = { quantum, quantum*2 , quantum*4 , quantum*8};
    alarm(timeout);
    signal(SIGALRM, &alarm_handler);
    signal(SIGINT, &sigint_handler);
    signal(SIGTERM, &sigterm_handler);

    //set up message queue
    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget"); exit(1);
    }

    //first 8 queues for for the 4 priorities, each having 2 that alternatate active/inactive. Last is blocked queue
    struct process_t *backofQueues[NUMQUEUES];
    struct process_t *frontofQueues[NUMQUEUES];
    for (int ii=0; ii<NUMQUEUES; ++ii){
        backofQueues[ii] = malloc(sizeof(process_t));
        frontofQueues[ii] = backofQueues[ii];
    }

    //make shared control table and clock
    shmidCtrl = shmget(getpid(), MAXCONCURRENT*sizeof(ctrlBlock), IPC_CREAT | 0600);
    if (shmidCtrl < 0) { perror("ctrlBlock shmget error"); exit(1); }
    shmidClock = shmget(getpid()+1, 2*sizeof(int), IPC_CREAT | 0600);
    if (shmidClock  < 0) { perror("clock shmget error"); exit(1); }

    //access shared clock and initialize to 0
    shmptrClock = (int *) shmat(shmidClock, NULL, 0);
    if (shmptrClock == (void *) -1) { perror("shmat clock error"); exit(1); }
    seconds = shmptrClock;
    nseconds = shmptrClock+1;
    *seconds = *nseconds = 0;

    //access shared control table and initialize it to 0
    controlTable = (ctrlBlock *) shmat(shmidCtrl, NULL, 0);
    if (controlTable == (void *) -1) { perror("shmat control block error - oss"); exit(1); }
    for (int ii=0; ii<MAXCONCURRENT; ++ii)
        clearBlock(controlTable+ii);

    getInterval(&nextS, &nextNS);
    //start looping until 100 processes have completed
    while(numLaunched < MAXFORKS || numCurrent > 0){

        //increment clock and count idle cpu time
        addTime(0, increment, seconds, nseconds);
        if (currentProc == 0)
            addTime(0, increment, &idleS, &idleNS);

        //wait until child process actually ends before launching new one, don't just use message
        if (waitpid(0, NULL, WNOHANG) > 0)
            --numCurrent;

        //launch process if it is time. get time for next launch.
        if (nextS < *seconds || (nextS <= *seconds && nextNS < *nseconds)){
            getInterval(&nextS, &nextNS);

            //see if there is room for another process
            if (numCurrent < MAXCONCURRENT && numLaunched < MAXFORKS) {

                //setup process and get simulated pid and priority
                priority = userOrRealtime();
                fakePid = setupProcess(controlTable, &procBitArray, priority, *seconds, *nseconds);
                controlTable[fakePid].startS = *seconds;
                controlTable[fakePid].startNS = *nseconds;

                //fork process
                snprintf(line, 200, "%u %d %d %u %u",fakePid,  shmidClock, shmidCtrl, terminateProb, blockProb);
                pid = fork();
                if (!pid) {
                    execl("./user","user",line);
                    perror("Exec didn't work");
                    sigint_handler(0);
                } else {
                    if (pid != -1){
                        //put process in ready queue where it waits for message from messsage queue
                        enqueue(fakePid, &backofQueues[priority*2+(1^turn)]);
                        ++numCurrent;
                        ++numLaunched;
                        //printf("Created process %u, total %u. put in queue %u at %u:%u.\n", fakePid, numLaunched, priority, *seconds, *nseconds);
                        snprintf(line, 200, "Created process %u, total %u. put in queue %u at %u:%u.\n", fakePid, numLaunched, priority, *seconds, *nseconds);
                        printf(line);
                        //write to log
                        if (++linesWritten < 10000)
                            fprintf(ofp, line);
                    } else {
                        //undo bit vector mark because fork messed up
                        markBlock(controlTable, &procBitArray,  fakePid, 0);
                        perror("fork error");
                    }
                }
            }
        }

        //wake up next process in line
        if (currentProc==0 && numCurrent>0){
            currentQueue = level*2+turn;

            //send message to process in front of hightest priority occupied queue
            if (qlength(frontofQueues[currentQueue]) > 1){
                currentProc = frontofQueues[currentQueue]->next->pid;
                thisQuantum = timeSlices[level];
                sprintf(msqMsg.mtext, "%d", thisQuantum);
                msqMsg.mtype = currentProc;
                if (msgsnd(msqid, &msqMsg, strlen(msqMsg.mtext)+1, 0) == -1) perror("msgsnd - oss");
                dequeue(&frontofQueues[currentQueue]);

                //write to log
                snprintf(line, 200, "Dispatched process %d with priority %u and timeslice %u at %u:%u\n",
                    currentProc, level, thisQuantum, *seconds, *nseconds);
                if (++linesWritten < 10000)
                    fprintf(ofp, line);

                //imcrement clock to simulate time used to dispatch
                addTime(0, increment, seconds, nseconds);

            //find the next queue to traverse if current queue is empty
            } else {
                if (level < 3){
                    ++level;
                } else {
                    level = 0;
                    turn ^= 1;
                }
            }
        }

        //check blocked processes to see if it's time to unblock them
        for (int ii=0; ii<20; ++ii)
            if ((controlTable[ii].blockNS > 0 || controlTable[ii].blockS > 0) && checkEvent(ii)){
                //calculate this round of sleep time and add it to running total
                subtractTime(
                    controlTable[ii].endSliceS,
                    controlTable[ii].endSliceNS,
                    *seconds, *nseconds, &tempS, &tempNS);
                addTime(tempS, tempNS, &controlTable[ii].sleepS, &controlTable[ii].sleepNS);
                //put into ready queue
                priority = controlTable[ii].priority;
                enqueue(ii, &backofQueues[priority*2+(turn^1)]);
                //write to log
                snprintf(line, 200, "Unblocked process %d and put into queue %u at %u:%u\n", ii, priority, *seconds, *nseconds);
                printf(line);
                if (++linesWritten < 10000)
                    fprintf(ofp, line);
                //increment time to simulate time spend unblocking process
                addTime(0, increment, seconds, nseconds);
                if (currentProc == 0)
                    addTime(0, increment, &idleS, &idleNS);
                controlTable[ii].blockS = 0;
                controlTable[ii].blockNS = 0;
            }

        //check messages
        if (msgrcv(msqid, &msqMsg, sizeof(msqMsg.mtext), 99, IPC_NOWAIT) != -1) {
            //puts("trying to process message");
            fakePid  = msqMsg.mtext[0];
            message  = msqMsg.mtext[1];
            priority = controlTable[fakePid].priority;

            switch (message){
                //if process used its time slice, send it to back of the appropriate queue
                case USED_FULL:
                    snprintf(line, 200, "Process %u used its full timeslice. Moving from queue %u to queue %u. time: %u:%u\n",
                        fakePid, priority, (priority<3&&priority>0 ? priority+1 : priority), *seconds, *nseconds);
                    if (priority < 3 && priority > 0)
                        controlTable[fakePid].priority = ++priority;
                    enqueue(fakePid, &backofQueues[priority*2+(turn^1)]);
                    //write to log
                    if (++linesWritten < 10000)
                        fprintf(ofp, line);
                    break;

                //if process is blocked, put it in blocked queue and calculate time to unblock
                case BLOCKED:
                    if (priority > 0)
                        controlTable[fakePid].priority = 1;
                    srand(rand()+*nseconds);
                    controlTable[fakePid].blockS = controlTable[fakePid].endSliceS + (rand() % 6);
                    srand(rand()+*nseconds);
                    controlTable[fakePid].blockNS = controlTable[fakePid].endSliceNS + ((rand() % 1000) * 1E6);
                    snprintf(line, 200, "Blocking Process %u until %u:%u. used %uns of timeslice. time: %u:%u\n",
                      fakePid, controlTable[fakePid].blockS, controlTable[fakePid].blockNS, controlTable[fakePid].burstNS, *seconds, *nseconds);
                    printf(line);
                    //write to log
                    if (++linesWritten < 10000)
                        fprintf(ofp, line);
                    break;

                //calculate stats and clean up when process terminates
                case TERMINATED:
                    //calculate process turnaround and add it to running total
                    subtractTime(
                        controlTable[fakePid].startS,
                        controlTable[fakePid].startNS,
                        *seconds,
                        *nseconds,
                        &controlTable[fakePid].totalS,
                        &controlTable[fakePid].totalNS);
                    addTime(
                        controlTable[fakePid].totalS,
                        controlTable[fakePid].totalNS,
                        &avgTurnaroundS,
                        &avgTurnaroundNS);
                    //add process cpu time to running total
                    addTime(
                        controlTable[fakePid].cpuS,
                        controlTable[fakePid].cpuNS,
                        &avgCpuS,
                        &avgCpuNS);
                    //add process sleep time to running total
                    addTime(
                        controlTable[fakePid].sleepS,
                        controlTable[fakePid].sleepNS,
                        &avgSleepS,
                        &avgSleepNS);
                    //calculate process wait time and add it to running total
                    tempS = tempNS = 0;
                    addTime(
                        controlTable[fakePid].sleepS,
                        controlTable[fakePid].sleepNS,
                        &tempS,
                        &tempNS);
                    addTime(
                        controlTable[fakePid].cpuS,
                        controlTable[fakePid].cpuNS,
                        &tempS,
                        &tempNS);
                    subtractTime(
                        tempS,
                        tempNS,
                        controlTable[fakePid].totalS,
                        controlTable[fakePid].totalNS,
                        &tempS,
                        &tempNS);
                    addTime(tempS, tempNS, &avgWaitS, &avgWaitNS);

                    //print stats and clean up dead process
                    snprintf(line, 200, "Process %u terminated. used %u of timeslice. cpu time %u:%u  total time: %u:%u  sleep time: %u:%u wait time: %u:%u\n", 
                        fakePid, controlTable[fakePid].burstNS,
                        controlTable[fakePid].cpuS, controlTable[fakePid].cpuNS,
                        controlTable[fakePid].totalS, controlTable[fakePid].totalNS,
                        controlTable[fakePid].sleepS, controlTable[fakePid].sleepNS,
                        tempS, tempNS);
                    printf(line);
                    //write to log
                    if (++linesWritten < 10000)
                        fprintf(ofp, line);
                    ++numFinished;
                    markBlock(controlTable, &procBitArray,  fakePid, 0);
                    break;
            }
            currentProc=0;
        }

    }

    //clean up before exiting
    kill(0, SIGTERM);
    return (0);
}


//clean up processes and shared memory on sigterm
void sigterm_handler(int sig){
    //print stats before exiting
    divideTime(&avgTurnaroundS, &avgTurnaroundNS, numFinished);
    divideTime(&avgSleepS, &avgSleepNS, numFinished);
    divideTime(&avgCpuS, &avgCpuNS, numFinished);
    divideTime(&avgWaitS, &avgWaitNS, numFinished);
    snprintf(line, 200, "Total idle time: %u:%u. Avg Turnaround: %u:%u. Avg Sleep: %u:%u.  Avg cpu time: %u:%u Avg wait: %u:%u\n",
        idleS, idleNS, avgTurnaroundS, avgTurnaroundNS, avgSleepS, avgSleepNS, avgCpuS, avgCpuNS, avgWaitS, avgWaitNS);
    printf(line);
    fprintf(ofp, line);
    fclose(ofp);

    while( wait(0) > 0 );
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl"); exit(1);
    }
    shmdt(shmptrClock);
    shmdt(controlTable);
    shmctl(shmidClock, IPC_RMID, NULL);
    shmctl(shmidCtrl, IPC_RMID, NULL);
    exit(0);
}
//sigint and alarm trigger signal to kill children and clean up shared resources
void sigint_handler(int sig){ 
    fprintf(stderr, "Recieved Sigint\n");
    kill(0, SIGTERM); }
void alarm_handler(int sig){
    fprintf(stderr, "Timed out\n");
    kill(0, SIGTERM);
}
