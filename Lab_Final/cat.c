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
	settty(tty);

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
        	if (ch == '\r')			// this takes care of writing to the stdout/redirect/pipe
        		write(1, "\n", 1);
        	else
        		write(1, &ch, 1);

        	if (readredirected && writeredirected) // just cat is between pipes cmd | cat | cmd'
        		continue;						 // we don't really check anything except whats done above

        	else if (readredirected == 0 && writeredirected == 0)	// only cat cmd
        	{
        		buf[j++] = ch;
        		if (ch == '\r')
        		{
        			write(STDERR, &ch, 1);	
					write(STDERR, &buf, strlen(buf));
					write(STDERR, "\n\r", 2);	
					for (j=0; j < 256; j++) { buf[j] = NULL; } j = 0;	// clear out the line buf
        		}
        	}
        	else if (writeredirected && readredirected == 0)		// cat > file
            {
            	write(STDERR, &ch, 1);		// this is just to display
            	if (ch == '\r')
                	write(STDERR, "\n\r", 2);
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