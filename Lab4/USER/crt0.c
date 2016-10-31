
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
    argc = 0;
    temp = strtok(path, " ");
    while (temp != 0)
    {
        argv[argc] = temp;
        argc++;
        temp = strtok(0, " ");
    }
}

int main0(char *s) // *s is the original command line “cmd a1 a2 ... an”
{
    int i;

    printf("in main0: command=%s\n", s);
    //tokenize *s into char *argv[ ] with argc = number of token strings;
    parse(s);
    /*for (i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);*/
    main(argc, argv);
}
