// ucode.c file

char *cmd[]={"getpid", "ps", "chname",  "kmode", "switch", "wait", "exit", "fork","exec",
         "pipe", "read", "write", "close", "pfd", "hop", "itimer", 0};

#define LEN 64

int show_menu()
{
   printf("***************** Menu ********************************\n");
   printf("*  ps  chname  switch  kmode  wait  exit  fork  exec  *\n");
   printf("*  hop   pipe   read   write  close pfd   itimer      *\n");
   printf("*******************************************************\n");
}

int find_cmd(char *name)
{
  // return command index
    int i=0; char *p=cmd[0];

    while (p) {
        if (!strcmp(p, name))
                return i;
        i++;
        p = cmd[i];
    }
    return -1;
}

int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   return syscall(1, 0, 0);
}

int chname()
{
    char s[32];
    printf("input new name : ");
    gets(s);
    return syscall(2, s, 0);
}

//int kfork()
//{
//  int child, pid;
//  pid = getpid();
//  printf("proc %d enter kernel to kfork a child\n", pid);
//  child = syscall(3, 0, 0);
//  printf("proc %d kforked a child %d\n", pid, child);
//}

int kmode()
{
    printf("kmode : enter Kmode via INT 80\n");
    printf("proc %d going K mode ....\n", getpid());
    syscall(3, 0, 0);
    printf("proc %d back from Kernel\n", getpid());
}

int kswitch()
{
    return syscall(4,0,0);
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n"); 
} 

int geti()
{
  // return an input integer
    char *s;
    gets(s);
    return atoi(s);
}

int exit()
{
   int exitValue;
   printf("enter an exitValue: ");
   exitValue = geti();
   printf("exitvalue=%d\n", exitValue);
   printf("enter kernel to die with exitValue=%d\n", exitValue);
   _exit(exitValue);
}

int _exit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}

int ufork()
{
  int child;
  child = syscall(7,0,0,0);
  (child) ? printf("parent ") : printf("child ");
  printf("%d return form fork, child_pid=%d\n", getpid(), child);
  /*if (child)
    printf("parent %d returned form fork with child pid=%d\n", getpid(), child);
  else
    printf("child %d returned from fork, child pid =%d\n", getpid(), child);*/
}

int uexec()
{
  int r;
  char filename[32];
  printf("enter exec filename [from /bin/]: ");
  gets(filename);
  r = syscall(8,filename,0,0);
  printf("exec failed\n");
}

int pd[2];

int upipe()
{
    return syscall(30,pd,0,0);
}

int uread()
{
    char buf[1024];
    int fd, len, r;

    upfd();
    printf("read :\n");
    printf("  enter fd: ");
    fd = geti();
    printf("  enter nbytes: ");
    len = geti();
    printf("\nfd=%d nbytes=%d\n", fd, len);

    r = syscall(31,fd,buf,len);

    if (r >= 0)
    {
        printf("proc %d back to Umode, read %d bytes from pipe\n", getpid(), r);
        buf[r] = 0;         // null character
        printf("%s\n", buf);
    }
    else
        printf("writing to pipe failed\n");
}

int uwrite()
{
    char buf[1024];
    int fd, len, r;

    upfd();
    printf("write :\n");
    printf("  enter fd: ");
    fd = geti();
    printf("  enter text: ");
    gets(buf);
    len = strlen(buf);
    printf("\nfd=%d nbytes=%d text=%s\n", fd, len, buf);

    r = syscall(32,fd,buf,len);

    if (r >= 0)
        printf("proc %d back to Umode, wrote %d bytes to pipe\n", getpid(), r);
    else
        printf("writing to pipe failed\n");
}

int uclose()
{
    int fd;
    upfd();
    printf("Enter fd to close: ");
    fd = geti();
    return syscall(33,fd,0,0);
}

int upfd()
{
    return syscall(34,0,0,0);
}

int uhop()
{
    int r, seg;
    char segment[16];
    if (getpid() != 1)
    {
        printf("Only P1 can be a Casanova process\n");
        return -1;
    }
    printf("enter an segment to hop to : 0x");
    gets(segment);
    // can I use sscanf?
    seg = strtol(segment, 0, 16);
    //printf("segment = %s\n", segment);
    //printf("seg = %x\n", seg);
    r = syscall(50, seg, 0, 0);
    printf("proc %d back from hop at segment=%x\n", getpid(), r);
}

int utime_slice(int s)
{
  return syscall(51,s,0,0);
}

int u_itimer()
{
  char time[16];
  printf("input a timer value: ");
  gets(time);
  return syscall(52, atoi(time), 0, 0);
}


int getc()
{
    return syscall(97,0,0); // 127 in ASCII table
    //printf("getc went to kernel and returned a %c\n", r);
    //return r;
}

int putc(char c)
{
    return syscall(98,c,0,0);
}
