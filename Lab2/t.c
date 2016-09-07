/************ MTX4.2 kernel t.c file **********/
#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define RED

/******* PROC status ******/
#define FREE    0
#define READY   1
#define STOP    2
#define DEAD    3
typedef struct proc{
    struct proc *next;
    int    *ksp;               // saved ksp when not running
    int    pid;                // add pid for identify the proc
    int    ppid;               // parent pid;
    int    status;              // status = FREE|READY|STOPPED|DEAD, etc
    int    priority;            // scheduling priority
    int    kstack[SSIZE];      // proc stack area
}PROC;

int  procSize = sizeof(PROC);
PROC proc[NPROC], *running, *freeList, *readyQueue;

#include "gio.c"
#include "queue.c"

extern int color;

int body()
{ 
   char c;
   color = running->pid + 7;
   printf("PROC %d resumes from body()\n", running->pid);
   while(1){
     printf("------------------------------------------------------\n");
     printList("freelist   => ", freeList); // optional: show the freelist
     printList("readyQueue => ", readyQueue); // show the readyQueue
     printf("------------------------------------------------------\n");
     printf("PROC %d running: parent = %d  ", running->pid, running->ppid);
     printf("enter a char [s|f] : ");
     c = getc(); printf("%c\n", c);
     switch(c)
     {
         case 'f' : do_kfork(); break;
         case 's' : do_tswitch(); break;
     }
   }
}

PROC *kfork() // create a child process, begin with body()
{
    int i;
    PROC *p = get_proc(&freeList);
    if (!p)
    {
        printf("no more PROC, kfork() failed\n");
        return 0;
    }
    p->status = READY;
    p->priority = 1;            // priority = 1 for all proc except P0
    p->ppid = running->pid;     // parent = running
    /* initialize new proc's kstack [] */
    for (i=1; i<10; i++)        // saved CPU registers
    {
        p->kstack[SSIZE - i] = 0;   // all initialized to 0
    }
    p->kstack[SSIZE-1] = (int)body; // resume point = address of body()
    p->ksp = &(p->kstack[SSIZE-9]); // proc saved sp = ksp -> kstack top
    enqueue(&readyQueue, p);        // enter p into readyQueue by priority
    return p;                       // return child PROC pointer
}

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

   printf("complete\n");
 }
   
int scheduler()
{
    if (running->status == READY)         // if running is still READY
    {
        enqueue(&readyQueue, running);    // enter it into readyQueue
    }
   running = dequeue(&readyQueue);        // new running process
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

/*************** kernel command functions ***********/
int do_kfork()
{
    PROC *p = kfork();
    if (p == 0) { printf("kfork failed\n"); return -1; }
    printf ("PROC %d kfork a child. child PROC pid = %d\n", running->pid, p->pid);
    return p->pid;
}
int do_tswitch() { tswitch(); }
