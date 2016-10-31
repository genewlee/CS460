#include "ucode.c"
int color;

/*main()
{ 
   printf("I am in Umode segment=%s\n", getcs());
   while(1);
}*/


main(int argc, char *argv[])
//main()
{
  char name[64]; int pid, cmd;
  int r, i;
  char c;

  printf("enter main() : argc = %d\n", argc);
  for (i=0; i<argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

  while(1){
    pid = getpid();
    color = (pid + 8) % 7 + 1;// plus 1 to not be 0 which is black
    
    //printf("----------------------------------------------\n");
    printf("--------------------U MODE-------------------\n");
    printf("----------------------U1---------------------\n");
    printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
    show_menu();
    printf("Command ? ");
    gets(name);
    if (name[0]==0)
        continue;

    cmd = find_cmd(name);
    switch(cmd){
           case 0 : getpid();   break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode(); break;//kfork();    break;
           case 4 : kswitch();  break;
           case 5 : wait();     break;
           case 6 : exit();     break;

           case 7 : ufork();     break;
           case 8 : uexec();     break;
           /*
           case 7 : printf("enter an char: ");
                    r = getc();
                    printf("getc went to kernel and got a %c\n", r);
                    break;
           case 8 : printf("enter an char: ");
                    c = getc();
                    printf("putc went to kernel and put %c\n", r);
                    break;*/
           default: invalid(name); break;
    }
  }
}



