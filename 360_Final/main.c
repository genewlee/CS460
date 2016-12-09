#include "type.h"

char *rootdev = "disk", *slash = "/", *dot = ".";

int DEBUG = 0;
int nproc = 0;

///************************ globals *****************************/
//MINODE *root;
//char pathname[128], parameter[128], *name[128], cwdname[128];
//char names[128][256], buf[1024];
//
//int  nnames;
//
//int iblock;
//
//MINODE minode[NMINODES];
//MOUNT  mounttab[NMOUNT];
//PROC   proc[NPROC], *running;
//OFT    oft[NOFT];
//
//MOUNT *getmountp();
//
//char *cp;
//
///************************ end globals *****************************/

// include ALL YOUR .c files here
#include "level1.c"  
#include "level2.c"

static void get_inode(int fd, int ino, INODE *inode)
{
	lseek(fd, BLOCK_OFFSET(inode_table) + (ino - 1) * sizeof(INODE), SEEK_SET);
	read(fd, inode, sizeof(INODE));
}

int get_block(int fd, int blk, char buf[])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[])
{
	lseek(fd, (long)blk*BLKSIZE, 0);
	write(fd, buf, BLKSIZE);
}

//MINODE *iget(int devv, int ino)
//{
//	MINODE *mip;
//	INODE *ip;
//	int i, blk, offset;
//	char buff[1024];
//
//	//(1). Search minode[i] for an entry whose refCount > 0 with the SAME (dev,ino)
//	//if found: refCount++; mip-return &minode[i];
//	for (i = 0; i < NMINODES; i++)
//	{
//		mip = &minode[i];
//
//		if (mip->refCount > 0)
//		{
//			if (mip->ino == ino && mip->dev == dev)
//			{
//				mip->refCount++;
//				return mip;
//			}
//		}
//	}
//
//	/* Must be in disk because not found in minode array */
//
//	//Mailmans algorithem.
//	blk = (ino - 1) / INODES_PER_BLOCK + gp->bg_inode_table;
//	offset = (ino - 1) % INODES_PER_BLOCK;
//
//	//read blk into buf[ ]
//	get_block(devv, blk, buff);
//	ip = (INODE *)buff + offset;
//	//ip = (INODE *)(&buf[offset * 128]);
//
//	//Find a minode[i] whose refCount = 0 => let MINODE *mip = &minode[i]
//	for (i = 0; i < NMINODES; i++)
//	{
//		if (minode[i].ino == 0)
//		{
//			if (minode[i].refCount == 0)
//			{
//				//copy *ip into mip->INODE
//				memcpy(&minode[i].INODE, ip, sizeof(INODE));
//				mip = &minode[i];
//				//mip->INODE = *ip;
//				//initialize other fields of *mip:
//				mip->dev = devv;
//				mip->ino = ino;
//				mip->dirty = 0;
//				mip->refCount = 1;
//				mip->mounted = 0;
//				mip->mountptr = NULL;
//				return mip;
//			}
//		}
//	}
//}

MINODE *iget(int devv, int ino)
{
	MINODE *mip;
	INODE *ip;
	int i, blk, offset;
	char buff[1024];

	//printf("I GET INO is: %d\n", ino);
	//(1). Search minode[i] for an entry whose refCount > 0 with the SAME (dev,ino)
	//if found: refCount++; mip-return &minode[i];
	for (i = 0; i < NMINODES; i++)
	{
		mip = &(minode[i]);

		if (mip->refCount > 0 && mip->ino == ino)
		{
			mip->refCount++;
			return mip;
		}
	}

	/// Must be in disk because not found in minode array /
	get_block(dev, GDBLOCK, buff);
	gp = (GD*)buff;
	//inode_table = gp->bg_inode_table;

	//Mailmans algorithem.
	//printf("GD INODE TABLE is: %d\n", gp->bg_inode_table);//gp->bg_free_inodes_count);
	
	blk = (ino - 1) / INODES_PER_BLOCK + gp->bg_inode_table;
	offset = (ino - 1) % INODES_PER_BLOCK;

	//read blk into buf[ ]
	get_block(devv, blk, buff);
	ip = (INODE *)buff + offset;
	//ip = (INODE )(&buf[offset  128]);

	//Find a minode[i] whose refCount = 0 => let MINODE *mip = &minode[i]
	for (i = 0; i < NMINODES; i++)
	{
		if (minode[i].ino == 0)
		{
			if (minode[i].refCount == 0)
			{
				//copy *ip into mip->INODE
				memcpy(&minode[i].INODE, ip, sizeof(INODE));
				mip = &minode[i];
				//mip->INODE = *ip;
				//initialize other fields of *mip:
				mip->dev = devv;
				mip->ino = ino;
				mip->dirty = 0;
				mip->refCount = 1;
				mip->mounted = 0;
				mip->mountptr = NULL;
				//printf("INODE is: %d\n", mip->ino);
				return mip;
			}
		}
	}
}

int iput(MINODE *mip)
{
	char putbuf[BLKSIZE];
	int blk, inode, offset;
	//dec refCount by 1. 
	mip->refCount--;

	//if (after dec) refCount > 0 ==> return;
	if (mip->refCount > 0)
	{
		return;
	}

	//if (Minode[].dirty == 0) ==> no need to write back, so return;
	if (mip->dirty == 0)
	{
		return;
	}

	//must write the INODE back to disk
	//if (mip->refCount == 0 && mip->dirty == 1)
	//{
		/*To write an INODE back to disk:
		 Use minode's (dev, ino) to determine which INODE on disk,i.e. Use Mailman's
		algorithm to determine the disk block and inode's offset in that block.*/
		get_block(dev, GDBLOCK, putbuf);
		gp = (GD*)putbuf;
		blk = (mip->ino - 1) / INODES_PER_BLOCK + gp->bg_inode_table;
		offset = (mip->ino - 1) % INODES_PER_BLOCK;

		//let INODE *ip point at the INODE in buf[].
		ip = ((INODE *)putbuf) + offset;

		//Read that block into a buf[], 
		get_block(dev, blk, putbuf);

		//Copy mip->INODE into *ip in buf[ ];
		//memcpy(ip, &mip->INODE, 128);
		*ip = mip->INODE;

		//Write the block (in buf[ ]) back to disk.
		put_block(dev, blk, putbuf);
	//}
}


void ParsePath(char * pathnames)
{
	nnames = 0;
	char *temp, copy[128];
	strcpy(copy, pathnames);
	temp = strtok(copy, "/");
	while (temp != NULL)
	{
		name[nnames] = temp;
		nnames++;
		temp = strtok(0, "/");
	}
	//int i;
	//for (i = 0; i < nnames; i++)
	//{
	//	printf("PARSE: name[%d] = %s \n", i, name[i]);
	//}
}

int search(MINODE *mip, char *pname)
{
	// search for name string in the data blocks of this INODE
	// if found, return name's inumber
	// else      return 0S
	char sbuf[BLKSIZE], temp[256];
	int ino, i;

	DIR *dp;
	char *cp;

	//printf("SEARCH: pname= %s \n", pname);
	for (i = 0; i < 12; i++)
	{
		// ASSUME DIRs only has 12 direct blocks
		if (mip->INODE.i_block[i] == 0)
			return 0;

		// ASSUME INODE *ip -> INODE
		//printf("i_block[0] = %d\n", ip->i_block[0]); // print blk number
		get_block(dev, mip->INODE.i_block[i], sbuf);     // read INODE's i_block[0]
		cp = sbuf;
		dp = (DIR *)sbuf;

		while (cp < sbuf + BLKSIZE) // while we havent traversed through all dirs
		{
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			//printf("dp->ino = %d, dp->name=%s\n", dp->inode, temp);
			//printf("pname = %s, dp->name=%s\n", pname, temp);
			//printf("SEARCH: temp=%s pname=%s dplength=%d templen=%d\n", temp, pname, dp->name_len, strlen(temp));
			if (strcmp(pname, temp) == 0)
			{
				ino = dp->inode;
				//printf("SEARCH: dp->inode=%d \n", ino);
				return ino;
			}

			// move to the next DIR entry:
			cp += dp->rec_len;   // advance cp by rec_len BYTEs
			dp = (DIR *)cp;     // pull dp along to the next record
		}
	}
	return 0;
}

int getino(int *devv, char *pname)
{
	int i, inumber, disp, block;
	char *tempPath, temp[128];
	MINODE * mip;

	if (pathname[0] == '/')
	{
		devv = root->dev;
		inumber = root->ino;
	}
	else
	{
		devv = running->cwd->dev;
		inumber = running->cwd->ino;
		mip = running->cwd;
	}
	//printf("getino: pname= %s \n", pname);
	if (strcmp(pathname, "/") == 0)
	{
		return root->ino;
	}

	ParsePath(pname);

	if (strcmp(name[0], ".") == 0)
	{
		return running->cwd->ino;
	}
	

	for (i = 0; i < nnames; i++)
	{
		//printf("nnames=%d\n", nnames);
		//printf("getino: name[%d] = %s \n", i, name[i]);
		//printf("getino: pname= %s \n", i, pname);
		strcpy(temp, name[i]);
		//printf("getino: temp= %s \n", temp);
		inumber = search(mip, temp);
		//printf("INUMBER=%d\n", inumber);
		
		if (inumber == 0) //: can't find name[i], BOMB OUT!
		{
			printf("Unable to find %s\n", name[i]);
			//exit(1);
			iput(mip);
			return 0;
		}
		if ((mip->INODE.i_mode & 0xF000) != 0x4000)
		{
			printf("cannot ls non dir");
			iput(mip);
			return 0;
		}
		//get_inode(dev, inumber, &mip->INODE);
		iput(mip);
		mip = iget(devv, inumber);
	}
	
	// if you reach here, you must have ip --> the INODE of pathname.
	// Extract information from ip --> as required.
	return inumber;
}


int findmyname(MINODE *parent, int myino, char *myname)
{
	/*Given the parent DIR(MINODE pointer) and myino, this function finds
		the name string of myino in the parent's data block. This is the SAME
		as SEARCH() by myino, then copy its name string into myname[].*/
	char buff[BLKSIZE];
	int ino, i;

	DIR *dp;
	char *cp;

	for (i = 0; i < 12; i++)
	{
		// ASSUME DIRs only has 12 direct blocks
		if (parent->INODE.i_block[i] == 0)
			return 0;

		// ASSUME INODE *ip -> INODE
		//printf("i_block[0] = %d\n", ip->i_block[0]); // print blk number
		get_block(dev, parent->INODE.i_block[i], buff);     // read INODE's i_block[0]
		cp = buff;
		dp = (DIR *)buff;

		while (cp < buff + BLKSIZE) // while we havent traversed through all dirs
		{
			if (myino == dp->inode)
			{
				
				strncpy(myname, dp->name, dp->name_len);
				myname[dp->name_len] = 0;
				return;
			}

			// move to the next DIR entry:
			cp += dp->rec_len;   // advance cp by rec_len BYTEs
			dp = (DIR *)cp;     // pull dp along to the next record
		}
	}
	return 0;

}

int findino(MINODE *mip, int *myino, int *parentino)
{
	/*Given the parent DIR (MINODE pointer) and myino, this function finds
	the name string of myino in the parent's data block. This is the SAME
	as SEARCH() by myino, then copy its name string into myname[ ].*/
	int i;
	char buff[BLOCK_SIZE], inobuf[256], *cp;

	get_block(mip->dev, mip->INODE.i_block[0], buff);
	dp = (DIR *)buff; // '.'
	cp = buff;

	*myino = dp->inode;
	cp += dp->rec_len; // advance cp by rec_len BYTEs
	dp = (DIR *)cp; // '..'
	*parentino = dp->inode;

	return 0;

}

int tst_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	if (buf[i] & (1 << j))
		return 1;
	return 0;
}

int set_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8; j = bit % 8;
	buf[i] &= ~(1 << j);
}

int incFreeBlocks(int dev)
{
	char buff[1024];
	//get_super(dev);
	sp = (SUPER*)buff;
	sp->s_free_blocks_count += 1;
	put_block(dev, SUPERBLOCK, buff);
	get_block(dev, GDBLOCK, buff);
	gp = (GD*)buff;
	gp->bg_free_blocks_count += 1;
	put_block(dev, GDBLOCK, buff);
	return 1;
}

int incFreeInodes(int dev)
{
	char buf[BLKSIZE];

	// dec free inodes count in SUPER and GD
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count++;
	put_block(dev, 1, buf);

	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_inodes_count++;
	put_block(dev, 2, buf);
}

void decFreeBlocks(dev)
{
	char buff[1024];
	// We need to decrement the total free inodes in both super block and group block
	SUPER * sp;
	GD * gd;

	// SUPER BLOCK
	get_block(dev, SUPERBLOCK, buff);
	sp = (SUPER *)&buff[0];

	// Decrement super block
	sp->s_free_blocks_count--;

	put_block(dev, SUPERBLOCK, buff);

	// GROUP BLOCK
	get_block(dev, GDBLOCK, buff);

	gd = (GD *)&buff[0];

	// Decrement group block
	gd->bg_free_blocks_count--;

	put_block(dev, GDBLOCK, buff);

	return;
}

int decFreeInodes(int dev)
{
	char buff[BLKSIZE];

	// dec free inodes count in SUPER and GD
	get_block(dev, 1, buff);
	sp = (SUPER *)buff;
	sp->s_free_inodes_count--;
	put_block(dev, 1, buff);

	get_block(dev, 2, buff);
	gp = (GD *)buff;
	gp->bg_free_inodes_count--;
	put_block(dev, 2, buff);
}

//int ialloc(int dev)
//{
//	int  i;
//	char buff[BLKSIZE];
//
//	// read inode_bitmap block
//	get_block(dev, gp->bg_inode_bitmap, buff);
//
//	for (i = 0; i < NMINODES; i++) {
//		if (tst_bit(buff, i) == 0) {
//			set_bit(buff, i);
//			decFreeInodes(dev);
//
//			put_block(dev, gp->bg_inode_bitmap, buff); // IBITMAP -> gp->bg_inode_bitmap
//
//			return i + 1;
//		}
//	}
//	printf("ialloc(): no more free inodes\n");
//	return 0;
//}


//int balloc(int dev)
//{
//	int  i;
//	char buff[BLKSIZE];
//
//	// read inode_bitmap block
//	get_block(dev, BBITMAP, buff);
//
//	for (i = 0; i < NBLOCKS; i++) {
//		if (tst_bit(buff, i) == 0) {
//			set_bit(buff, i);
//			decFreeBlocks(dev);
//
//			put_block(dev, BBITMAP, buff);
//			
//			return i + 1;
//		}
//	}
//	printf("balloc(): no more free blocks\n");
//	return 0;
//}

int ialloc(int dev)
{
	int  i, imap;
	char buff[BLKSIZE];
	get_block(dev, GDBLOCK, buff);
	gp = (GD*)buff;

	imap = gp->bg_inode_bitmap;
	// read inode_bitmap block
	get_block(dev, imap, buff);

	for (i = 0; i < NMINODES; i++) {
		if (tst_bit(buff, i) == 0) {
			set_bit(buff, i);
			decFreeInodes(dev);

			put_block(dev, imap, buff); // IBITMAP -> gp->bg_inode_bitmap

			return i + 1;
		}
	}
	printf("ialloc(): no more free inodes\n");
	return 0;
}


int balloc(int dev)
{
	int  i, bmap;
	char buff[BLKSIZE];
	get_block(dev, GDBLOCK, buff);
	gp = (GD*)buff;
	bmap = gp->bg_block_bitmap;
	// read inode_bitmap block
	get_block(dev, bmap, buff);

	for (i = 0; i < NBLOCKS; i++) {
		if (tst_bit(buff, i) == 0) {
			set_bit(buff, i);
			decFreeBlocks(dev);

			put_block(dev, bmap, buff);

			return i + 1;
		}
	}
	printf("balloc(): no more free blocks\n");
	return 0;
}

int idealloc(int dev, int ino) // which deallocates an inode number, ino
{
	get_block(dev, IBITMAP, buf);
	clr_bit(buf, ino - 1);
	put_block(dev, IBITMAP, buf);
	incFreeInodes(dev);
}

int bdealloc(int dev, int bno) // which deallocates an block number, bno
{
	get_block(dev, BBITMAP, buf);
	clr_bit(buf, bno - 1);
	put_block(dev, BBITMAP, buf);
	incFreeBlocks(dev);
}


mountroot()   /* mount root file system */
{
	int i, ino, fd;// , dev;
	MOUNT *mp;
	SUPER *sp;
	MINODE *ip;

	char line[64], buff[BLOCK_SIZE], *rootdev;
	int ninodes, nblocks, ifree, bfree;

	printf("enter rootdev name (RETURN for disk) : ");
	gets(line);

	rootdev = "disk";

	if (line[0] != 0)
		rootdev = line;

	dev = open(rootdev, O_RDWR);
	if (dev < 0) {
		printf("panic : can't open root device\n");
		exit(1);
	}

	/* get super block of rootdev */
	get_block(dev, 1, buff);
	sp = (SUPER *)buff;

	/* check magic number */
	printf("SUPER magic=0x%x  ", sp->s_magic);
	if (sp->s_magic != SUPER_MAGIC) {
		printf("super magic=%x : %s is not a valid Ext2 filesys\n",
			sp->s_magic, rootdev);
		exit(0);
	}

	mp = &mounttab[0];      /* use mounttab[0] */

	/* copy super block info to mounttab[0] */
	ninodes = mp->ninodes = sp->s_inodes_count;
	NBLOCKS = nblocks = mp->nblocks = sp->s_blocks_count;

	bfree = sp->s_free_blocks_count;
	ifree = sp->s_free_inodes_count;

	get_block(dev, 2, buff);
	gp = (GD *)buff;

	mp->dev = dev;
	mp->busy = BUSY;

	mp->bmap = gp->bg_block_bitmap;
	mp->imap = gp->bg_inode_bitmap;
	mp->iblock = gp->bg_inode_table;

	strcpy(mp->name, rootdev);
	strcpy(mp->mount_name, "/");

	printf("bmap=%d  ", gp->bg_block_bitmap);
	printf("imap=%d  ", gp->bg_inode_bitmap);
	printf("iblock=%d\n", gp->bg_inode_table);

	/***** call iget(), which inc the Minode's refCount ****/

	root = iget(dev, 2);          /* get root inode */
	mp->mounted_inode = root;
	root->mountptr = mp;
	strncpy(root->name, "/", 1);

	printf("mount : %s  mounted on / \n", rootdev);
	printf("nblocks=%d  bfree=%d   ninodes=%d  ifree=%d\n",
		nblocks, bfree, ninodes, ifree);

	return(0);
}

init()
{
	int i, j;
	PROC *p;

	for (i = 0; i < NMINODES; i++)
		minode[i].refCount = 0;

	for (i = 0; i < NMOUNT; i++)
		mounttab[i].busy = 0;

	for (i = 0; i < NPROC; i++) {
		proc[i].status = FREE;
		for (j = 0; j < NFD; j++)
			proc[i].fd[j] = 0;
		proc[i].next = &proc[i + 1];
	}

	for (i = 0; i < NOFT; i++)
		oft[i].refCount = 0;

	printf("mounting root\n");
	mountroot();
	printf("mounted root\n");

	printf("creating P0, P1\n");
	p = running = &proc[0];
	p->status = BUSY;
	p->uid = 0;
	p->pid = p->ppid = p->gid = 0;
	p->parent = p->sibling = p;
	p->child = 0;
	p->cwd = root;
	p->cwd->refCount++;

	p = &proc[1];
	p->next = &proc[0];
	p->status = BUSY;
	p->uid = 2;
	p->pid = 1;
	p->ppid = p->gid = 0;
	p->cwd = root;
	p->cwd->refCount++;

	nproc = 2;
}

int quit()
{
	// write YOUR quit function here
	int i = 0;
	
	for (i = 0; i < NMINODES; i++)
	{
		if (minode[i].ino != 0)
		{
			if (minode[i].ino < 100)
			{
				iput(&minode[i]);
			}
		}
			
	}
	exit(0);
}

void menu()
{
	printf(
		"\t************************MENU*************************\n"
		"\tmenu		 	displays menu\n"
		"\tmkdir [path] 		creates a directory\n"
		"\trmdir [path] 		removes an empty directory\n"
		"\tls [path] 		lists a directory\n"
		"\tcd [path] 		moves to directory\n"
		"\tpwd	 		displays cwd\n"
		"\tcreat [path] 		creates file\n"
		"\ttouch [path] 		changes mtime of a file\n"
		"\trm [path] 		removes file\n"
		"\tcat [path]    		prints file\n"
		"\topen [path] [mode] 	opens a file\n"
		"\tclose [fd] 		close an open file\n"
		"\tpfd			shows open files\n"
		"\tread [fd] [nbytes] 	read a file for nbytes\n"
		"\twrite [fd] [string]	write to file\n"
		"\tlseek [fd] [nbytes] 	set offset to nbytes of file\n"
		"\tcp [src] [dest] 	copy a file\n"
		"\tmv [src] [dest] 	move a file\n"
		"\tchmod [path] 		change file permissions\n"
		"\tchown [path] [uid] 	change file owner\n"
		"\tlink [src] [dest] 	hard links a file\n"
		"\tsymlink [src] [dest] 	symbolically links a file\n"
		"\tunlink [path] 		unlinks a file\n"
		"\treadlink [path]		displays file that links to\n"
		"\tstat [path] 		shows file stat\n"
		"\texit  			exit program\n"	
		"\t*****************************************************\n"
		);
}

int findCommand(command)
{
	char * cmds[27] = { "menu", "rm", "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "open", "chmod", "link", "unlink", "symlink", "readlink", "stat", "chown", "touch", "open", "close", "read", "write", "lseek", "cat", "cp", "mv", "pfd", "quit"}; // "mount", "umount", 
	int i = 0;

	for (; i < 27; i++)
	{
		//printf("%s\n", cmds[i]);
		if (strcmp(cmds[i], command) == 0)
		{
			return i;
		}
	}
	return -1;
}

main(int argc, char *argv[])
{
	int i, cmd;
	char line[128], cname[64];
	int(*fptr[])() = { (int(*)())menu, rm, make_dir, rm_dir, ls, cd, pwd, creat_file, open_file, ch_mod, link, unlink, symlink, readlink, mystat, ch_own, touch, open_file, close_file, read_file, write_file, l_seek, cat, cp_file, mv_file, pfd, quit};// , mount, umount };

	if (argc > 1) {
		if (strcmp(argv[1], "-d") == 0)
			DEBUG = 1;
	}

	init();

	while (1) {
		printf("P%d running: ", running->pid);

		/* zero out pathname, parameter */
		for (i = 0; i < 64; i++) {
			pathname[i] = parameter[i] = name[i] = 0;
		}
		/* these do the same; set the strings to 0 */
		memset(pathname, 0, 64);
		memset(parameter, 0, 64);
		memset(buf, 0, 1024);

		printf("input command : ");
		gets(line);
		if (line[0] == 0) continue;

		sscanf(line, "%s %s %64c", cname, pathname, parameter);

		//ParsePath(pathname);

		int cmd = findCommand(cname);
		if (cmd == -1)
		{
			printf("invalid command\n");
		}
		else
		{
			fptr[cmd]();
		}
	}
} /* end main */

// NOTE: you MUST use a function pointer table








