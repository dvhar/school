#include "control.h"
#include <stdio.h>


void clearBlock(ctrlBlock *block) {
    block->startS     = 0;
    block->startNS    = 0;
    block->cpuS       = 0;
    block->cpuNS      = 0;
    block->burstNS    = 0;
    block->totalS     = 0;
    block->totalNS    = 0;
    block->pid        = 0;
    block->priority   = 0;
    block->endSliceS  = 0;
    block->endSliceNS = 0;
    block->sleepS     = 0;
    block->sleepNS    = 0;
    block->blockS     = 0;
    block->blockNS    = 0;
}

//return pid of next available control block, or 21 if none available
int nextOpenBlock(int procBitArray){
    int ii=1;
    for (; ii<=MAXCONCURRENT+1; ++ii)
        if ((procBitArray & (1<<ii)) == 0)
            break;
    return ii;
}

//mark a control bloack available or used in the bit vector and initialize the block
void markBlock(ctrlBlock *ctrl, int *procBitArray, int idx, int action){
    if (action == 1)
        *procBitArray |= (1<<idx);
    if (action == 0)
        *procBitArray &= (~(1<<idx));
    clearBlock(ctrl+idx);
}

//prepare control block and return simulated pid
int setupProcess(ctrlBlock *ctrl, int *procBitArray, int priority, unsigned int startS, unsigned int startNS){
    int pid = nextOpenBlock(*procBitArray);
    if (pid > MAXCONCURRENT) return -1;
    markBlock(ctrl, procBitArray, pid, 1);
    ctrl[pid].priority = priority;
    ctrl[pid].pid = pid;
    ctrl[pid].startS = startS;
    ctrl[pid].startNS = startNS;
    return pid;
}

//time utilities
void addTime(unsigned int addSec, unsigned int addNsec, unsigned int*toSec, unsigned int*toNsec){
    *toSec += addSec;
     if (*toNsec+addNsec < 1E9) {
         *toNsec += addNsec;
     } else {
         (*toSec)++;
         *toNsec += (addNsec-1E9);
     }
}
void subtractTime(unsigned int startS, unsigned int startNS, unsigned int endS, unsigned int endNS, unsigned int*diffS, unsigned int*diffNS){
    unsigned long long start = (((unsigned long long) startS) * (unsigned long long)1E9) + startNS;
    unsigned long long end   = (((unsigned long long) endS) * (unsigned long long)1E9) + endNS;
    unsigned long long result = end - start;
    *diffS = (result / (unsigned long long)1E9);
    *diffNS = (result % (unsigned long long)1E9);
}
void divideTime(unsigned int*s, unsigned int*ns, unsigned int divisor){
    if (divisor == 0) return;
    unsigned long long total = (((unsigned long long) *s) * (unsigned long long)1E9) + *ns;
    *s = ((total / divisor) / (unsigned long long)1E9);
    *ns = ((total / divisor) % (unsigned long long)1E9);
}
