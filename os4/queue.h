#ifndef QUEUE_H
#define QUEUE_H

//pid is the simulated pid of a process
typedef struct process_t
{
  int pid;
  struct process_t * next;
  struct process_t * prev;
} process_t;

void enqueue(int, process_t**back);
void dequeue(process_t**front);
int qlength(process_t*front);
void qshow(process_t*front);

#endif

