#include "type.h"

int make_dir()
{
	printf("DOLE REMY\n");
	int ino, r, dev;
	MINODE *pip;
	char *parent, *child;
	if (pathname[0] == 0)
	{
		printf("mkdir : input pathname :");
		fgets(pathname, 256, stdin);
		pathname[strlen(pathname) - 1] = 0;
	}


	//check if it is absolute path to determine where the inode comes from
	if (pathname[0] == '/')
		dev = root->dev;
	else
		dev = running->cwd->dev;


	if (findparent(pathname))
	{
		parent = dirname(pathname);
		child = basename(pathname);
		ino = getino(&dev, parent);

		if (ino == 0)
			return -1;
		pip = iget(dev, ino);

	}
	else
	{
		pip = iget(running->cwd->dev, running->cwd->ino);
		child = (char *)malloc((strlen(pathname) + 1)*sizeof(char));
		strcpy(child, pathname);
	}


	// verify INODE is a DIR and child does not exists in the parent directory
	if ((pip->INODE.i_mode & 0040000) != 0040000)
	{
		printf("%s is not a directory.\n", parent);
		iput(pip);
		return -1;
	}

	if (search(pip, child))
	{
		printf("%s already exists.\n", child);
		iput(pip);
		return -1;
	}

	r = my_mkdir(pip, child);

	return r;
}

int findparent(char *pathn)
{
	int i = 0;
	while (i < strlen(pathn))
	{
		if (pathn[i] == '/')
			return 1;
		i++;
	}
	return 0;
}

int my_mkdir(MINODE *pip, char *name)
{
	unsigned long inumber, bnumber;
	int i = 0, datab;
	char buf[BLOCK_SIZE], buf2[BLOCK_SIZE];
	char *cp;
	int need_length, ideal_length, rec_length;
	DIR *dirp;
	MINODE *mip;

	// allocate an inode and a disk block for the new directory
	inumber = ialloc(pip->dev);
	bnumber = balloc(pip->dev);

	mip = iget(pip->dev, inumber);

	/* write the content to mip->INODE*/
	mip->INODE.i_mode = 0x41ED;
	mip->INODE.i_uid = running->uid;
	mip->INODE.i_gid = running->gid;
	mip->INODE.i_size = 1024;
	mip->INODE.i_links_count = 2;
	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
	mip->INODE.i_blocks = 2;    /* Blocks count in 512-byte blocks */
	mip->dirty = 1;

	for (i = 1; i < 15; i++)
		mip->INODE.i_block[i] = 0;
	mip->INODE.i_block[0] = bnumber;

	iput(mip);

	// write the . and .. entries into a buf[] of BLOCK_SIZE
	memset(buf, 0, BLOCK_SIZE);

	dp = (DIR *)buf;

	dp->inode = inumber;
	strncpy(dp->name, ".", 1);
	dp->name_len = 1;
	dp->rec_len = 12;

	cp = buf;
	cp += dp->rec_len;
	dp = (DIR *)cp;

	dp->inode = pip->ino;
	dp->name_len = 2;
	strncpy(dp->name, "..", 2);
	dp->rec_len = BLOCK_SIZE - 12;

	put_block(pip->dev, bnumber, buf);

	//Finally enter name into parent's directory, assume all direct data blocks
	i = 0;
	while (pip->INODE.i_block[i])
		i++;

	i--;

	get_block(pip->dev, pip->INODE.i_block[i], buf);
	dp = (DIR *)buf;
	cp = buf;
	rec_length = 0;

	// step to the last entry in a data block
	while (dp->rec_len + rec_length < BLOCK_SIZE)
	{
		rec_length += dp->rec_len;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}


	need_length = 4 * ((8 + strlen(name) + 3) / 4);
	ideal_length = 4 * ((8 + dp->name_len + 3) / 4);
	rec_length = dp->rec_len;

	// check if it can enter the new entry as the last entry
	if ((rec_length - ideal_length) >= need_length)
	{
		// trim the previous entry to its ideal_length
		dp->rec_len = ideal_length;
		cp += dp->rec_len;
		dp = (DIR *)cp;
		dp->rec_len = rec_length - ideal_length;
		dp->name_len = strlen(name);
		strncpy(dp->name, name, dp->name_len);
		dp->inode = inumber;

		// write the new block back to the disk
		put_block(pip->dev, pip->INODE.i_block[i], buf);

	}
	else {
		// otherwise allocate a new data block 
		i++;
		datab = balloc(pip->dev);
		pip->INODE.i_block[i] = datab;
		get_block(pip->dev, datab, buf2);

		// enter the new entry as the first entry 
		dirp = (DIR *)buf2;
		dirp->rec_len = BLOCK_SIZE;
		dirp->name_len = strlen(name);
		strncpy(dirp->name, name, dirp->name_len);
		dirp->inode = inumber;

		pip->INODE.i_size += BLOCK_SIZE;
		// write the new block back to the disk
		put_block(pip->dev, pip->INODE.i_block[i], buf2);

	}

	pip->INODE.i_links_count++;
	pip->INODE.i_atime = time(0L);
	pip->dirty = 1;

	iput(pip);

	return 0;
}