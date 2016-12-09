#include "type.h"


int falloc(OFT* oftp)
{
	int i = 0;
	for (i = 0; i < NFD; i++)
	{
		if (running->fd[i] == NULL)
			break;
	}

	if (i == NFD)
	{
		return -1;
	}

	running->fd[i] = oftp;
	return i;
}

int truncate(MINODE *mip)
{
	//printf("Wer are in truncate\n");
	char tempBuff[256];
	int bnumber, i, j;

	//Null guard
	if (mip == NULL)
	{
		printf("mip is null reference.\n");
	}

	/*1. release mip->INODE's data blocks;
	a file may have 12 direct blocks, 256 indirect blocks and 256 * 256
	double indirect data blocks.release them all.*/

	for (i = 0; i < 12; i++)
	{
		if (mip->INODE.i_block[i] != 0)
		{
			bdealloc(mip->dev, mip->INODE.i_block[i]);
		}
	}

	/*For indirect blocks.*/

	//No need to continue if there are no indirect blocks.
	if (mip->INODE.i_block[12] == 0)
	{
		return 1;
	}

	get_block(dev, mip->INODE.i_block[12], buf);
	for (i = 0; i < 256; i++)
	{
		if (buf[i] != 0)
		{
			bdealloc(mip->dev, buf[i]);
		}
	}

	/*For double indirect blocks.*/

	get_block(mip->dev, mip->INODE.i_block[13], buf);
	for (i = 0; i < 256; i++)
	{
		if (buf[i])
		{
			get_block(mip->dev, buf[i], tempBuff);

			for (j = 0; j < 256; j++)
			{
				if (tempBuff[j] != 0)
				{
					bdealloc(mip->dev, tempBuff[j]);
				}
				bdealloc(mip->dev, buf[i]);
			}
		}
	}
	bdealloc(mip->dev, mip->INODE.i_block[13]);

	/*2. update INODE's time field*/
	//assigning them all at the same time.
	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);

	/*3. set INODE's size to 0 and mark Minode[ ] dirty*/
	mip->INODE.i_size = 0;
	mip->dirty = 1;
	return 1;
}

int open_fileH(char* path, char newmode)
{
	/*1. ask for a pathname and mode to open:
         You may use mode = 0|1|2|3 for R|W|RW|APPEND*/
	
	// pathname is global pathname
	// mode is global parameter
	int ino, i, fdindex, mode = newmode - '0';// = strtol(newmode, (char **)NULL, 10);
	MINODE *mip;
	OFT* oftp;
	//printf("MODE:%d\n", mode);
	/*  2. get pathname's inumber:
         if (pathname[0]=='/') dev = root->dev;          // root INODE's dev
         else                  dev = running->cwd->dev;  
         ino = getino(&dev, pathname);  // NOTE: dev may change with mounting*/
	if (pathname[0] == '/')
	{
		dev = root->dev;
	}
	else
	{ 
		dev = running->cwd->dev;
	}
	
	ino = getino(&dev, pathname);
	if (ino == 0) //getino returned an error
	{
		return -1;
	}
	/* 3. get its Minode pointer
         mip = iget(dev,ino);  */
	mip = iget(dev, ino);
	

	/* 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.*/
	if ((mip->INODE.i_mode & 0xF000) != 0x8000)
	{
		printf("file must be regular\n");
		iput(mip);
		return -1;
	}

	/*Check whether the file is ALREADY opened with INCOMPATIBLE mode:
	If it's already opened for W, RW, APPEND : reject.
	(that is, only multiple R are OK)*/
	for (i = 0; i < NFD; i++)
	{
		if (running->fd[i] != NULL)
		{
			if (running->fd[i]->minodeptr == mip) // found
			{
				if (running->fd[i]->mode > 0)
				{
					printf("file is already opened for writing\n");
					iput(mip);
					return -1;
				}
			}
		}
	}

	/*5. allocate a FREE OpenFileTable (OFT) and fill in values:*/
	oftp = malloc(sizeof(OFT));

	oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	oftp->minodeptr = mip;  // point at the file's minode[]

	/*6. Depending on the open mode 0 | 1 | 2 | 3, set the OFT's offset accordingly:*/

	switch (oftp->mode) {
		case 0: oftp->offset = 0;     // R: offset = 0
			break;
		case 1: truncate(mip);        // W: truncate file to 0 size
			oftp->offset = 0;
			break;
		case 2: oftp->offset = 0;     // RW: do NOT truncate file
			break;
		case 3: oftp->offset = mip->INODE.i_size;  // APPEND mode
			break;
		default: printf("invalid mode\n");
			//running->fd[index] = 0;
			iput(mip);
			return(-1);
		}

	/*7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
	/*Let running->fd[i] point at the OFT entry*/
	fdindex = falloc(oftp);
	if (fdindex == -1)
	{
		printf("No open fd slots in running process \n");
		iput(mip);
		return -1;
	}

	/*8. update INODE's time field
         for R: touch atime. 
         for W|RW|APPEND mode : touch atime and mtime*/
	if (mode == 0)
	{
		mip->INODE.i_atime = time(0L);
	}
	else if (mode == 1 || mode == 2 || mode == 3)
	{
		mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	}

    //mark Minode[ ] dirty*/
	mip->dirty = 1;
	iput(mip);
	/*  9. return i as the file descriptor*/
	return fdindex;
}

int open_file()
{
	if ((pathname[0] == '\0') || (parameter[0] == '\0'))
	{
		printf("cmd syntax: open [pathname] [mode]\n");
		printf("modes: READ = 0, WRITE = 1, RW = 2, APPEND = 3\n");
		return -1;
	}
	open_fileH(pathname, parameter[0]);
}

//seeks to inputted offset of open file
int l_seek()
{
	int offset = atoi(parameter);
	int fd = atoi(name[0]);

	//From fd, find the OFT entry.
	if ((pathname[0] == '\0') || (parameter == '\0'))
	{
		printf("cmd syntax: lseek [fd] [offset]\n");
		return -1;
	}

	//change OFT entry's offset to position but make sure NOT to over run either end
	//of the file.
	
	if (running->fd[fd]->minodeptr->INODE.i_size < offset)
	{
		printf("file run over\n");
		return -1;
	}
	else if (offset < 0)
	{
		printf("file cannot seek negative offset\n");
		return -1;
	}
	int originalPosition = running->fd[fd]->offset;
	running->fd[fd]->offset = offset;
	
	//return originalPosition
	return originalPosition;
}

int pfd()
{
	//This function displays the currently opened files as follows :

	/*fd     mode    offset    INODE
		----    ----    ------   --------
		0     READ    1234[dev, ino]
		1     WRITE      0[dev, ino]
		--------------------------------------
		to help the user know what files has been opened.*/

	int i;
	printf("\tfd\tmode\toffset\tINODE\n");
	printf("\t----\t----\t-----\t-------\n");
	for (i = 0; i < NFD; i++)
	{
		if (running->fd[i] != NULL)
		{
			printf("\t%d\t", i);
			switch (running->fd[i]->mode)
			{
			case 0:
				printf("READ\t");
				break;
			case 1:
				printf("WRITE\t");
				break;
			case 2:
				printf("R/W\t");
				break;
			case 3:
				printf("APPEND\t");
				break;
			default:
				printf("no mode\t");
				break;
			}
			printf("%li\t", running->fd[i]->offset);
			printf("[%d, %d]\n", running->fd[i]->minodeptr->dev, running->fd[i]->minodeptr->ino);
		}
	}
	return 0;
}

int myread(int fd, char *buf, int nbytes)
{
	MINODE * mip = running->fd[fd]->minodeptr;
	OFT* oftp = running->fd[fd];
	//1. 
	int count = 0, lbk, startByte, blk, dblk;
	//avil = fileSize - OFT's offset // number of bytes still available in file.
	int avil = mip->INODE.i_size - oftp->offset, remain;
	char *cq = buf, indirectbuf[BLOCK_SIZE], dblindirectbuf[BLOCK_SIZE];


	while (nbytes && avil)
	{
		//Compute LOGICAL BLOCK number lbk and startByte in that block from offset;
		// the current byte position, offset falls in a LOGICAL block(lbk)
		lbk = oftp->offset / BLKSIZE;
		// the byte to start read in that logical block is 
		startByte = oftp->offset % BLKSIZE;

		if (lbk < 12) //read DIRECT BLOCKS
		{             // lbk is a direct block
			blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
		}
		else if (lbk >= 12 && lbk < 256 + 12) //  indirect blocks 
		{
			get_block(mip->dev, mip->INODE.i_block[12], indirectbuf);
			blk = indirectbuf[lbk - 12];
		}
		else //  double indirect blocks
		{
			get_block(mip->dev, mip->INODE.i_block[13], dblindirectbuf);
			lbk -= 12 + 256;
			dblk = dblindirectbuf[lbk / 256];
			get_block(mip->dev, dblk, dblindirectbuf);
			blk = dblindirectbuf[lbk % 256];
		}
		/* get the data block into readbuf[BLKSIZE] */
		get_block(mip->dev, blk, buf);

		/* copy from startByte to buf[ ], at most remain bytes in this block */
		char *cp = buf + startByte;
		remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
		
		while (remain > 0) {
			*cq++ = *cp++;             // copy byte from readbuf[] into buf[]
			oftp->offset++;           // advance offset 
			count++;                  // inc count as number of bytes read
			avil--; nbytes--;  remain--;
			if (nbytes <= 0 || avil <= 0)
				break;
		}
		// if one data block is not enough, loop back to OUTER while for more ...
	}
	//printf("myread: read %d char from file descriptor %d\n", count, fd);
	return count;   // count is the actual number of bytes read
}

int read_fileH(int fd, int nbytes)
{
	printf("fd=%d nbytes=%d\n", fd, nbytes);
	/*Preparations:
		ASSUME: file is opened for RD or RW;
		ask for a fd  and  nbytes to read;
		verify that fd is indeed opened for RD or RW;
		return(myread(fd, buf, nbytes));*/
	char buf[BLOCK_SIZE];
	
	if ((fd >= NFD) || (fd < 0))
	{
		printf("CMD: read [fd] [nbytes]\n");
		return -1;
	}
	if (running->fd[fd] == NULL)
	{
		printf("invalid fd\n");
		return -1;
	}
	if ((running->fd[fd]->mode != 0) && (running->fd[fd]->mode != 2)) // 0 = read , 2 = RW
	{
		printf("file must be opened for read\n");
		return -1;
	}
	return myread(fd, buf, nbytes);
}


int read_file()
{
	return read_fileH(strtol(pathname, (char**)NULL, 10), strtol(parameter, (char**)NULL, 10));
}


int cat()
{
	char catbuf[1024], dummy = 0, i;  // a null char at end of mybuf[ ]
	int n, fd;
	char buff[1024];
	MINODE *mip;

	//int ino = getino(&dev, pathname);
	//mip = iget(dev, ino);

	//if ((mip->INODE.i_mode & 0xA000) == 0xA000)
	//{
	//	
	//	strncpy(buff, mip->INODE.i_block[0], mip->INODE.i_size);
	//	printf("%s\n", buff);
	//	fd = open_fileH(buff, '0');
	//}
	//else
	//{
		fd = open_fileH(pathname, '0');
	//}
	
	if (fd == -1) 
	{
		printf("%s is currently open\n", pathname);
		return -1;
	}

	while (n = myread(fd, catbuf, 1024))
	{
		buf[n] = 0;
		printf("%s", catbuf);
	}
	close_fileH(fd);
}

int close_fileH(int fd)
{
	//1. verify fd is within range.
	if ((fd >= NFD) || (fd < 0))
	{
		printf("invalid fd\n");
		return -1;
	}

	//2. verify running->fd[fd] is pointing at a OFT entry
	if (running->fd[fd] == NULL)
	{
		printf("fd not found\n");
		return -1;
	}

	//3. The following code segments should be fairly obvious :
	OFT * oftp = running->fd[fd];
	running->fd[fd] = 0;
	oftp->refCount--;
	if (oftp->refCount > 0) return 0;

	// last user of this OFT entry ==> dispose of the Minode[]
	MINODE * mip = oftp->minodeptr;
	iput(mip);

	return 0;
}

int close_file()
{
	if (strcmp(pathname, "") == 0)
	{
		printf("CMD: close [fd]\n");
		return -1;
	}
	return close_fileH(atoi(pathname));
}

int copy_file(char *src, char *dest)
{
	char cpbuf[BLKSIZE];
	int n;
	//1. fd = open src for READ;
	int fd = open_fileH(src, '2');
	if (fd == -1)
	{
		return -1;
	}

	//2. gd = open dst for WR | CREAT;
	/*NOTE:In the project, you may have to creat the dst file first, then open it
	for WR, OR  if open fails due to no file yet, creat it and then open it
	for WR.*/
	int gd = open_fileH(dest, '2');
	/*if (gd == -1)
	{
		creat_file(dest);
		gd = open_fileH(dest, '2');
	}*/

	//3. 
	while (n = myread(fd, cpbuf, BLKSIZE))
	{
		mywrite(gd, cpbuf, n);  // notice the n in write()
	}
	close_fileH(fd);
	close_fileH(gd);
	return 0;
}


int cp_file() //cp src dest
{
	if (strcmp(pathname, "") == 0 || strcmp(parameter, "") == 0)
	{
		printf("CMD: cp [src] [dest]\n");
		return -1;
	}
	//src == name[0]
	//dest == parameter;
	printf("path=%s\n", pathname);
	printf("para=%s\n", parameter);
	copy_file(pathname, parameter);
}

int move_file(char *src, char *dest) //mv src dest
{
	MINODE *mip;
	int fd, fdino;
	//1. verify src exists; get its INODE in == > you already know its dev
	fd = open_file(src, '0'); //dev
	if (fd == -1)
	{
		return -1;
	}
	fdino = getino(&dev, src);
	mip = running->fd[fd]->minodeptr;
	//2. check whether src is on the same dev as src

	//CASE 1: same dev :
	if (mip->dev == fd)
	{
		//3. Hard link dst with src(i.e.same INODE number)
		link(src, dest);
		//4. unlink src(i.e.rm src name from its parent directory and reduce INODE's link count by 1).
		unlink(src);
	}
	else //CASE 2 : not the same dev :
	{
		//3. cp src to dst
		copy_file(src, dest);
		//4. unlink src
		unlink(src);
	}
	return 0;
}
 
int mv_file()
{
	if (strcmp(name[0], "") == 0 || strcmp(pathname, "") == 0)
	{
		printf("CMD: mv [src] [dest]\n");
		return -1;
	}
	//src == name[0]
	//dest == parameter;
	move_file(name[0], parameter);
}

int write_file()
{
	int fd, nbytes;

	// 1. ask for a fd and a text string to write
	// fd is in string form in pathname, and text string is stored in parameter
	fd = strtol(pathname, (char **)NULL, 10);

	// 2. verify fd is indeed opened for WR or RW or APPEND mode
	if ((fd >= NFD) || (fd < 0))
	{
		printf("CMD: write [fd] [string]\n");
		return -1;
	}
	if (running->fd[fd] == NULL)
	{
		printf("invalid fd\n");
		return -1;
	}
	if (running->fd[fd]->mode == 0)
	{
		printf("file must be opened for write\n");
		return -1;
	}

	// 3. copy the text string into a buf[] and get its length as nbytes.
	strcpy(buf, parameter);
	nbytes = strlen(buf);

	return(mywrite(fd, buf, nbytes));
}

int mywrite(int fd, char buf[], int nbytes)
{
	int lbk, startByte, blk, dblk, iblk;
	int ibuff[256], dibuff[256];
	OFT *oftp = running->fd[fd];
	MINODE*mip = running->fd[fd]->minodeptr;
	//avil = fileSize - OFT's offset // number of bytes still available in file.
	int avil = mip->INODE.i_size - oftp->offset, remain;
	char *cq = buf, indirectbuf[BLOCK_SIZE], dblindirectbuf[BLOCK_SIZE];

	while (nbytes > 0)
	{
		// compute LOGICAL BLOCK(lbk) and the startByte in that lbk :

		lbk = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;

		// I only show how to write DIRECT data blocks, you figure out how to 
		// write indirect and double-indirect blocks.

		if (lbk < 12)
		{   // direct block
			if (mip->INODE.i_block[lbk] == 0)
			{   // if no data block yet
				mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block

				 // write a block of 0's to blk on disk: OPTIONAL for data block 
				//  but MUST for I or D blocks
			}
			blk = mip->INODE.i_block[lbk];   // blk should be a disk block now
		}
		else if (lbk >= 12 && lbk < 256 + 12)
		{
			// indirect blocks
			if (mip->INODE.i_block[12] == 0) // has allocated indirect
			{
				iblk = balloc(dev);
				mip->INODE.i_block[12] = iblk;
			}
			memset((char *)ibuff, 0, BLKSIZE);
			get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuff);
			blk = ibuff[lbk - 12];
			if (blk == 0)
			{
				iblk = balloc(dev);
				ibuff[lbk - 12] = iblk;
				put_block(mip->dev, mip->INODE.i_block[12], (char *)ibuff);
			}

		}
		else
		{
			// double indirect blocks
			if (mip->INODE.i_block[13] == 0) // has allocated indirect
			{
				iblk = balloc(dev);
				mip->INODE.i_block[13] = iblk;
			}
			memset((char *)ibuff, 0, BLKSIZE);
			get_block(mip->dev, mip->INODE.i_block[13], (char *)ibuff);
			iblk = ibuff[lbk - (256 + 12)] / 256;
			dblk = ibuff[lbk - (256 + 12)] % 256;
			if (ibuff[iblk] == 0)
			{
				blk = balloc(dev);
				ibuff[iblk] = blk;
			}
			memset((char *)dibuff, 0, BLKSIZE);
			get_block(mip->dev, ibuff[iblk], (char *)dibuff);
			blk = dibuff[dblk];
			if (blk == 0)
			{
				blk = balloc(dev);
				dibuff[dblk] = blk;
				put_block(dev, ibuff[iblk], (char*)dibuff);
			}
			put_block(dev, mip->INODE.i_block[13], (char*)ibuff);
		}
	
		/* all cases come to here : write to the data block */
		get_block(mip->dev, blk, buf);   // read disk block into wbuf[ ]  
		char *cp = buf + startByte;      // cp points at startByte in wbuf[]
		remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

		while (remain > 0)
		{               // write as much as remain allows  
			*cp++ = *cq++;              // cq points at buf[ ]
			nbytes--; remain--;         // dec counts
			oftp->offset++;             // advance offset
			if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
				mip->INODE.i_size++;    // inc file size (if offset > fileSize)
			if (nbytes <= 0) break;     // if already nbytes, break
		}
		put_block(mip->dev, blk, buf);   // write wbuf[ ] to disk
		// loop back to while to write more .... until nbytes are written
	}

	mip->dirty = 1;       // mark mip dirty for iput() 
	iput(mip);
	printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);
	return nbytes;
}

