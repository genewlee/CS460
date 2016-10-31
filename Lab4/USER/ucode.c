// ucode.c file

char *cmd[]={"getpid", "ps", "chname",  "kmode", "switch", "wait", "exit", "fork", "exec", 0};

#define LEN 64

int show_menu()
{
   printf("***************** Menu ********************************\n");
   printf("*  ps  chname  switch  kmode  wait  exit  fork  exec  *\n");
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
    gets(s); putc('\r');
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

int getc()
{
    return syscall(97,0,0); // 127 in ASCII table
    //rintf("getc went to kernel and returned a %c\n", r);
    //return r;
}

int putc(char c)
{
    return syscall(98,c,0,0);
}
