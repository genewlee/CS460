#include "type.h"
#include <time.h>


void ch_mod()
{
	int mode, ino;
	MINODE *mip;

	if (pathname[0] == 0)
	{
		printf("no pathname provided\n");
		return -1;
	}
	if (parameter[0] == 0)
	{
		printf("no permissions provided\n");
		return -1;
	}

	mode = strtol(parameter, (char **)NULL, 8);
	ino = getino(&dev, pathname);

	if (ino == 0)
	{
		return -1;
	}

	mip = iget(dev, ino);

	if ((mip->INODE.i_mode & 0100000) == 0100000)
		mip->INODE.i_mode = 0100000 + mode;
	else if ((mip->INODE.i_mode & 0040000) == 0040000)
		mip->INODE.i_mode = 0040000 + mode;
	else
		mip->INODE.i_mode = 0120000 + mode;

	mip->dirty = 1;

	iput(mip);
	return 0;
}

int ch_own()
{
	int own, ino;
	MINODE *mip;

	if (pathname[0] == 0)
	{
		printf("no pathname provided\n");
		return -1;
	}
	if (parameter[0] == 0)
	{
		printf("no new owner provided\n");
		return -1;
	}

	own = strtol(parameter, (char **)NULL, 10);
	ino = getino(&dev, pathname);

	if (ino == 0)
		return -1;

	mip = iget(dev, ino);

	mip->INODE.i_uid = own;
	mip->dirty = 1;

	iput(mip);
	return 0;
}

int mystat()
{
	MINODE *mip;
	int ino;

	dev = running->cwd->dev;

	if (pathname[0] == 0)
	{
		ino = running->cwd->ino;
	}
	else
	{
		ino = getino(&dev, pathname);
	}

	if (ino == 0)
	{
		return 0;
	}

	mip = iget(dev, ino);
	printf("\n\tSTAT\t\n");
	printf("dev = %d\n", mip->dev);
	printf("ino = %lu\n", mip->ino);
	printf("mode = %4x\n", mip->INODE.i_mode);
	printf("size = %d\n", mip->INODE.i_size);
	printf("uid = %d\n", mip->INODE.i_uid);
	printf("gid = %d\n", mip->INODE.i_gid);
	printf("blocks = %d\n", mip->INODE.i_blocks);
	printf("linkcount = %d\n", mip->INODE.i_links_count);
	printf("atime = %s", (char *)ctime(&(mip->INODE.i_atime)));
	printf("mtime = %s", (char *)ctime(&(mip->INODE.i_mtime)));
	printf("ctime = %s\n", (char *)ctime(&(mip->INODE.i_ctime)));
	iput(mip);
	return 1;
}

int make_dir()
{
	//printf("%s\n", "This is mkdir");
	char *parent, *child, copypath[1028];
	MINODE *pip;
	int pino;

	strcpy(copypath, pathname);
	
	/*1. pahtname = "/a/b/c" start mip = root;         dev = root->dev;
				  = "a/b/c" start mip = running->cwd; dev = running->cwd->dev;*/
	if (pathname[0] == 0)
	{
		printf("Pathname error\n");
		return 1;
	}

	if (pathname[0] == '/')
	{
		dev = root->dev;
	}
	else
	{
		dev = running->cwd->dev;
	}
	//2.
	parent = dirname(copypath);   //parent = "/a/b" OR "a/b"
	strcpy(copypath, pathname);
	child = basename(copypath);  //child = "c"
	
	//3. Get the In_MEMORY minode of parent :
	pino = getino(&dev, parent);
	pip = iget(dev, pino);
	
	//Verify: (1).parent INODE is a DIR(HOW ? )
	if ((pip->INODE.i_mode & 0xF000) != 0x4000)
	{
		printf("parent not a dir\n");
		iput(pip);
		return -1;
	}
	//(2).child does NOT exists in the parent directory(HOW ? )
	
	if (getino(&dev, pathname) != 0)
	{
		printf("this dir already exists\n");
		iput(pip);
		return -1;
	}

	//4. call 
	mymkdir(pip, child);
	
	/*5. inc parent inodes's link count by 1; 
			touch its atime and mark it DIRTY*/
	pip->INODE.i_links_count++;
	pip->INODE.i_atime = time(0L);
	pip->dirty = 1;

	//6. 
	iput(pip);

	return 0;
}

int mymkdir(MINODE *pip, char *name)
{
	int ino, bno, i, ino2;
	MINODE *mip;
	char buff[1024];
	//1. pip points at the parent minode[] of "/a/b", name is a string "c")
	
	//2. allocate an inode and a disk block for the new directory;
	ino = ialloc(dev);
	if (ino == 2) // dont overwrite root
	{
		ino = ialloc(dev);
	}
	bno = balloc(dev);
	//ino2 = ialloc(dev);
	//printf("ino=%d bno=%d ino2=%d\n", ino, dev, ino2);
	
	/*3. mip = iget(dev, ino) to load the inode into a minode[](in order to
		wirte contents to the INODE in memory).*/
	//4. Write contents to mip->INODE to make it as a DIR.
	//5. iput(mip); which should write the new INODE out to disk.

	// C CODE of (3), (4) and (5):
	//**********************************************************************/
	mip = iget(dev, ino);
	//INODE *ip = &mip->INODE;
	
	/*Use ip->to acess the INODE fields :*/

	mip->INODE.i_mode = 0x41ED;		// OR 040755: DIR type and permissions
	mip->INODE.i_uid = running->uid;	// Owner uid 
	mip->INODE.i_gid = running->gid;	// Group Id
	mip->INODE.i_size = BLKSIZE;		// Size in bytes 
	mip->INODE.i_links_count = 2;	        // Links count=2 because of . and ..
	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);  // set to current time
	mip->INODE.i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
	mip->INODE.i_block[0] = bno;             // new DIR has one data block 
	for (i = 1; i < 14; i++)
	{
		mip->INODE.i_block[i] = 0;
	}

	mip->dirty = 1;               // mark minode dirty
	iput(mip);                    // write INODE to disk
	
	//***** create data block for new DIR containing . and .. entries *******/
	/*6. Write . and ..entries into a buf[] of BLKSIZE

		| entry . | entry .. |
		----------------------------------------------------------------------
		| ino | 12 | 1 | . | pino | 1012 | 2 | .. |
		----------------------------------------------------------------------*/
	//memset(buf, 0, 1024);
	
	for (i = 0; i < 1024; i++)
	{
		buff[i] = 0;
	}

	dp = (DIR *)buff;
	dp->inode = ino;
	strncpy(dp->name, ".", 1);
	dp->name_len = 1;
	dp->rec_len = 12;

	cp = buff + 12;
	dp = (DIR *)cp;
	dp->inode = pip->ino;
	strncpy(dp->name, "..", 2);
	dp->name_len = 2;
	dp->rec_len = BLKSIZE - 12;

	//Then, write buf[] to the disk block bno;
	put_block(dev, bno, buff);

	//7. Finally, enter name ENTRY into parent's directory by 
	enter_name(pip, ino, name);
	return 0;
}

int enter_name(MINODE *pip, int myino, char *myname)
{
	int i, ilen, nlen, rlen = 0, bl;
	char buff[1024];

	//For each data block of parent DIR do { // assume: only 12 direct blocks
	for (i = 0; i < 12; i++)
	{
		if (pip->INODE.i_block[i] == 0) break;
	}
	
	//(1).get parent's data block into a buf[];
	// step to LAST entry in block: int blk = parent->INODE.i_block[i];
	get_block(dev, pip->INODE.i_block[--i], buff);
	dp = (DIR *)buff;
	cp = buff;
	
	// need to write a new block entry, either at the and of an existing one (if room), or in a new block
	//(4).Step to the last entry in a data block(HOW ? ).
	//printf("step to LAST entry in data block %d\n", blk);
	while (dp->rec_len + rlen < BLOCK_SIZE)
	{
		rlen += dp->rec_len;
		cp += dp->rec_len;
		dp = (DIR *)cp; // dp NOW points at last entry in block
	}
	
	/*(2).EXT2 DIR entries : Each DIR entry has rec_len and name_len.Each entry's
	ideal length is*/
	/*(3).To enter a new entry of name with n_len, the needed length is
	need_length = 4 * [(8 + n_len + 3) / 4]  // a multiple of 4*/
	nlen = 4 * ((8 + strlen(myname) + 3) / 4);
	ilen = 4 * ((8 + dp->name_len + 3) / 4);
	//Let remain = LAST entry's rec_len - its IDEAL_LENGTH;
	rlen = dp->rec_len - ilen;
				
	/*All DIR entries in a data block have rec_len = IDEAL_LEN, except the last
	entry.The rec_len of the LAST entry is to the end of the block, which may
	be larger than its IDEAL_LEN.

	-- | -4-- - 2----2-- | ---- | -------- - | -------- - rlen ->------------------------ |
	| ino rlen nlen NAME | ......... | ino rlen nlen | NAME |
	--------------------------------------------------------------------------*/

	if (rlen >= nlen) 
	{
		//printf("enought room\n");
		/*enter the new entry as the LAST entry and trim the previous entry
		to its IDEAL_LENGTH;
		goto (6) below.*/
		
		//(6).Write data block to disk;
		dp->rec_len = ilen;
		cp += dp->rec_len;
		dp = (DIR *)cp;
		dp->rec_len = rlen;
		dp->name_len = strlen(myname);
		strncpy(dp->name, myname, dp->name_len);
		dp->inode = myino;
		put_block(pip->dev, pip->INODE.i_block[i], buff);
		return 0;
	}
	
	/*(5).// Reach here means: NO space in existing data block(s)

	Allocate a new data block; INC parent's isze by 1024;
	Enter new entry as the first entry in the new data block with rec_len = BLKSIZE.

	| --------------------rlen = BLKSIZE------------------------------------ -
	| myino rlen nlen myname |
	--------------------------------------------------------------------------*/
	//printf("new block\n");
	bl = balloc(pip->dev); //bl is the block number
	printf("bl=%d i=%d\n", bl, i);
	pip->INODE.i_block[++i] = bl;
	printf("bl=%d i=%d\n", bl, i);
	get_block(pip->dev, bl, buff);

	dp = (DIR *)buff;
	dp->rec_len = BLOCK_SIZE;
	dp->name_len = strlen(myname);
	strncpy(dp->name, myname, dp->name_len);
	dp->inode = myino;
	pip->INODE.i_size += BLOCK_SIZE;

	put_block(pip->dev, pip->INODE.i_block[i], buff);
	return 0;
}

//given a dp->inode, gets the INODE in disk
INODE *findinode(int inumber)
{
	int blk, offset;
	INODE *ip;
	char buff[1024];

	//Mailmans algorithem.
	blk = (inumber - 1) / INODES_PER_BLOCK + gp->bg_inode_table;
	offset = (inumber - 1) % INODES_PER_BLOCK;

	get_block(dev, blk, buff);
	//ip = (INODE *)(&buff[offset * 128]);
	ip = (INODE *)buf + offset;
	if (ip == 0)
	{
		printf("Error reading inode block\n");
	}

	return ip;
}

int ls()
{
	//printf("%s\n", "This is ls");

	int ino, dev = running->cwd->dev, i, j;
	MINODE *mip, *pip;
	char buff[BLKSIZE], temp[256], ftime[128];
	DIR *dp;
	char *cp;

	if (pathname[0] != 0) {   // ls pathname:
		if (pathname[0] == '/')
			dev = root->dev;
		ino = getino(&dev, pathname);
		//printf("%d\n", running->cwd->ino);
	}
	else
	{
		ino = running->cwd->ino;
	}

	pip = iget(dev, ino);

	if ((pip->INODE.i_mode & 0xF000) != 0x4000)
	{
		printf("cannot ls non dir\n");
		return;
	}
	 //mip points at minode; 
	 //Each data block of mip->INODE contains DIR entries
	 //print the name strings of the DIR entries
	
	//ino = running->cwd->ino;
	//mip = iget(dev, ino);
	//BASICALLY OUR SEARCH FUNCTION
	for (i = 0; i < 12; i++)
	{
		//printf("iblock[%d]=%d\n", i, mip->INODE.i_block[i]);
		//ASSUME DIRs only has 12 direct blocks
		if (pip->INODE.i_block[i] == 0)
		{
			return 0;
		}
			
		//ASSUME INODE *ip -> INODE
		get_block(dev, pip->INODE.i_block[i], buff);     // read INODE's i_block[0]
		cp = buff;
		dp = (DIR *)buff;
		ino = dp->inode;
		//printf("test\n");
		while (cp < buff + BLKSIZE) // while we havent traversed through all dirs
		{
			//INODE *ip = findinode(dp->inode);

			mip = iget(running->cwd->dev, ino);
			if ((mip->INODE.i_mode & 0xF000) == 0x8000)
				printf("%c", '-');
			if ((mip->INODE.i_mode & 0xF000) == 0x4000)
				printf("%c", 'd');
			if ((mip->INODE.i_mode & 0xF000) == 0xA000)
				printf("%c", 'l');

			for (j = 8; j >= 0; j--) {
				if (mip->INODE.i_mode & (1 << j))
					printf("%c", t1[j]);
				else
					printf("%c", t2[j]);
			}

			printf("%2d ", mip->INODE.i_links_count);
			printf("%2d ", mip->INODE.i_gid);
			printf("%2d ", mip->INODE.i_uid);

			//print time
			strcpy(ftime, ctime(&mip->INODE.i_mtime));
			ftime[strlen(ftime) - 1] = 0;
			printf("%s  ", ftime);

			printf("%4d ", mip->INODE.i_size);

			//print name
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			printf("%s", temp);

			printf("\n");

			//move to the next DIR entry:
			iput(mip);
			cp += dp->rec_len;   // advance cp by rec_len BYTEs
			dp = (DIR *)cp;     // pull dp along to the next record
			ino = dp->inode;
		}
	}
	
	if (pathname[0] != 0)
	{
		iput(pip);
	}

	return 0;
}

int pwd()
{
	rpwd(running->cwd);
	printf("\n");
	return 0;
}

void rpwd(MINODE *mip)
{
	if (mip == root)
	{
		printf("%s", root->name);
		return;
	}
	int p_ino, ino;
	char* name = (char*)malloc(sizeof(char) * 128);
	MINODE *parent;
	DIR *dp;

	//get_block(dev, mip->INODE.i_block[0], buf);
	// MOVE TO THE NEXT entry:
	findino(mip, &ino, &p_ino);
	parent = iget(dev, p_ino);
	rpwd(parent);

	findmyname(parent, mip->ino, name);
	printf("%s/", name);
	iput(parent);
}

int mycreat(MINODE *pip, char *name)
{
	int ino, bno, i;
	MINODE *mip;
	//1. pip points at the parent minode[] of "/a/b", name is a string "c")

	//2. allocate an inode for the new file;
	ino = ialloc(dev);
	if (ino == 2) // dont overwrite root
	{
		ino = ialloc(dev);
	}
	bno = balloc(dev);
	//ino2 = ialloc(dev);
	//printf("ino=%d bno=%d ino2=%d\n", ino, dev, ino2);

	/*3. mip = iget(dev, ino) to load the inode into a minode[](in order to
	wirte contents to the INODE in memory).*/
	//4. Write contents to mip->INODE to make it as a DIR.
	//5. iput(mip); which should write the new INODE out to disk.

	// C CODE of (3), (4) and (5):
	//**********************************************************************/
	mip = iget(dev, ino);
	//INODE *ip = &mip->INODE;

	/*Use ip->to acess the INODE fields :*/

	mip->INODE.i_mode = 0x81A4;		// OR 040755: DIR type and permissions
	mip->INODE.i_uid = running->uid;	// Owner uid 
	mip->INODE.i_gid = running->gid;	// Group Id
	mip->INODE.i_size = 0;		// Size in bytes 
	mip->INODE.i_links_count = 1;	        // Links count=1 because of file
	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);  // set to current time
	mip->INODE.i_blocks = 2;
	mip->INODE.i_block[0] = bno;
	for (i = 1; i < 15; i++)
	{
		mip->INODE.i_block[i] = 0;
	}

	mip->dirty = 1;               // mark minode dirty
	iput(mip);                    // write INODE to disk

									 //***** create data block for new DIR containing . and .. entries *******/
									 /*6. Write . and ..entries into a buf[] of BLKSIZE

									 | entry . | entry .. |
									 ----------------------------------------------------------------------
									 | ino | 12 | 1 | . | pino | 1012 | 2 | .. |
									 ----------------------------------------------------------------------*/
									 //memset(buf, 0, 1024);

	//7. Finally, enter name ENTRY into parent's directory by 
	enter_name(pip, ino, name);
	return 0;
}

int creat_file()
{
	//printf("%s\n", "This is creat");

	char *parent, *child, copypath[1028];
	MINODE *pip;
	int pino;

	strcpy(copypath, pathname);

	/*1. pahtname = "/a/b/c" start mip = root;         dev = root->dev;
	= "a/b/c" start mip = running->cwd; dev = running->cwd->dev;*/
	if (pathname[0] == 0)
	{
		printf("Pathname error\n");
		return 1;
	}

	if (pathname[0] == '/')
	{
		dev = root->dev;
	}
	else
	{
		dev = running->cwd->dev;
	}
	//2.
	parent = dirname(copypath);   //parent = "/a/b" OR "a/b"
	strcpy(copypath, pathname);
	child = basename(copypath);  //child = "c"

	//3. Get the In_MEMORY minode of parenzt :
	pino = getino(&dev, parent);
	pip = iget(dev, pino);

	//Verify: (1).parent INODE is a DIR(HOW ? )
	if ((pip->INODE.i_mode & 0xF000) != 0x4000)
	{
		printf("parent not a dir\n");
		iput(pip);
		return -1;
	}
	//(2).child does NOT exists in the parent directory(HOW ? )

	if (getino(&dev, pathname) != 0)
	{
		printf("this file already exists\n");
		iput(pip);
		return -1;
	}

	//4. call 
	mycreat(pip, child);

	/*5.
	touch its atime and mark it DIRTY*/
	pip->INODE.i_atime = time(0L);
	pip->dirty = 1;

	//6. 
	iput(pip);

	return 0;
}

int link()
{
	int i, pino, ino;
	MINODE * pip, *mip;
	
	char oldFileName[1024], newFileName[1024];
	strcpy(oldFileName, pathname);
	strcpy(newFileName, parameter);
	
	if ((oldFileName[0] == '\0') || (newFileName[0] == '\0')) 
	{
		printf("CMD: link [src] [dest]\n");
		return -1;
	}

	if (newFileName[0] != '/') 
	{
		pino = running->cwd->ino; 
	}
	else 
	{
		pino = getino(&dev, newFileName);
	}
	
	ino = getino(&dev, oldFileName);
	
	if ((ino == 0) || (pino == 0)) 
	{
		return -1;
	}

	//parent inode
	pip = iget(dev, pino);
	
	/*(1). get the INODE of /a/b/c into memory: mip->minode[ ]
     INODE of /a/b/c
              dev,ino.......*/
	mip = iget(dev, ino);
	
	//(2). check /a/b/c is a REG or LNK file (link to DIR is NOT allowed).
	if ((mip->INODE.i_mode & 0xF000) != 0x8000) 
	{
		iput(pip);
		iput(mip);
		printf("Not REG file\n");
		return -1;
	}
	
	enter_name(pip, ino, newFileName);
	
	pip->dirty = 1;
	pip->refCount++;
	pip->INODE.i_atime = time(0);
	iput(pip);
	//(5). increment the i_links_count of INODE by 1
	mip->INODE.i_links_count++;
	//(6).write INODE back to disk
	iput(mip);
	return 0;
}

int symlink()
{
	//(1).verify oldNAME exists(either a DIR or a REG file)

	int ino;
	MINODE *mip;
	char temp[128], oldFileName[1024], newFileName[1024];
	
	strcpy(oldFileName, pathname);
	strcpy(newFileName, parameter);

	if (pathname[0] == 0 || parameter[0] == 0)
	{
		printf("Pathname error\n");
		return 1;
	}

	if (pathname[0] == '/')
	{
		dev = root->dev;
	}
	else
	{
		dev = running->cwd->dev;
	}

	ino = getino(&dev, pathname);

	if (ino == 0)
	{
		printf("Pathname doesn't exist");
		return 1;
	}

	strcpy(temp, pathname);
	strcpy(pathname, parameter);
	//(2).creat a FILE / x / y / z
	if (creat_file())
	{
		// Failed so return
		printf("Error creating file");
		return 1;
	}

	//(3).change / x / y / z's type to LNK (0120000)=(1010.....)=0xA...
	ino = getino(&dev, parameter);
	mip = iget(dev, ino);

	mip->INODE.i_mode = 0xA1FF;
	//(4).write the string oldNAME into the i_block[], which has room for 60 chars.
	strcpy((char *)(mip->INODE.i_block), temp);
	mip->INODE.i_size = strlen(temp);
	mip->dirty = 1;
	iput(mip);

	return 0;
}

int readlink()
{
	int ino;
	MINODE *mip;
	char buff[1024];

	//(1).get INODE of pathname into a minode[].
	if (pathname[0] == 0)
	{
		printf("Pathname error\n");
		return 1;
	}

	if (pathname[0] == '/')
	{
		dev = root->dev;
	}
	else
	{
		dev = running->cwd->dev;
	}

	ino = getino(&dev, pathname);

	if (ino == 0)
	{
		printf("Pathname doesn't exist\n");
		return 1;
	}

	mip = iget(dev, ino);
	//(2).check INODE is a symbolic Link file.
	if ((mip->INODE.i_mode & 0xA000) != 0xA000)
	{
		printf("File is not LNK file\n");
		iput(mip);
		return 1;
	}
	//(3). return its string contents in INODE.i_block[].
	strncpy(buff, mip->INODE.i_block, mip->INODE.i_size);
	buff[mip->INODE.i_size] = '\0';
	printf("Link name: %s\n", buff);
	iput(mip);
	return 0;
}

int unlink()
{
	unlink_h(0);
}

int unlink_h(int mode)
{
	//(1).get pathname's INODE into memory
	int ino;
	MINODE *mip;
	char temp[128];

	if (pathname[0] == 0)
	{
		printf("Pathname error\n");
		return 1;
	}

	if (pathname[0] == '/')
	{
		dev = root->dev;
	}
	else
	{
		dev = running->cwd->dev;
	}

	ino = getino(&dev, pathname);

	if (ino == 0)
	{
		printf("Pathname doesn't exist");
		return 1;
	}

	mip = iget(dev, ino);

	//(2).verify it's a FILE (REG or LNK), can not be a DIR; 
	if ((mip->INODE.i_mode & 0x4000) == 0x4000)
	{
		printf("Invalid file\n");
		iput(mip);
		return 1;
	}

	//(3).decrement INODE's i_links_count by 1;
	mip->INODE.i_links_count--;
	//(4). if i_links_count == 0 == > rm pathname
	if (mip->INODE.i_links_count == 0 || mode == 1)
	{
		truncate(mip);
	}
	
	iput(mip);
	//(5). Remove childName=basename(pathname) from the parent directory
	strcpy(temp, pathname);
	ino = getino(&dev, dirname(temp));
	mip = iget(dev, ino);
	strcpy(temp, pathname);
	rm_child(mip, basename(temp));

	iput(mip);
	return 0;
}

int touch()
{
	int dev = running->cwd->dev;
	int ino = getino(&dev, pathname);
	MINODE *mip = iget(dev, ino);

	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	mip->dirty = 1;
	iput(mip);
	return 0;
}

int cd()
{
	int ino;
	MINODE *mip;
	int devv = dev;

	if (pathname[0] == 0)
	{
		printf("Pathname error\n");
		return 1;
	}

	if (pathname[0] == '/')
	{
		devv = root->dev;
	}
	else
	{
		devv = running->cwd->dev;
	}
	
	ino = getino(&devv, pathname);
	
	if (ino == 0)
	{
		printf("Pathname doesn't exist");
		return 1;
	}

	mip = iget(devv, ino);

	if ((mip->INODE.i_mode & 0x4000) != 0x4000)
	{
		printf("Not a dir\n");
		iput(mip);
		return 1;
	}

	running->cwd = mip;
	iput(mip);
	return 0;
}

//int rm_dir()
//{
//	int ino, i;
//	MINODE *mip;
//	char *temp, buff[1024];
//
//	if (pathname[0] == 0)
//	{
//		printf("Pathname error\n");
//		return 1;
//	}
//
//	if (pathname[0] == '/')
//	{
//		dev = root->dev;
//	}
//	else
//	{
//		dev = running->cwd->dev;
//	}
//
//	// 1. get inumber of pathname
//	ino = getino(&dev, pathname);
//
//	if (ino == 0)
//	{
//		printf("Pathname doesn't exist");
//		return 1;
//	}
//
//	// 2. get its minode[] pointer
//	mip = iget(dev, ino);
//
//	// 3. check ownership
//	// Do we really need to do this?...
//
//	// 4. check DIR type (HOW?) AND not BUSY (HOW?) AND is empty:
//	if ((mip->INODE.i_mode & 0x4000) != 0x4000)
//	{
//		printf("Not a dir\n");
//		iput(mip);
//		return 1;
//	}
//
//	if (mip->refCount>2)
//	{
//		printf("Dir is being used\n");
//		iput(mip);
//		return 1;
//	}
//
//	if (mip->INODE.i_links_count > 2)
//	{
//		printf("Dir is not empty\n");
//		iput(mip);
//		return 1;
//	}
//
//	if (mip->INODE.i_links_count == 2)
//	{
//		char name[256], *cp;
//		DIR *dirp;
//
//		for (i = 0; i < 12; i++)
//		{
//			if (mip->INODE.i_block[i])
//			{
//				get_block(mip->dev, mip->INODE.i_block[i], buff);
//				cp = buff;
//				dirp = (DIR *)buff;
//
//				while (cp < &buff[BLOCK_SIZE])
//				{
//					strncpy(name, dirp->name, dirp->name_len);
//					name[dirp->name_len] = 0;
//
//					// if stuff exists, this directory isn't empty
//					if (strcmp(name, ".") && strcmp(name, ".."))
//					{
//						iput(mip);
//						return 0;
//					}
//
//					cp += dirp->rec_len;
//					dirp = (DIR *)cp;
//				}
//			}
//		}
//	}
//
//	// 5. Deallocate its block and inode
//	for (i = 0; i<12; i++)
//	{
//		if (mip->INODE.i_block[i] == 0)
//		{
//			continue;
//		}
//		bdealloc(mip->dev, mip->INODE.i_block[i]);
//	}
//	idealloc(mip->dev, mip->ino);
//	iput(mip);
//
//	// 6. get parent DIR's ino and Minode
//	strcpy(temp, pathname);
//	ino = getino(&dev, dirname(temp));
//	mip = iget(dev, ino);
//	// 7. remove child's entry from parent directory by
//	strcpy(temp, pathname);
//	rm_child(mip, temp);
//	// 8. decrement pip's link_count by 1, etc..
//	mip->INODE.i_links_count--;
//	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
//	mip->dirty = 1;
//	iput(mip);
//	return 0;
//}

//int rm()
//{
//	unlink_h(1);
//}

//int rm_child(MINODE *pip, char *name)
//{
//	int i, j, tlen, nlen, rlen, plen = 0;
//	DIR *dirn;
//	char namebuf[256], temp[1024], *cn, buff[1024];
//
//	// 1. Search parent INODE's data block(s) for the entry of name
//	// for each block
//	for (i = 0; i < 12; i++)
//	{
//		// If node has contents 
//		if (pip->INODE.i_block[i] != 0)
//		{
//			// prep for search
//			get_block(pip->dev, pip->INODE.i_block[i], buff);
//			dp = (DIR *)buff;
//			cp = buff;
//			tlen = 0;
//			for (j = 0; cp < &buff[1024]; j++)
//			{
//				// get name
//				strncpy(namebuf, dp->name, dp->name_len);
//				namebuf[dp->name_len] = '\0';
//				tlen += dp->rec_len;
//				// if name found and not first block entry (have to move other blocks over...)
//				if (strcmp(namebuf, name) == 0 && j != 0)
//				{
//					// if name is the last entry in the data block, easy enough...
//					if (tlen == BLOCK_SIZE)
//					{
//						rlen = dp->rec_len;
//						cp -= plen;
//						dp = (DIR *)cp;
//						dp->rec_len += rlen;
//						put_block(pip->dev, pip->INODE.i_block[i], buff);
//						pip->dirty = 1;
//						return 0;
//					}
//
//					// if not, have to move everything left
//					rlen = dp->rec_len;
//					cn = cp + dp->rec_len;
//					dirn = (DIR *)cn;
//
//					// keep moving until done
//					while (tlen + dirn->rec_len < BLOCK_SIZE)
//					{
//						tlen += dirn->rec_len;
//						nlen = dirn->rec_len;
//						dp->inode = dirn->inode;
//						dp->rec_len = dirn->rec_len;
//						dp->name_len = dirn->name_len;
//						strncpy(dp->name, dirn->name, dirn->name_len);
//						cn += nlen;
//						dirn = (DIR *)cn;
//						cp += nlen;
//						dp = (DIR *)cp;
//					}
//
//					dp->inode = dirn->inode;
//					dp->rec_len = dirn->rec_len + rlen;
//					dp->name_len = dirn->name_len;
//					strncpy(dp->name, dirn->name, dirn->name_len);
//					put_block(pip->dev, pip->INODE.i_block[i], buff);
//					pip->dirty = 1;
//					return 0;
//				}
//				// name found and is first entry (a lot easier...)
//				else if (strcmp(namebuf, name) == 0)
//				{
//					bdealloc(pip->dev, pip->INODE.i_block[i]);
//					memset(temp, 0, BLOCK_SIZE);
//					put_block(pip->dev, pip->INODE.i_block[i], temp);
//					pip->INODE.i_size -= BLOCK_SIZE;
//					pip->INODE.i_block[i] = 0;
//					pip->dirty = 1;
//					return 0;
//				}
//				plen = dp->rec_len;
//				cp += dp->rec_len;
//				dp = (DIR *)cp;
//			}
//		}
//	}
//}

int rm_dir()
{
	int ino, i;
	MINODE *mip;
	char parent, child;
	char *temp, buff[1024];
	MINODE *pip;

	if (pathname[0] == 0)
	{
		printf("Pathname error\n");
		return 1;
	}

	if (pathname[0] == '/')
	{
		dev = root->dev;
	}
	else
	{
		dev = running->cwd->dev;
	}

	// 1. get inumber of pathname
	ino = getino(&dev, pathname);

	if (ino == 0)
	{
		printf("Pathname doesn't exist");
		return 1;
	}

	// 2. get its minode[] pointer
	mip = iget(dev, ino);

	// 3. check ownership
	// Do we really need to do this?...

	// 4. check DIR type (HOW?) AND not BUSY (HOW?) AND is empty:
	if ((mip->INODE.i_mode & 0x4000) != 0x4000)
	{
		printf("Not a dir\n");
		iput(mip);
		return 1;
	}

	if (mip->refCount>1)
	{
		printf("Dir is being used\n");
		iput(mip);
		return 1;
	}

	if (mip->INODE.i_links_count > 2)
	{
		printf("Dir is not empty\n");
		iput(mip);
		return 1;
	}

	if (mip->INODE.i_links_count == 2)
	{
		char name[256], *cp;

		DIR *dirp;


		for (i = 0; i < 12; i++)
		{
			if (mip->INODE.i_block[i])
			{
				//bdealloc(mip->dev, mip->INODE.i_block[i]);
				get_block(mip->dev, mip->INODE.i_block[i], buff);
				cp = buff;
				dirp = (DIR *)buff;

				while (cp < &buff[BLOCK_SIZE])
				{
					strncpy(name, dirp->name, dirp->name_len);
					name[dirp->name_len] = 0;

					// if stuff exists, this directory isn't empty
					if (strcmp(name, ".") && strcmp(name, ".."))
					{
						iput(mip);
						return 0;
					}

					cp += dirp->rec_len;
					dirp = (DIR *)cp;
				}
			}

		}
	}

	// 5. Deallocate its block and inode
	for (i = 0; i<12; i++)
	{
		if (mip->INODE.i_block[i] == 0)
		{
			continue;
		}
		bdealloc(mip->dev, mip->INODE.i_block[i]);
	}
	idealloc(mip->dev, mip->ino);
	iput(mip);
	parent = dirname(pathname);
	child = basename(pathname);
	// 6. get parent DIR's ino and Minode
	//strcpy(temp, pathname);
	ino = getino(&dev, parent);
	pip = iget(dev, ino);
	// 7. remove child's entry from parent directory by
	//strcpy(temp, pathname);
	rm_child(pip, child);
	// 8. decrement pip's link_count by 1, etc..
	pip->INODE.i_links_count--;
	pip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	pip->dirty = 1;
	touch(parent);
	iput(pip);
	return 0;
}

int rm()
{
	unlink_h(1);
}

int rm_child(MINODE* pip, char *name)
{
	int i, j, tlen, nlen, rlen, plen = 0;
	DIR *dirn;
	char namebuf[256], temp[1024], *cn, buff[1024];

	// 1. Search parent INODE's data block(s) for the entry of name
	// for each block
	for (i = 0; i < 12; i++)
	{
		// If node has contents 
		if (pip->INODE.i_block[i] != 0)
		{
			// prep for search
			get_block(pip->dev, pip->INODE.i_block[i], buff);
			dp = (DIR *)buff;
			cp = buff;
			tlen = 0;
			for (j = 0; cp < &buff[1024]; j++)
			{
				// get name
				strcpy(namebuf, dp->name);
				printf("NAME IN NAMEBUF RM_CHILD: %s NAME= %s\n", namebuf, name);
				//namebuf[dp->name_len] = '\0';
				tlen += dp->rec_len;
				// if name found and not first block entry (have to move other blocks over...)
				if (strcmp(namebuf, name) == 0 && j != 0)
				{
					// if name is the last entry in the data block, easy enough...
					if (tlen == BLOCK_SIZE)
					{
						rlen = dp->rec_len;
						cp -= plen;
						dp = (DIR *)cp;
						dp->rec_len += rlen;
						put_block(pip->dev, pip->INODE.i_block[i], buff);
						pip->dirty = 1;
						return 0;
					}

					// if not, have to move everything left
					rlen = dp->rec_len;
					cn = cp + dp->rec_len;
					dirn = (DIR *)cn;

					// keep moving until done
					while (tlen + dirn->rec_len < BLOCK_SIZE)
					{
						tlen += dirn->rec_len;
						nlen = dirn->rec_len;
						dp->inode = dirn->inode;
						dp->rec_len = dirn->rec_len;
						dp->name_len = dirn->name_len;
						strncpy(dp->name, dirn->name, dirn->name_len);
						cn += nlen;
						dirn = (DIR *)cn;
						cp += nlen;
						dp = (DIR *)cp;
					}

					dp->inode = dirn->inode;
					dp->rec_len = dirn->rec_len + rlen;
					dp->name_len = dirn->name_len;
					strncpy(dp->name, dirn->name, dirn->name_len);
					put_block(pip->dev, pip->INODE.i_block[i], buff);
					pip->dirty = 1;
					return 0;
				}
				// name found and is first entry (a lot easier...)
				else if (strcmp(namebuf, name) == 0)
				{
					bdealloc(pip->dev, pip->INODE.i_block[i]);
					memset(temp, 0, BLOCK_SIZE);
					put_block(pip->dev, pip->INODE.i_block[i], temp);
					pip->INODE.i_size -= BLOCK_SIZE;
					pip->INODE.i_block[i] = 0;
					pip->dirty = 1;
					return 0;
				}
				plen = dp->rec_len;
				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
		}
	}
}


