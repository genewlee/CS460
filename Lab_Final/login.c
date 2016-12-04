#include "ucode.c"

char *tty, *token, *upwlines[128], *upwline[10];
char buffer[2048], username[64], password[64], user[64], pw[64], home[64], program[64];
int uid, gid, i, j, valid;
int stdin, stdout, stderr, upwfd;

int main(int argc, char *argv[])   // invoked by exec("login /dev/ttyxx")
{
  i = 0; j = 0; valid = 0;
  tty =  argv[1];

 //1. close(0); close(1); close(2); // login process may run on different terms
  close (0); close (1); close(2);

 //2. // open its own tty as stdin, stdout, stderr
  stdin = open(tty, O_RDONLY);
  stdout = open(tty, O_WRONLY);
  stderr = open(tty, O_RDWR);

 //3. settty(tty);   // store tty string in PROC.tty[] for putc()
  settty(tty);

  // NOW we can use printf, which calls putc() to our tty
  printf("LOGIN : open %s as stdin, stdout, stderr\n", tty);

  signal(2,1);  // ignore Control-C interrupts so that 
                // Control-C KILLs other procs on this tty but not the main sh

  while(1){
    //1. show login:           to stdout
    printf("login: ");
    //2. read user nmae        from stdin
    gets(username);
    //3. show passwd:
    printf("password: ");
    //4. read user passwd
    gets(password);

    //5. verify user name and passwd from /etc/passwd file
    upwfd = open("/etc/passwd", O_RDONLY);
    read(upwfd, buffer, 2048);

    upwlines[i] = strtok(buffer, "\n");
    while (upwlines[i] != NULL)         // each line split by ':'
    {
      upwlines[++i] = strtok(NULL, "\n");
    } 
    
    for (j = 0; upwlines[j] != NULL; j++)
    {
      i = 0;
      upwline[i] = strtok(upwlines[j], ":");
      while (upwline[i] != NULL)        // each part in line split by ':'
      {
        upwline[++i] = strtok(NULL, ":");
      }
      if (strcmp(upwline[0], username)==0)  // matching username?
      {
        strcpy(user, username);
        if (strcmp(upwline[1], password)==0)  // matching password?
        {
          strcpy(pw, password);
          //6. if (user account valid){
          uid = upwline[2];    //setuid to user uid.
          gid = upwline[3];    //set group id
          chuid(uid, gid);
          strcpy(home, upwline[5]);   // get home dir
          strcpy(program, upwline[6]);// get program
          chdir(home);                // set home as cwd
          exec(program);      //exec to the program in users's account
        }
      }
    }
    printf("login failed, try again\n");
  }
  close(upwfd);               // close /etc/passwd
}