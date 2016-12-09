#include "ucode.c"

int fd, cmdfd;
int readredirected, writeredirected;

int printline()
{
	int i, readCount;
	char c;

	while (read(fd, &c, 1) > 0)
	{
		if (readredirected == 0) // from file 'more f'
		{
			if (c == '\n')
				write(1, "\n\r", 2);
			else
				write(1, &c, 1);
		}
		else // read is redirected // 'cat f | more'
		{
			write(1, &c, 1);
			if (c == '\n')
				write(1, "\r", 1);
		}

		if (c == '\n' || c == '\r') 
			return;
	}

	// if we get here we've read entire file because read's count was <= 0
	exit(0); // so exit 'more' program
}

int printpage()
{
	int j;

	for (j = 0; j < 24; j++)
	{
		printline();
	}
}

int main (int argc, char *argv[])
{
	char c, tty[64];
	STAT st0, st1, st_tty;

	fd = 0; 		// stdin or pipe

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
	
	if (argc > 1)
	{
		fd = open(argv[1], O_RDONLY);
		if (fd < 0)
		{
			printf ("error getting file %s\n", argv[2]); exit (-1);
		}
	}
	else if (argc == 1 && readredirected) // read redirected
	{
		cmdfd = open(tty, O_RDONLY); //since reading from stdin (fd = 0), we need commands from tty
	}

	//printf ("argc = %d, fd = %d\n", argc, fd);

	printpage();	// prints one page first
	while (1)
	{
		if (readredirected) // need to get commands from user from tty
			read(cmdfd, &c, 1);
		else 
			c = getc();
		switch (c)
		{
			case '\n':
				printline(); break;

			case '\r':
			 	printline(); break;

			case ' ':
				printpage(); break;

			case 'q':
				exit(0);

			default:
				printline(); break;
		}
	}
	close(fd);
}
