#include<stdlib.h>
#include<stdio.h>
#include"queue.h"

//put in back of queue
void enqueue(int pid, process_t **back)
{
  (*back)->next = malloc(sizeof(process_t));
  (*back)->next->prev = *back;
  (*back)->next->next = NULL;
  (*back)->next->pid = pid;
  *back = (*back)->next;
}

//remove from front of queue
void dequeue(process_t **front)
{
  *front = (*front)->next;
  free((*front)->prev);
}

//get queue length
int qlength(process_t *front)
{
  int count = 0;
  while (front && ++count && (front = front->next));
  return count;
}


//leftover from debugging. shows contents of a queue
void qshow(process_t*front)
{
  printf("front: ");
  while (front){
    printf(" %d ", front->pid);
    front = front->next;
  }
  puts("");
}
