#include "ucode.c"

#define STDERR 2

int main (int argc, char*argv[])
{
	int cfd, ttyfd, readCount;
	int i = 0, j;
	char ch, buf[256], tty[64];

	STAT st0, st1, st_tty;
	int readredirected, writeredirected;

	char *welcome = "\n ================== Gene's almost as cool as KC's cat MEOW! ==================\n\r";
	
	write(STDERR, welcome, strlen(welcome));

	cfd = 0; // from stdin

	gettty(&tty);
	stat(tty, &st_tty);
	fstat(0, &st0); 
	fstat(1, &st1);

  	readredirected = 1;
	if (st_tty.st_dev == st0.st_dev && st_tty.st_ino == st0.st_ino)
		readredirected = 0;

	writeredirected = 1;
	if (st_tty.st_dev == st1.st_dev && st_tty.st_ino == st1.st_ino)
		writeredirected = 0;

	//printf ("readredirected %d, writeredirected %d\n", readredirected, writeredirected);

	ttyfd = open(tty, O_WRONLY);

	if (argc > 1)
	{
		cfd = open(argv[1], O_RDONLY);
	}

	if (cfd < 0)
	{
		printf ("MEOW error\n"); exit (-1);
	}

    while (read(cfd, &ch, 1) > 0)
    {
        if (cfd == 0) 		// from stdin
        {
        	if (ch == '\r')
        		write(1, "\n", 1);
        	else
        		write(1, &ch, 1);

        	if (readredirected && writeredirected) // just cat is between pipes cmd | cat | cmd'
        		continue;						 // we don't really check anything except whats done above

        	if (readredirected == 0 && writeredirected == 0)
        		buf[j++] = ch;
        	
        	if (readredirected == 0 && writeredirected == 0 && ch == '\r') // for only cat command
        	{
        		write(STDERR, "\r", 1);	
				write(STDERR, &buf, strlen(buf));
				write(STDERR, "\n\r", 2);	
				for (j=0; j < 256; j++) buf[j] = NULL;
				j = 0;
        	}
        	if (readredirected == 0 && ch == '\n')	// just cat command 
    		{
        		write(STDERR, "\r", 1);		
        	}
            if (writeredirected && readredirected == 0)		// cat > file
            {
            	write(STDERR, &ch, 1);
            	if (ch == '\n')
                	write(STDERR, "\r", 1);
            }	
        }
        else				// not from stdin
        {
        	write(1, &ch, 1); // write to stdout/redirect/pipe 
            if (ch == '\n')
            {
                write(STDERR, "\r", 1);
            }
        }
	}
	close(cfd);
}


















// #include "ucode.c"

// #define STDERR 2

// int main (int argc, char*argv[])
// {
// 	int cfd, readCount;
// 	int i = 0, j;
// 	char ch, buf[128], tty[64];

// 	STAT st0, st1, st_tty;
// 	int readredirected, writeredirected;

// 	char *welcome = "\n ================== Gene's almost as cool as KC's cat MEOW! ==================\n\r";
	
// 	write(STDERR, welcome, strlen(welcome));

// 	cfd = 0; // from stdin

// 	gettty(&tty);
// 	stat(tty, &st_tty);
// 	fstat(0, &st0); 
// 	fstat(1, &st1);
// 	if (st_tty.st_dev != st0.st_dev && st_tty.st_ino != st0.st_ino)
// 		readredirected = 1;
// 	else 
// 		readredirected = 0;
// 	if (st_tty.st_dev != st1.st_dev && st_tty.st_ino != st1.st_ino)
// 		writeredirected = 1;
// 	else
// 		writeredirected = 0;

// 	if (argc > 1)
// 	{
// 		cfd = open(argv[1], O_RDONLY);
// 	}

// 	if (cfd < 0)
// 	{
// 		printf ("MEOW error\n"); exit (-1);
// 	}

//     //printf ("readredirected %d, writeredirected %d\n", readredirected, writeredirected);
//     while (read(cfd, &ch, 1) > 0)
//     {
//         if (cfd == 0) 		// from stdin
//         {
//         	if (ch == '\r')
//         	{
//         		write(1, "\n", 1);
//         		buf[j++] = ch;
//         	}
//         	else
//         	{
//         		write(1, &ch, 1);
//         		buf[j++] = ch;
//         	}
//         	if (readredirected && writeredirected) // just cat is between pipes cmd | cat | cmd'
//         		continue;							// we don't really need to write anything -> pass it along
//         	if (readredirected == 0 && writeredirected == 0 && ch == '\r') // for only cat command
//         	{
//         		write(STDERR, "\r", 1);	
// 				write(STDERR, &buf, strlen(buf));
// 				write(STDERR, "\n\r", 2);	
// 				for (j=0; j < 128; j++) buf[j] = NULL;
// 				j = 0;
//         	}
//         	if (readredirected == 0 && ch == '\n')	// just cat command 
//     		{
//         		write(STDERR, "\r", 1);		
//         	}
//             if (writeredirected && readredirected == 0)		// cat > file
//             {
//             	write(STDERR, &ch, 1);
//             	if (ch == '\n')
//                 	write(STDERR, "\r", 1);
//             }	
//         }
//         else				// not from stdin
//         {
//         	write(1, &ch, 1); // write to stdout/redirect/pipe 
//             if (ch == '\n')
//             {
//                 write(STDERR, "\r", 1);
//             }
//         }
// 	}
// 	close(cfd);
// }