
/*************** kernel command functions ***********/
int body()
{
   char c; int exitValue;
   color = running->pid + 7;
   printf("----------------KERNEL MODE-------------------\n");
   printf("PROC %d resumes from body()\n", running->pid);
   while(1){
     printf("PROC %d running: parent = %d\n", running->pid, running->ppid);
     printf("------------------------------------------------------\n");
     printList("freelist   => ", freeList); // optional: show the freelist
     printList("readyQueue => ", readyQueue); // show the readyQueue
     printList("sleepList  => ", sleepList);
     printf("------------------------------------------------------\n");
     printf("enter a char [s|f|w|q|u] : ");
     c = getc(); printf("%c\n", c);
     switch(c)
     {
         case 'f' : fork();       break;//do_kfork();   break;
         case 's' : do_tswitch(); break;
         case 'w' : do_wait();    break;
         case 'q' : printf("enter an exitValue: ");
                    exitValue = gets();
                    do_exit(atoi(exitValue));    break;
         case 'u' : goUmode();    break;
         default: printf("invalid command\n"); break;
     }
   }
}

/* create the Umode image from file u1
 * and load into its segment (pg 153)
 */
int makeUserImage(char* filename, PROC *p)
{
    int i, segment, high;

    segment = 0x800*(p->pid + 1) + 0x1000;
    high =  32 * 1024;
    printf("segment = %x, high = %x\n", segment, high);

    if (!load(filename, segment))   // load filename to segment
        return -1;                  // if load failed, return -1 to Umode

    /* re-initialize process ustack for it return to VA=0 */
    for (i = 1; i <= 12; i++)
        put_word(0, segment, high-2*i);

    /*  -1   -2 -3  -4 -5 -6 -7 -8 -9 -10 -11 -12 */
    /* flag uCS uPC ax bx cx dx bp si  di uES uDs */
    put_word(segment, segment, high-2*12);  // saved uDS=segment
    put_word(segment, segment, high-2*11);  // saved uES=segment
    put_word(segment, segment, high-2*2);   // uCS=segment; uPC=0
    put_word(0x0200,  segment, high-2*1);   // Umode flag=0x200

    p->usp = high-(2 *12);                   // new usp = -2 * 12
    p->uss = segment;               // set at segment
}

PROC *kfork(char *filename) // create a child process, begin with body()
{
    int i, segment;
    PROC *p = get_proc(&freeList); // get proc for child process
    if (!p)
    {
        printf("no more PROC, kfork() failed\n");
        return 0;
    }

    /* initialize new proc and its stack */
    p->status = READY;
    p->priority = 1;            // priority = 1 for all proc except P0
    p->ppid = running->pid;     // parent = running
    p->parent = running;

    /* initialize new proc's kstack [] */
    for (i=1; i<10; i++)        // saved CPU registers
    {
        p->kstack[SSIZE - i] = 0;   // all initialized to 0
    }

    p->kstack[SSIZE-1] = (int)goUmode;//(int)body; // resume point = address of body()
    p->ksp = &(p->kstack[SSIZE-9]); // proc saved sp = ksp -> kstack top

    enqueue(&readyQueue, p);        // enter p into readyQueue by priority
    nproc++;

    segment = 0x800*(p->pid + 1) + 0x1000;

    if (filename)
    {
        makeUserImage(filename, p);
        printf("PROC %d kforked a child (proc %d, segment %x)\n", running->pid, p->pid, segment);
    }
    else // just to see which one it pritns
        printf("PROC %d kforked a child proc %d, segment %x\n", running->pid, p->pid, segment);
    return p;                       // return child PROC pointer
}

#include "forkexec.c" // Lab 4

int do_kfork()
{
    PROC *p = kfork("/bin/u1");
    if (p == 0) { printf("kfork failed\n"); return -1; }
    return p->pid;
}

int do_tswitch()
{
    printf("proc %d called tswitch()\n", running->pid);
    tswitch();
    printf("proc %d resumes\n", running->pid);
}

int do_sleep(int event)
{ // shown above
    ksleep(event);
}

int do_wakeup(int event)
{ // shown above
    kwakeup(event);
}

int do_exit(int exitValue)
{
    /*char* s; int s_code;
    printf("Enter exit code: ");
    gets(s); putc('\r');
    s_code = atoi(s);*/
    if (running->pid == 1 && nproc > 2)
    {
        printf("! PROC 1 cannot die. It has child procs\n");
        return -1;
    }
    kexit(exitValue);
}

int do_wait()
{
    int exitCode;
    int childPid = kwait(&exitCode);
    if (childPid == -1)
        printf("PROC %d has no children to wait on\n", running->pid);
    else
        printf ("child PROC %d has been laid to rest. child PROC exit code = %d\n", childPid, exitCode);
}
