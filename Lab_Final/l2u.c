#include "ucode.c"

int main (int argc, char *argv[])
{
	int srcfd, destfd, readCount, i;
	char buf[1024];

	if (argc < 2)		// stdin -> piped / write redirected
	{
		srcfd = 0; destfd = 1;
		while (1)
		{
			readCount = read(srcfd, &buf[0], 1);
			if (readCount == 0)
			{
				break;
			}
			if (buf[0] >= 'a' && buf[0] <= 'z')
			{
				buf[0] = 'A' + buf[0] - 'a';
			}
			else if (buf[0] == '\r')
			{
				buf[0] = '\n';
			}
			if (buf[0] == '\n')
			{
				write(2, "\r", readCount);
			}
			write(destfd, &buf, readCount);
		}
	}

	else if (argc < 3)	// not enough args
	{
		printf("format: l2u f1 f2\n");
		exit(-1);
	}

	else	// two files passed in
	{
		srcfd = open(argv[1], O_RDONLY);			// open the source file
		if (srcfd < 0)
		{
			printf("error opening %s\n", argv[1]);
			exit(-1);
		}

		destfd = open(argv[2], O_WRONLY | O_CREAT);	// open the destination file
		if (destfd < 0)
		{
			printf("error opening %s\n", argv[2]);
			exit(-1);
		}

		readCount = read(srcfd, buf, 1024);
		while (readCount > 0)						// copy files
		{
			for (i = 0; i < readCount; i++)
			{
				if (buf[i] >= 'a' && buf[i] <= 'z')
				{
					buf[i] = 'A' + buf[i] - 'a';
				}
			}
			write(destfd, &buf, readCount);
			readCount = read(srcfd, buf, 1024);
		}

		close(srcfd); close(destfd);				// close the fd's
	}
}