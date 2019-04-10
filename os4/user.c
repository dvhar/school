#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include "control.h"
#define key 7676767

//message queue
struct mq_msgbuf {
   long mtype;
   char mtext[200];
};
struct mq_msgbuf msqMsg;
int msqid;

//global vars
static unsigned int *seconds, *nseconds, *shmptrClock;
static int shmidCtrl, shmidClock, fakePid;
static ctrlBlock * controlTable;
void sigterm_handler(int);
void alarm_handler(int);

int main(int argc, char**argv){

    //use timer to avoid fork bombs in case oss screws up
    alarm(10);
    signal(SIGTERM, &sigterm_handler);
    signal(SIGALRM, &alarm_handler);

    unsigned timeslice,
        chance,
        endS,
        endNS,
        startS,
        startNS,
        terminate=0,
        blocked=0,
        terminateProb,
        blockProb;
    sscanf(argv[1], "%d %d %d %d %d", &fakePid, &shmidClock, &shmidCtrl, &terminateProb, &blockProb);

    //access message queue
    if ((msqid = msgget(key, 0644)) == -1) { /* connect to the queue */
         perror("msgget - user");
         exit(1);
    }

    //access shared clock
    shmptrClock = (int *) shmat(shmidClock, NULL, 0);
    if (shmptrClock == (void *) -1) { perror("shmat clock error - user"); exit(1); }
    seconds = shmptrClock;
    nseconds = shmptrClock+1;

    //access shared control table
    controlTable = (ctrlBlock *) shmat(shmidCtrl, NULL, 0);
    if (controlTable == (void *) -1) { perror("shmat control block error - user"); exit(1); }

    while(1){
        terminate=0; blocked=0;

        //recieve message and record starting time
        if (msgrcv(msqid, &msqMsg, sizeof(msqMsg.mtext), fakePid, 0) == -1) {
             perror("msgrcv - user"); exit(1);
        }
        endS = startS = *seconds;
        endNS = startNS = *nseconds;
        sscanf(msqMsg.mtext, "%d", &timeslice);
        msqMsg.mtype = 99;
        msqMsg.mtext[0] = fakePid;
        msqMsg.mtext[1] = USED_FULL;

        //maybe terminate
        srand(rand()+*nseconds);
        chance = rand() % 1000;
        if (chance == chance % terminateProb){
            //determine where in timeslice to terminate
            srand(rand()+*nseconds);
            timeslice = (timeslice * (rand() % 1000))/1000;
            terminate = 1;
            msqMsg.mtext[1] = TERMINATED;
        }

        //maybe get blocked if not terminating
        if (terminate==0){
            srand(rand()+*nseconds);
            chance = rand() % 1000;
            if (chance == chance % blockProb){
                srand(rand()+*nseconds);
                timeslice = (timeslice * (rand() % 100))/100;
                blocked = 1;
                msqMsg.mtext[1] = BLOCKED;
            }
        }

        //calculte end of timeslice
        addTime(0, timeslice, &endS, &endNS);

        //wait for end of timeslice and record time used
        while (endS > *seconds || (endS >= *seconds && endNS >= *nseconds));

        endS = *seconds;
        endNS = *nseconds;

        //&startS param is a placeholder, only need nanoseconds
        subtractTime(startS, startNS, endS, endNS, &startS, &controlTable[fakePid].burstNS);
        addTime(
            0, controlTable[fakePid].burstNS,
            &controlTable[fakePid].cpuS,
            &controlTable[fakePid].cpuNS);
        controlTable[fakePid].endSliceS = endS;
        controlTable[fakePid].endSliceNS = endNS;


        //tell oss how it ended
        if (msgsnd(msqid, &msqMsg, 2, 0) == -1)
             perror("msgsnd - user");

        if (terminate){
            sigterm_handler(1);
        }

    }

    return (0);
}

//fork bomb prevention
void alarm_handler(int sig){
    fprintf(stderr, "parent process failed to kill forks. fork terminating.\n");
    kill(0, SIGTERM);
}
//detach from memory and exit on sigterm
void sigterm_handler(int sig){
    shmdt(shmptrClock);
    shmdt(controlTable);
    exit(0);
}
