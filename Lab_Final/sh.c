#include "ucode.c"

char *commands[] = {"cd", "logout"};
char buf[64], *cmdline[20];

int getcmd(char *cmd)
{
  int i = 0;
  char *cp = commands[0];
  while(cp)
  {
    if(strcmp(cp, cmd) == 0)
      return i;
    i++;
    cp = commands[i];
  }
  return -1;
}

int main(int argc, char *argv[])
{
  int i = 0;
  //loops forever (until "logout" or Contro-D):
  while (1)
  {
    printf ("glsh : ");
    gets(buf);

    cmdline[i] = strtok(buf, " ");
    while (cmdline[i] != NULL)         // split command by ' '
    {
      cmdline[++i] = strtok(NULL, " ");
    } 

    if (strcmp(cmdline[0], "quit")==0)
    {
      exit(0);
    }

    // // if just ONE cmd:  
    // pid = fork();
    // if (pid==0)
    //    exec(cmdLine);
    // else
    //    pid = wait(&status);
   }

}
