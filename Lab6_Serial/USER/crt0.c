
/*
* main0(s) is called from u.s
* s is original passed in command with args
* tokenlize s into char *argv[ ] and call main(argc, argv).
*/


int argc;
char *argv[32], *temp;

// token() breaks up a string into argc of tokens, pointed by argv[]
int parse (char *path)
{
    char *s;
    s = path;
    argc = 0;
    temp = strtok(s, " ");
    while (temp != 0)
    {
        argv[argc++] = temp;
        temp = strtok(0, " ");
    }
}

/* Using KC's function
* for some reason, without calling this (printing)
*  it's causing garbage */
int showarg(argc, argv) int argc; char *argv[];
{
  int i;
  printf("argc=%d ", argc);
  for (i=0; i<argc; i++)
    printf("argv[%d]=%s ", i, argv[i]);
  printf("\n");
}

int main0(char *s)// *s is the original command line “cmd a1 a2 ... an”
{
    if (s != 0)
    {
        printf("in main0: command=%s\n", s);
        parse(s);
    }
    else
        printf("in main0: command=%s\n", "");
    main(argc, argv);
}
