#include "ucode.c"

int main (int argc, char *argv[])
{
	int fd; 
	int patternLen, readCount, i, lineNum = 0;
	char linebuf[512], *pline, *crline;

	if (argc < 3 && argc != 2)					// didnt enter all arguments
	{
		printf("format: grep PATTERN FILE\n");
		exit(-1);
	}

	fd = 0; // from stdin?

	patternLen = strlen(argv[1]);

	if (argc == 3)
	{
		fd = open(argv[2], O_RDONLY);
		if (fd < 0)
		{
			printf ("error getting file %s\n", argv[2]); exit (-1);
		}
	}

	while (1)
	{	
		// read in a line
		for (i = 0; i < 512; i++)
		{
			readCount = read(fd, &linebuf[i], 1);
			//if (linebuf[i] == '\r') i--;
			if (linebuf[i] == '\n')// && linebuf[i] != '\r') // '/r' when passed into l2u
			{
				lineNum++;
				linebuf[i + 1] = NULL; break;
			}
		}

		if (readCount == 0)
			break;

		pline = linebuf;
		while (*pline == '\r') pline++; 		// remove leading \r
		crline = pline;//strcpy(crline,pline);					// used so that cr is not perfomed before printing lineNum
		// look through line for pattern match
		for (i = 0; linebuf[i] != '\n'; i++, pline++)
		{
			if (strncmp(pline, argv[1], patternLen) == 0)
			{
				printf("[%d] %s\r", lineNum, crline); break;
			}
		}

		// clear the linebuf
		for (i = 0; i < 512; i++)
		{
			linebuf[i] = NULL;
		}
	}

	close(fd);
	exit(0);
}	