/************ MTX4.5 kernel t.c file **********/
#include "type.h"

int  procSize = sizeof(PROC), nproc, rflag;              // global reschedule flag;
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

extern int color;

#include "gio.c"
#include "queue.c"
#include "wait.c"
#include "kernel.c"

int init()
{
   PROC *p;
   int i;

   /* initialize all proc's */
   printf("init ... ");

   for (i=0; i<NPROC; i++){
       p = &proc[i];
       p->pid = i;                        // pid = 0,1,2,..NPROC-1
       p->status = FREE;
       p->priority = 0;
       p->next = &proc[i+1];              // point to next proc
   }
   proc[NPROC-1].next = 0;         // all procs are in freeLiest
   freeList = &proc[0];
   readyQueue = 0;
   /**** create P0 as running ****/
   p = get_proc(&freeList);               // allocate a PROC from freeList
   p->ppid = 0;                           // P0's parent is itself
   p->status = READY;
   running = p;                           // P0 is running
   nproc = 1;
   printf("complete\n");
 }
   
int scheduler()
{
    if (running->status == READY)         // if running is still READY
    {
        enqueue(&readyQueue, running);    // enter it into readyQueue
    }
   running = dequeue(&readyQueue);        // new running process
   rflag = 0;
   color = running->pid + 7;
}

main()
{
    printf("\nMTX starts in main()\n");
    init();                                 // initialize and create P0 as running
    printf ("PROC %d running\n", running->pid);
    do_kfork();                                // P0 creates P1
    while(1){                               // P0 switches if readyQueue not empty
        if (readyQueue)
            printf ("PROC %d switch process\n", running->pid);
            do_tswitch();
    }
}
