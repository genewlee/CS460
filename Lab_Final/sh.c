#include "ucode.c"

char *commands[] = {"?", "cd", "pwd", "mkdir", "rmdir", "creat", "rm", 
                    "chmod", "chown", "logout", "quit"};
char buf[64], *CMDLine, *cmdline[20];

int status;

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

int menu()
{
  printf("#############################################################\n\r");
  printf("#  ls       cd     pwd    cat   more    cp          >   >>  #\n\r");
  printf("#  mkdir    rmdir  creat  rm    chmod   chown       <   |   #\n\r");
  printf("#############################################################\n\r");
}

int main(int argc, char *argv[])
{
  int i, cmd, pid, ampersand = 0;
  
  setcolor(0);
  //loops forever (until "logout" or Contro-D):
  while (1)
  {
    i = 0;
    printf ("glsh : ");
    gets(buf);
    strcpy(CMDLine, buf);

    // tokenize command line
    cmdline[i] = strtok(buf, " ");
    while (cmdline[i] != NULL)         // split command by ' '
    {
      //printf("cmdline[%d] = %s\n", i, cmdline[i]);
      if (strcmp(cmdline[i], "&") == 0) ampersand = 1;
      cmdline[++i] = strtok(NULL, " ");
    }

    // built in commands -> do the cmd directly
    cmd = getcmd(cmdline[0]);
    switch (cmd) {
      case 0: // show menu
              menu(); continue;
      case 1: // cd
              if (cmdline[1] == NULL)
                chdir("/");
              else
                chdir(cmdline[1]);
              continue;
      case 2: // pwd
              pwd(); continue;
      case 3: // mkdir
              mkdir(cmdline[1]); continue;
      case 4: // rmdir
              rmdir(cmdline[1]); continue;
      case 5: // creat
              creat(cmdline[1]); continue;
      case 6: // rm
              unlink(cmdline[1]); continue;
      case 7: // chmod
              chmod(cmdline[1], atoi(cmdline[2])); continue;
      case 8: // chown
              chown(cmdline[1], atoi(cmdline[2])); continue;
      case 9: // logout
              exit(0); continue;
      case 10: // quit
              exit(0); continue;
      default: 
              if (cmdline[0] == NULL) continue;
              else break;
    }

    // for binary executable command  
    pid = fork();
    if (pid)   // parent sh
    {
      // check for & symbol
      if (ampersand)
      {
        tswitch(); continue;
      }

      pid = wait(&status);
      continue;
    }
    else      // child sh exec
    {
      do_pipe(CMDLine, 0);
    }
   }
}

int do_pipe(char *cmdLine, int *pd)
{  
  int hasPipe, pid, lpd[2];

  char *head, *tail;

  if (pd)   // if has pipe passed in, as WRITER on right side pipe pd
  {
    close (pd[0]); dup2(pd[1], 1); close(pd[1]);
  }
  tail = head = cmdLine;
  hasPipe = scan(cmdLine, &head, &tail);
  // printf("TAIL = %s\n", tail);
  // printf("HEAD = %s\n", head);   return;
  //printf("hasPipe = %d\n", hasPipe); //return;
  if (hasPipe)
  {
    // create a pipe lpd;
    pipe(lpd);  // returns a pid
    
    pid = fork();
    if (pid) // parent
    {
      // as READER on lpd:
      close(lpd[1]); dup2(lpd[0], 0); close(lpd[0]);
      //printf("TAIL = %s\n", tail);
      do_command(tail);
    }
    else // child handles the head 
    {
      //printf("HEAD = %s\n", head);
      do_pipe(head, lpd);
    }
  }
  else{
    do_command(cmdLine);
  }
}

int do_command(char* cmdLine)
{
  char cmd[64], *token, *head, *tail;
  int i = 0, cmdlen = strlen(cmdLine), redirection = 0, fd;

  // scan cmdLine for I/O redirection symbols;
  strcpy(cmd, cmdLine);
  while (cmd[i] != NULL) 
  {
    if (cmd[i] == '<' || cmd[i] == '>') 
    {
      redirection = 1;    // there is a redirection symbol
      break;
    }
    i++;
  } 

  //printf("cmdLine[%d] = %c\n", i, cmd[i]);

  // do I/O redirections;
  if (redirection)
  {
    if (cmd[i + 1] == '>')        // append to file
    {
      head = strtok(cmd, ">>");   // dont want a space because it will token out the file name
      tail = strtok(NULL, ">> "); // removes whitespace to get correct file
      close(1);
      fd = open(tail, O_WRONLY | O_APPEND);
    }
    else if (cmd[i] == '>')       // write to file
    {
      head = strtok(cmd, ">");
      tail = strtok(NULL, "> ");
      //printf("head = %s\n", head);
      close(1);
      fd = open(tail, O_WRONLY|O_CREAT);
    }
    else if (cmd[i] == '<')       // read from file
    {
      head = strtok(cmd, "<");
      tail = strtok(NULL, "< ");
      //printf ("head = %s, tail = %s\n", head, tail);
      close(0);
      fd = open(tail, O_RDONLY);
    }
    exec(head);
  }
  else
  {
    //printf("cmdline[%d] = %s\n", 0, cmdline[0]);
    exec(cmdLine);
  }
}

int scan (char *cmdLine, char **head, char **tail)
{
  /* divide cmdLine into head and tail by rightmost | symbol
  *  cmdLine = cmd1 | cmd2 | ... | cmdn-1 | cmdn
  *            |<------- head ----------> | tail |; return 1;
  *  cmdLine = cmd1 ==> head=cmd1, tail=null;       return 0; */

  int i, j;
  // divide cmdLine into head, tail by rightmost pipe symbol
  // i is the index of rightmost pipe symbol '|'
  // tail = head = cmdLine;
  for (i = strlen(cmdLine) - 1; *(cmdLine + i) != NULL && *(cmdLine + i) != '|'; i--);
  
  // set tail
  for (j = 0; j <= i; j++) (*tail)++;
  while (**tail == ' ') (*tail)++; // remove leading whitespace in tail, also for checking no pipe
  
  // set head
  *(*head + i) = NULL;

  //printf("i = %d, TAIL = '%s'\n", i, *tail);
  //printf ("tail == null : %d", strcmp(*tail, ""));
  //printf("i = %d, HEAD = '%s'\n", i, *head);   //return;

  // no pipe // if i meets this then nothing on right side of pipe
  if ((*head == *tail) || (strcmp(*tail, "") == 0)) return 0;
  //printf ("THERE IS A PIPE\n");
  return 1; // otherwise there is a pipe
}
