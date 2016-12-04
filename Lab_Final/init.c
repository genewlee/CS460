//*************************************************************************
//                      Logic of init.c 
// NOTE: this init.c creates only ONE login process on console=/dev/tty0
// YOUR init.c must also create login processes on serial ports /dev/ttyS0
// and /dev/ttyS1.. 
//************************************************************************

int pid, child, status;
int stdin, stdout;

#include "ucode.c"  //<========== AS POSTED on class website

int s0, s1;

int main(int argc, char *argv[])
{
  //1. // open /dev/tty0 as 0 (READ) and 1 (WRTIE) in order to display messages
  stdin = open("/dev/tty0", O_RDONLY);
  stdout = open("/dev/tty0", O_WRONLY);

  //2. // Now we can use printf, which calls putc(), which writes to stdout
   printf("INIT : fork a login task on console\n"); 
   child = fork();

   if (child)
   {
      //printf("CHILD = %d\n", child);
      printf("INIT: fork login on serial port 0\n");
      s0 = fork();
      if (s0)
      {
        printf("INIT: fork login on serial port 1\n");
        // s1 = fork();
        // if (s1)
        //   parent();
        // else
        //   loginS1();
        parent();
      }
      else
        loginS0();
    }
    else // child exec to login on tty0
    {
      //printf("CHILD ELSE= %d\n", child);
      login();
    }
}       

int login()
{
  exec("login /dev/tty0");
}

int loginS0()
{
  exec("login /dev/ttyS0");
}

int loginS1()
{
  exec("login /dev/ttyS1");
}
      
int parent()
{
  while(1) {
    printf("INIT : waiting .....\n");

    pid = wait(&status);
    //printf("INIT : pid=%d, child=%d, s0=%d, s1=%d\n", pid, child, s0, s1);
    if (pid == child) // if true -> child has died
    {
      printf("HELLO CHILD\n");
      child = fork(); // fork another login child
      if (!child)     // child exec login
        login();
    }
    else if (pid == s0)
    {
      printf("HELLO S0\n");
      s0 = fork();
      if (!s0)
        loginS0();
    }
    else if (pid == s1)
    {
      printf("HELLO S1\n");
      s1 = fork();
      if (!s1)
        loginS1();
    }
    else
      printf("INIT : buried an orphan child %d\n", pid);
  }
}
