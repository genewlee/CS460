
/*************** kernel command functions ***********/
int body()
{
   char c;
   color = running->pid + 7;
   printf("PROC %d resumes from body()\n", running->pid);
   while(1){
     if (rflag){
         printf("proc %d: reschedule\n", running->pid);
         rflag = 0;
         tswitch();
     }
     printf("PROC %d running: parent = %d\n", running->pid, running->ppid);
     printf("------------------------------------------------------\n");
     printList("freelist   => ", freeList); // optional: show the freelist
     printList("readyQueue => ", readyQueue); // show the readyQueue
     printList("sleepList  => ", sleepList);
     printf("------------------------------------------------------\n");
     printf("enter a char [s|f|w|q] : ");
     c = getc(); printf("%c\n", c);
     switch(c)
     {
         case 'f' : do_kfork(); break;
         case 's' : do_tswitch(); break;
         case 'w' : do_wait(); break;
         case 'q' : do_exit(); break;
         default: printf("invalid command\n"); break;
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
    p->parent = running;
    /* initialize new proc's kstack [] */
    for (i=1; i<12; i++)        // saved CPU registers
    {
        p->kstack[SSIZE - i] = 0;   // all initialized to 0
    }
    p->kstack[SSIZE-1] = (int)body; // resume point = address of body()
    p->kstack[SSIZE-10] = 0x1000;
    p->kstack[SSIZE-11] = 0x1000;
    p->ksp = &(p->kstack[SSIZE-11]); // proc saved sp = ksp -> kstack top
    enqueue(&readyQueue, p);        // enter p into readyQueue by priority
    nproc++;
    return p;                       // return child PROC pointer
}

int do_kfork()
{
    PROC *p = kfork();
    if (p == 0) { printf("kfork failed\n"); return -1; }
    printf ("PROC %d kfork a child. child PROC pid = %d\n", running->pid, p->pid);
    return p->pid;
}

int do_tswitch() { tswitch();}

int do_sleep(int event)
{ // shown above
    ksleep(event);
}

int do_wakeup(int event)
{ // shown above
    kwakeup(event);
}

int do_exit()
{
    char* s; int s_code;
    printf("Enter exit code: ");
    gets(s); putc('\r');
    s_code = atoi(s);
    kexit(s_code);
}

int do_wait()
{
    int exitCode;
    int childPid = kwait(&exitCode);
    if (childPid == -1)
        printf("PROC %d has no children to wait on\n", running->pid);
    //else
    //    printf ("child PROC %d has been laid to rest. child PROC exit code = %d\n", childPid, exitCode);
}
// added scheduling functions in MTX4.5
/*int reschedule()
{
    PROC *p, *tempQ = 0;
    while ( (p=dequeue(&readyQueue)) ){ // reorder readyQueue
        enqueue(&tempQ, p);
    }
    readyQueue = tempQ;
    rflag = 0;                          // global reschedule flag
    if (running->priority < readyQueue->priority)
        rflag = 1;
}

int chpriority(int pid, int pri)
{
    PROC *p; int i, ok=0, reQ=0;
    if (pid == running->pid){
        running->priority = pri;
        if (pri < readyQueue->priority)
            rflag = 1;
        return 1;
    }
    // if not for running, for both READY and SLEEP procs
    for (i=1; i<NPROC; i++){
        p = &proc[i];
        if (p->pid == pid && p->status != FREE){
            p->priority = pri;
            ok = 1;
            if (p->status == READY) // in readyQueue==> redo readyQueue
                reQ = 1;
        }
    }
    if (!ok){
        printf("chpriority failed\n");
        return -1;
    }
    if (reQ)
        reschedule(p);
}

int do_chpriority()
{
    int pid, pri;
    printf("input pid " );
    pid = geti();
    printf("input new priority " );
    pri = geti();
    if (pri<1) pri = 1;
        chpriority(pid, pri);
}*/

