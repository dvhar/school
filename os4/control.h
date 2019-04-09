#ifndef CTRL_H
#define CTRL_H
#define MAXCONCURRENT 19
#define USED_FULL 1
#define TERMINATED 2
#define BLOCKED 3


typedef struct ctrlBlock
{
    unsigned int startS;
    unsigned int startNS;
    unsigned int cpuS;
    unsigned int cpuNS;
    unsigned int sleepS;
    unsigned int sleepNS;
    unsigned int burstNS;
    unsigned int totalS;
    unsigned int totalNS;
    unsigned int pid;
    unsigned int priority;
    unsigned int endSliceS;
    unsigned int endSliceNS;
    unsigned int blockS;
    unsigned int blockNS;
} ctrlBlock;


void clearBlock(ctrlBlock*);
int nextOpenBlock(int);
void markBlock(ctrlBlock *, int*, int , int );
int setupProcess(ctrlBlock *, int*, int, unsigned int, unsigned int);
void addTime(unsigned int, unsigned int, unsigned int*, unsigned int*);
void subtractTime(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int*, unsigned int*);
void divideTime(unsigned int*, unsigned int*, unsigned int);


#endif
