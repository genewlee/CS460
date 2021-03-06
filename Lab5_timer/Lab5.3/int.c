
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/
int kcinth()
{
   int a,b,c,d, r;
   int segment, offset;
   segment = running->uss;
   offset = running->usp;

//==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack
   a = get_word(segment, offset + (2 * 13));
   b = get_word(segment, offset + (2 * 14));
   c = get_word(segment, offset + (2 * 15));
   d = get_word(segment, offset + (2 * 16));

   switch(a){
       case 0 : r = kgetpid();        break;
       case 1 : r = kps();            break;
       case 2 : r = kchname(b);       break;
       case 3 : r = kkmode();         break;//r = kkfork();         break;
       case 4 : r = ktswitch();       break;
       case 5 : r = kkwait(b);        break;
       case 6 : r = kkexit(b);        break;

       case 7 : r = fork();           break;
       case 8 : r = exec(b);          break;

       case 30: r = kpipe(b);         break;
       case 31: r = read_pipe(b,c,d); break;
       case 32: r = write_pipe(b,c,d); break;
       case 33: r = close_pipe(b);    break;
       case 34: r = pfd();            break;

       case 50: r = hop(b);           break;
       case 51: r = ktime_slice(b);   break;
       case 52: r = itimer(b);        break;
       
       case 97: r = getc();           break;
       case 98: color=running->pid + 7;
                r = putc(b);          break;
       case 99: kkexit(b);            break;
       default: printf("invalid syscall # : %d\n", a); 
   }

//==> WRITE CODE to let r be the return value to Umode
   put_word(r, segment, offset + (2 * 8)); // ax which is at offset 8 contains return value to Umode
}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
    //WRITE YOUR C code
    return running->pid;
}

char *status[ ] = {"FREE", "READY", "RUNNING", "STOPPED", "SLEEP", "ZOMBIE"};

int kps()
{
    int i;
    char buf[32];

    //WRITE C code to print PROC information
    printf("===============================================\n");
    printf("name            status       pid     ppid   uss\n");
    printf("===============================================\n");

    for (i=0; i<NPROC; i++)
    {
        strcpy(buf, "                ");    // to set a lenght for formatting
        strncpy(buf, proc[i].name, strlen(proc[i].name)); // only copy over the length of string so that ws is preserverd after
        printf("%s", buf);          // print the formatted name
        if (&proc[i] == running)
            strcpy(buf, "running"); // print "running" if it is the running process
        else
        {    // or just print the status
            strcpy(buf, "       "); // to set a lenght for formatting
            strncpy(buf, status[proc[i].status], strlen(status[proc[i].status]));
        }
        printf("%s      ", buf);            // print the status
        printf("%d      ", proc[i].pid);    // print the pid
        if (proc[i].status != FREE)         // just to display nicely only print if its not free
        {
            printf("%d     ", proc[i].ppid);  // print the ppid
        }
        else
            printf("      ");                 // print empty
        if (proc[i].status != FREE)
            printf("%x      ", proc[i].uss);    // print the pid
        printf("\n");
    }
}

int kchname(char *name)
{
    //WRITE C CODE to change running's name string;
    char buf[32], before[32];
    int i =0;
    strcpy(before, running->name); // storing name before

    while (i < 32) // size of name is limited to 32 bits
    {
        buf[i] = get_word(running->uss, name);
        i++; name++;
    }
    strcpy(running->name, buf);
    printf("Changed proc %d name from <%s> to <%s>\n", running->pid, before, buf);
}

int kkfork()
{
  //use you kfork() in kernel;
  //return child pid or -1 to Umode!!!
    PROC* childProc = kfork("/bin/u1");
    if (childProc == 0)
        return -1;
    return childProc->pid;
}

int kkmode()
{
    body();
}

int ktswitch()
{
    return tswitch();
}

int kkwait(int *status)
{

  //use YOUR kwait() in LAB3;
  //return values to Umode!!!
    int exitCode, childPid;

    childPid = kwait(&exitCode);        // grabs the exitcode of the child and assigns it
    put_word(exitCode, running->uss, status); // send the exitCode to the address of umode status
    return childPid;
}

int kkexit(int value)
{
    //use your kexit() in LAB3
    //do NOT let P1 die
    kexit(value);       // exit and pass in the exitCode value of how it exited
}

int ktime_slice(int s)
{
  running->time = s;
}

