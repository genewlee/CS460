#include "ucode.c"

int main (int argc, char *argv[])
{
	int srcfd, destfd;
	int readCount;
	char buf[1024];

	printf("\n ================ Gene's cp ================\n\r");

	if (argc < 3)								// didnt enter all arguments
	{
		printf("Enter two files: src dest\n");
		exit(-1);
	}

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
		write(destfd, &buf, readCount);
		readCount = read(srcfd, buf, 1024);
	}

	close(srcfd); close(destfd);				// close the fd's
}