/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 * This file is part of the ELILO, the EFI Linux boot loader.
 *
 *  ELILO is bl_free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  ELILO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ELILO; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * Please check out the elilo.txt for complete documentation on how
 * to use this program.
 *
 * The ext2 code in this file is derived from aboot-0.7 (the Linux/Alpha
 * bootloader) and credits are attributed to:
 *
 * This file has been ported from the DEC 32-bit Linux version
 * by David Mosberger (davidm@cs.arizona.edu).
 */


#ifdef PATH_MAX
#error You must have included some Linux header file by error
#endif

#define PATH_MAX		4095
#define EXT2FS_PATH_MAXLEN	PATH_MAX

#include "ext2fs.h"
#include "ext2_private.h"

#define FS_NAME "ext2"

/* ext2 file size is __u32 */
#define EXT2_FILESIZE_MAX	(0x100000000ULL)

/* 
 * Number of simultaneous open files. This needs to be high because
 * directories are kept open while traversing long path names.
 */
#define MAX_OPEN_FILES		32 



typedef struct inode_table_entry {
	struct	ext2_inode	inode;
	int			inumber;
	int			bl_free;
	unsigned short		old_mode;

	uint32_t			pos;	/* current position in file ext2 uses __u32 !*/
} inode_entry_t;


typedef struct {
	struct ext2_super_block sb;
	struct ext2_group_desc *gds;
	struct ext2_inode *root_inode;
	int ngroups;
	int directlim;			/* Maximum direct blkno */
	int ind1lim;			/* Maximum single-indir blkno */
	int ind2lim;			/* Maximum double-indir blkno */
	int ptrs_per_blk;		/* ptrs/indirect block */
	char blkbuf[EXT2_MAX_BLOCK_SIZE];
	int cached_iblkno;
	char iblkbuf[EXT2_MAX_BLOCK_SIZE];
	int cached_diblkno;
	char diblkbuf[EXT2_MAX_BLOCK_SIZE];
	long blocksize;


	inode_entry_t inode_table[MAX_OPEN_FILES];

	/* fields added to fit the protocol construct */
} ext2fs_priv_state_t;


typedef union {
//	ext2fs_interface_t pub_intf;
	struct {
//		ext2fs_interface_t  pub_intf;
		ext2fs_priv_state_t priv_data;
	} ext2fs_priv;
} ext2fs_t;

static	ext2fs_t ext2fs_alloc;
static	ext2fs_t *this=NULL;

#define FS_PRIVATE(n)	(&(((ext2fs_t *)n)->ext2fs_priv.priv_data))

static int
read_bytes(unsigned int offset, void *addr, unsigned int size)
{
	int status;
	uint8_t *buffer;
	unsigned int count, buffer_size;
	uint64_t base;
	int ret  = -1;


	base        = offset / SECTOR_SIZE;
	count       = (size + SECTOR_SIZE-1) / SECTOR_SIZE;
	buffer_size = count *SECTOR_SIZE;

	DBG_PRT(("readblock(%d,%d) base=%d count=%d\n", offset, size, base, count));

	/*
	 * slow but avoid large buffer on the stack
	 */
	buffer = (uint8_t *)bl_malloc(buffer_size);
	if (buffer == NULL) {
		ERR_PRT(("cannot bl_mallocate ext2fs buffer size=%d\n", buffer_size));
		return ret;
	}
	

	DBG_PRT(("readblock(%d, %d, %x)\n",  base, buffer_size, buffer));

#if 0
	status = blkio->ReadBlocks(blkio, mediaid, base, buffer_size, buffer); 
	if (EFI_ERROR(status)) {
		ERR_PRT(("readblock(%d,%d)=%r", base, buffer_size, status));
		goto error;
	}
#else
	if (partition_read(buffer,base,count)!=count) {
		ret=-1;
		goto error;
	}
	////FIXME: XXX
#endif

	DBG_PRT(("readblock(%d,%d)->0\n", offset, buffer_size));

	memcpy(addr, buffer+(offset-(base*SECTOR_SIZE)), size);

	ret = 0;

error:
	bl_free(buffer);

	return ret;
}

/*
 * Read the specified inode from the disk and return it to the user.
 * Returns NULL if the inode can't be read...
 */
static struct ext2_inode *
ext2_iget(ext2fs_priv_state_t *e2fs, int ino)
{
	int i;
	struct ext2_inode *ip;
	struct inode_table_entry *itp = 0;
	int group;
	long offset;

	ip = 0;
	for (i = 0; i < MAX_OPEN_FILES; i++) {
		DBG_PRT(("ext2_iget: looping, entry %d inode %d bl_free %d",
		       i, e2fs->inode_table[i].inumber, e2fs->inode_table[i].bl_free));
		if (e2fs->inode_table[i].bl_free) {
			itp = &e2fs->inode_table[i];
			ip = &itp->inode;
			break;
		}
	}
	if (!ip) {
		ERR_PRT(("ext2_iget: no bl_free inodes"));
		return NULL;
	}


	group = (ino-1) / e2fs->sb.s_inodes_per_group;

	DBG_PRT((" itp-inode_table=%d bg_inode_table=%d  group=%d ino=%d\n", (unsigned int)(itp-e2fs->inode_table),
			(unsigned int)(e2fs->gds[group].bg_inode_table), (unsigned int)group, (unsigned int)ino));

	offset = ((long) e2fs->gds[group].bg_inode_table * e2fs->blocksize)
		+ (((ino - 1) % EXT2_INODES_PER_GROUP(&e2fs->sb)) * EXT2_INODE_SIZE(&e2fs->sb));
	
	DBG_PRT(("ext2_iget: reading %d bytes at offset %d"
	       " ((%d * %d) + ((%d) %% %d) * %d) "
	       "(inode %d -> table %d)", 
	       sizeof(struct ext2_inode), 
	       (unsigned int)offset,
	       (unsigned int)e2fs->gds[group].bg_inode_table, (unsigned int)e2fs->blocksize,
	       (unsigned int)(ino - 1), (unsigned int)EXT2_INODES_PER_GROUP(&e2fs->sb), EXT2_INODE_SIZE(&e2fs->sb),
	       (unsigned int)ino, (unsigned int) (itp - e2fs->inode_table)));

	if (read_bytes(offset, ip, sizeof(struct ext2_inode))) {
		ERR_PRT(("ext2_iget: read error"));
		return NULL;
	}
	
	DBG_PRT(("mode=%x uid=%d size=%d gid=%d links=%d flags=%d",
					(unsigned int)ip->i_mode,
					(unsigned int)ip->i_uid,
					(unsigned int)ip->i_size,
					(unsigned int)ip->i_gid,
					(unsigned int)ip->i_flags));

	itp->bl_free = 0;
	itp->inumber = ino;
	itp->old_mode = ip->i_mode;

	return ip;
}


/*
 * Release our hold on an inode.  Since this is a read-only application,
 * don't worry about putting back any changes...
 */
static void 
ext2_iput(ext2fs_priv_state_t *e2fs, struct ext2_inode *ip)
{
	struct inode_table_entry *itp;

	/* Find and bl_free the inode table slot we used... */
	itp = (struct inode_table_entry *)ip;

	DBG_PRT(("ext2_iput: inode %d table %d", itp->inumber, (int) (itp - e2fs->inode_table)));

	itp->inumber = 0;
	itp->bl_free = 1;
}


/*
 * Map a block offset into a file into an absolute block number.
 * (traverse the indirect blocks if necessary).  Note: Double-indirect
 * blocks allow us to map over 64Mb on a 1k file system.  Therefore, for
 * our purposes, we will NOT bother with triple indirect blocks.
 *
 * The "bl_mallocate" argument is set if we want to *bl_mallocate* a block
 * and we don't already have one bl_mallocated.
 */
static int 
ext2_blkno(ext2fs_priv_state_t *e2fs, struct ext2_inode *ip, int blkoff)
{
	unsigned int *lp;
	unsigned int *ilp;
	unsigned int *dlp;
	int blkno;
	int iblkno;
	int diblkno;
	long offset;

	ilp = (unsigned int *)e2fs->iblkbuf;
	dlp = (unsigned int *)e2fs->diblkbuf;
	lp = (unsigned int *)e2fs->blkbuf;

	/* If it's a direct block, it's easy! */
	if (blkoff <= e2fs->directlim) {
		return ip->i_block[blkoff];
	}

	/* Is it a single-indirect? */
	if (blkoff <= e2fs->ind1lim) {
		iblkno = ip->i_block[EXT2_IND_BLOCK];

		if (iblkno == 0) {
			return 0;
		}

		/* Read the indirect block */
		if (e2fs->cached_iblkno != iblkno) {
			offset = iblkno * e2fs->blocksize;
			if (read_bytes(offset, e2fs->iblkbuf, e2fs->blocksize)) {
				ERR_PRT(("ext2_blkno: error on iblk read"));
				return 0;
			}
			e2fs->cached_iblkno = iblkno;
		}

		blkno = ilp[blkoff-(e2fs->directlim+1)];
		return blkno;
	}

	/* Is it a double-indirect? */
	if (blkoff <= e2fs->ind2lim) {
		/* Find the double-indirect block */
		diblkno = ip->i_block[EXT2_DIND_BLOCK];

		if (diblkno == 0) {
			return 0;
		}

		/* Read in the double-indirect block */
		if (e2fs->cached_diblkno != diblkno) {
			offset = diblkno * e2fs->blocksize;
			if (read_bytes(offset,  e2fs->diblkbuf, e2fs->blocksize)) {
				ERR_PRT(("ext2_blkno: err reading dindr blk"));
				return 0;
			}
			e2fs->cached_diblkno = diblkno;
		}

		/* Find the single-indirect block pointer ... */
		iblkno = dlp[(blkoff - (e2fs->ind1lim+1)) / e2fs->ptrs_per_blk];

		if (iblkno == 0) {
			return 0;
		}

		/* Read the indirect block */
    
		if (e2fs->cached_iblkno != iblkno) {
			offset = iblkno * e2fs->blocksize;
			if (read_bytes(offset, e2fs->iblkbuf, e2fs->blocksize)) {
				ERR_PRT(("ext2_blkno: err on iblk read"));
				return 0;
			}
			e2fs->cached_iblkno = iblkno;
		}

		/* Find the block itself. */
		blkno = ilp[(blkoff-(e2fs->ind1lim+1)) % e2fs->ptrs_per_blk];
		return blkno;
	}

	if (blkoff > e2fs->ind2lim) {
		ERR_PRT(("ext2_blkno: block number too large: %d", blkoff));
		return 0;
	}
	return -1;
}


static int
ext2_breadi(ext2fs_priv_state_t *e2fs, struct ext2_inode *ip, long blkno, long nblks, char *buffer)
{ 
	long dev_blkno, ncontig, offset, nbytes, tot_bytes;

	tot_bytes = 0;
	if ((blkno+nblks)*e2fs->blocksize > ip->i_size)
		nblks = (ip->i_size + e2fs->blocksize) / e2fs->blocksize - blkno;

	while (nblks) {
		/*
		 * Contiguous reads are a lot faster, so we try to group
		 * as many blocks as possible:
		 */
		ncontig = 0; nbytes = 0;
		dev_blkno = ext2_blkno(e2fs,ip, blkno);
		do {
			++blkno; ++ncontig; --nblks;
			nbytes += e2fs->blocksize;
		} while (nblks &&
			 ext2_blkno(e2fs, ip, blkno) == dev_blkno + ncontig);

		if (dev_blkno == 0) {
			/* This is a "hole" */
			memset(buffer, 0, nbytes);
		} else {
			/* Read it for real */
			offset = dev_blkno*e2fs->blocksize;
			DBG_PRT(("ext2_bread: reading %d bytes at offset %d", nbytes, offset));

			if (read_bytes(offset, buffer, nbytes)) {
				ERR_PRT(("ext2_bread: read error"));
				return -1;
			}
		}
		buffer    += nbytes;
		tot_bytes += nbytes;
	}
	return tot_bytes;
}

static struct ext2_dir_entry_2 *
ext2_readdiri(ext2fs_priv_state_t *e2fs, struct ext2_inode *dir_inode, int rewind)
{
	struct ext2_dir_entry_2 *dp;
	static int diroffset = 0, blockoffset = 0;

	/* Reading a different directory, invalidate previous state */
	if (rewind) {
		diroffset = 0;
		blockoffset = 0;
		/* read first block */
		if (ext2_breadi(e2fs, dir_inode, 0, 1, e2fs->blkbuf) < 0)
			return NULL;
	}

	DBG_PRT(("ext2_readdiri: blkoffset %d diroffset %d len %d", blockoffset, diroffset, dir_inode->i_size));

	if (blockoffset >= e2fs->blocksize) {
		diroffset += e2fs->blocksize;
		if (diroffset >= dir_inode->i_size)
			return NULL;
		ERR_PRT(("ext2_readdiri: reading block at %d", diroffset));
		/* assume that this will read the whole block */
		if (ext2_breadi(e2fs, dir_inode, diroffset / e2fs->blocksize, 1, e2fs->blkbuf) < 0) return NULL;
		blockoffset = 0;
	}

	dp = (struct ext2_dir_entry_2 *) (e2fs->blkbuf + blockoffset);
	blockoffset += dp->rec_len;

#if 0
	Print("ext2_readdiri: returning %x = ");
	{ int i; for(i=0; i < dp->name_len; i++) Print("%c", (CHAR16)dp->name[i]); Print("\n");}
#endif
	return dp;
}

/*
 * the string 'name' is modified by this call as per the parsing that
 * is done in strtok_simple()
 */
static struct ext2_inode *
ext2_namei(ext2fs_priv_state_t *e2fs, char *name)
{
	char *component;
	struct ext2_inode *dir_inode;
	struct ext2_dir_entry_2 *dp;
	int next_ino;

	/* start at the root: */
	if (!e2fs->root_inode)
		e2fs->root_inode = ext2_iget(e2fs, EXT2_ROOT_INO);
	dir_inode = e2fs->root_inode;
	if (!dir_inode)
	  return NULL;

	component = strtok_simple(name, '/');
	while (component) {
		int component_length;
		int rewind = 0;
		/*
		 * Search for the specified component in the current
		 * directory inode.
		 */
		next_ino = -1;
		component_length = strlen(component);

		DBG_PRT(("ext2_namei: component = %a", component));

		/* rewind the first time through */
		while ((dp = ext2_readdiri(e2fs, dir_inode, !rewind++))) {
			if ((dp->name_len == component_length) &&
			    (strncmp(component, dp->name,
				     component_length) == 0))
			{
				/* Found it! */
				DBG_PRT(("ext2_namei: found entry %a", component));
				next_ino = dp->inode;
				break;
			}
			DBG_PRT(("ext2_namei: looping"));
		}
	
		DBG_PRT(("ext2_namei: next_ino = %d", next_ino));

		/*
		 * At this point, we're done with this directory whether
		 * we've succeeded or failed...
		 */
		if (dir_inode != e2fs->root_inode) ext2_iput(e2fs, dir_inode);

		/*
		 * If next_ino is negative, then we've failed (gone
		 * all the way through without finding anything)
		 */
		if (next_ino < 0) {
			return NULL;
		}

		/*
		 * Otherwise, we can get this inode and find the next
		 * component string...
		 */
		dir_inode = ext2_iget(e2fs, next_ino);
		if (!dir_inode)
		  return NULL;

		component = strtok_simple(NULL, '/');
	}

	/*
	 * If we get here, then we got through all the components.
	 * Whatever we got must match up with the last one.
	 */
	return dir_inode;
}


/*
 * Read block number "blkno" from the specified file.
 */
static int 
ext2_bread(ext2fs_priv_state_t *e2fs, int fd, long blkno, long nblks, char *buffer)
{
	struct ext2_inode * ip;
	ip = &e2fs->inode_table[fd].inode;
	return ext2_breadi(e2fs, ip, blkno, nblks, buffer);
}

#if 0
/*
 * Note: don't mix any kind of file lookup or other I/O with this or
 * you will lose horribly (as it reuses blkbuf)
 */
static const char *
ext2_readdir(ext2fs_priv_state_t *e2fs, int fd, int rewind)
{
	struct ext2_inode * ip = &e2fs->inode_table[fd].inode;
	struct ext2_dir_entry_2 * ent;
	if (!S_ISDIR(ip->i_mode)) {
		ERR_PRT(("fd %d (inode %d) is not a directory (mode %x)",
		       fd, e2fs->inode_table[fd].inumber, ip->i_mode));
		return NULL;
	}
	ent = ext2_readdiri(e2fs, ip, rewind);
	if (ent) {
		ent->name[ent->name_len] = '\0';
		return ent->name;
	} else { 
		return NULL;
	}
}
#endif

static int 
ext2_fstat(ext2fs_priv_state_t *e2fs, int fd, ext2fs_stat_t *buf)
{
	struct ext2_inode * ip = &e2fs->inode_table[fd].inode;

	memset(buf, 0, sizeof(*buf));

	/* fill in relevant fields */
	buf->st_ino = e2fs->inode_table[fd].inumber;
	buf->st_mode = ip->i_mode;
	buf->st_nlink = ip->i_links_count;
	buf->st_uid = ip->i_uid;
	buf->st_gid = ip->i_gid;
	buf->st_size = ip->i_size;
	buf->st_atime = ip->i_atime;
	buf->st_mtime = ip->i_mtime;
	buf->st_ctime = ip->i_ctime;

	return 0; /* NOTHING CAN GO WROGN! */
}

static int
ext2fs_fstat(unsigned int fd, ext2fs_stat_t *st)
{
	ext2fs_priv_state_t *e2fs;

	if (this == NULL || fd > MAX_OPEN_FILES || st == NULL) return -1;

	e2fs = FS_PRIVATE(this);

	ext2_fstat(e2fs, fd, st);

	return 0;
}

uint64_t ext2fs_len(unsigned int fd)
{
ext2fs_stat_t st;

ext2fs_fstat(fd,&st);

return st.st_size;
}

int
ext2fs_seek(unsigned int fd, uint64_t newpos)
{
	ext2fs_priv_state_t *e2fs;

	if (this == NULL || fd > MAX_OPEN_FILES || newpos >= EXT2_FILESIZE_MAX) return -1;

	e2fs = FS_PRIVATE(this);
	if (newpos > (uint64_t)e2fs->inode_table[fd].inode.i_size) return -1;

	e2fs->inode_table[fd].pos = newpos;

	return 0;
}

int
ext2fs_read(unsigned int fd, void *buf, unsigned int *size)
{
	ext2fs_priv_state_t *e2fs;
	unsigned int count, nc, bofs, bnum, pos;
	int ret = -1;
	char *block;

	if (this == NULL || size == NULL || buf == NULL || fd > MAX_OPEN_FILES) return -1;

	e2fs = FS_PRIVATE(this);

	count = MIN(*size, e2fs->inode_table[fd].inode.i_size - e2fs->inode_table[fd].pos);

	if (count == 0)  {
		*size = 0;
		return 0;
	}
	block = e2fs->blkbuf;

	*size = 0;

	pos = e2fs->inode_table[fd].pos;  
	DBG_PRT(("size=%d i_size=%d count=%d pos=%ld", *size,e2fs->inode_table[fd].inode.i_size, count, pos));
	while (count) {
		bnum = pos / e2fs->blocksize;
		bofs = pos % e2fs->blocksize;
		nc   = MIN(count, e2fs->blocksize - bofs);

		DBG_PRT(("bnum =%d bofs=%d nc=%d *size=%d\n", bnum, bofs, nc, *size));

		if (ext2_bread(e2fs, fd, bnum, 1, block) == -1) goto error;
#if 0		
		{ int i; char *p = block+bofs; 
			for(i=MIN(nc, 64); i>=0 ; i--, p++) {
				if (i % 16 == 0) Print("\n");
				Print("%02x ", (unsigned int)*p & 0xff);
			}
		}
#endif

		memcpy(buf, block+bofs, nc);
		count -= nc;
		pos   += nc;
		buf   += nc;
		*size += nc;
	}

	e2fs->inode_table[fd].pos += *size;
	ret = 0;
error:
	DBG_PRT(("*size=%d ret=%r", *size, ret));
	return ret;
}

static struct ext2_inode *
ext2_follow_link(ext2fs_priv_state_t *e2fs, struct ext2_inode * from, const char * base)
{
	char *linkto;

	if (from->i_blocks) {
		linkto = e2fs->blkbuf;
		if (ext2_breadi(e2fs, from, 0, 1, e2fs->blkbuf) == -1)
			return NULL;
		DBG_PRT(("long link!"));
	} else {
		linkto = (char*)from->i_block;
	}
	DBG_PRT(("symlink to %s", linkto));

	/* Resolve relative links */
	if (linkto[0] != '/') {
		char *end = strrchr(base, '/');
		if (end) {
			//char fullname[(end - base + 1) + strlena(linkto) + 1];
			char fullname[EXT2FS_PATH_MAXLEN];

			if (((end - base + 1) + strlen(linkto) + 1) >= EXT2FS_PATH_MAXLEN) {
				printf("%s: filename too long, can't resolve\n", __FUNCTION__);
				return NULL;
			}

			strncpy(fullname, base, end - base + 1);
			fullname[end - base + 1] = '\0';
			strcat(fullname, linkto);
			DBG_PRT(("resolved to %s", fullname));
			return ext2_namei(e2fs, fullname);
		} else {
			/* Assume it's in the root */
			return ext2_namei(e2fs, linkto);
		}
	} else {
		return ext2_namei(e2fs, linkto);
	}
}

static int
ext2_open(ext2fs_priv_state_t *e2fs, char *filename)
{
	/*
	 * Unix-like open routine.  Returns a small integer (actually
	 * an index into the inode table...
	 */
	struct ext2_inode * ip;

	ip = ext2_namei(e2fs, filename);
	if (ip) {
		struct inode_table_entry *itp;

		while (S_ISLNK(ip->i_mode)) {
			ip = ext2_follow_link(e2fs, ip, filename);
			if (!ip) return -1;
		}
		itp = (struct inode_table_entry *)ip;
		return itp - e2fs->inode_table;
	} else
		return -1;
}

static void ext2_close(ext2fs_priv_state_t *e2fs, int fd)
{
	/* blah, hack, don't close the root inode ever */
	if (&e2fs->inode_table[fd].inode != e2fs->root_inode)
		ext2_iput(e2fs, &e2fs->inode_table[fd].inode);
}

int
ext2fs_close(unsigned int fd)
{
	ext2fs_priv_state_t *e2fs;

	if (this == NULL || fd > MAX_OPEN_FILES) return -1;

	e2fs = FS_PRIVATE(this);

	ext2_close(e2fs, fd);

	return 0;
}

int
ext2fs_open(char *name, unsigned int *fd)
{
	ext2fs_priv_state_t *e2fs;
	int tmp;

	DBG_PRT(("name:%s fd=%x", name, fd));

	if (this == NULL || name == NULL || fd == NULL || strlen(name) >=EXT2FS_PATH_MAXLEN) return -1;

	e2fs = FS_PRIVATE(this);

	/*
	 * XXX: for security reasons, we may have to force a prefix like /boot to all filenames
	 */

	tmp = ext2_open(e2fs, name);
	if (tmp != -1) {
		*fd = (unsigned int)tmp;
		e2fs->inode_table[tmp].pos = 0; /* reset file position */
	}

	DBG_PRT(("name: %s fd=%d tmp=%d", name, *fd, tmp));

	return tmp == -1 ? -1:0;
}

#if 0
static int ext2fs_name(ext2fs_interface_t *this, char *name, unsigned int maxlen)
{
	if (name == NULL || maxlen < 1) return -1;

	strncpy(name, FS_NAME, maxlen-1);

	name[maxlen-1] = '\0';

	return 0;
}
#endif

static int
ext2fs_init_state(ext2fs_t *ext2fs, struct ext2_super_block *sb)
{
	ext2fs_priv_state_t *e2fs = FS_PRIVATE(ext2fs);
	unsigned int i;
	int status;

	memset(ext2fs, 0, sizeof(*ext2fs));

	/* fools gcc builtin memcpy */
	memcpy(&e2fs->sb, sb, sizeof(*sb));

	e2fs->ngroups = (sb->s_blocks_count - sb->s_first_data_block + EXT2_BLOCKS_PER_GROUP(sb) - 1) / EXT2_BLOCKS_PER_GROUP(sb);

	e2fs->gds = (struct ext2_group_desc *)bl_malloc(e2fs->ngroups * sizeof(struct ext2_group_desc));
	if (e2fs->gds == NULL) {
		ERR_PRT(("failed to bl_mallocate gds"));
		return -1;
	}
	
	e2fs->blocksize = EXT2_BLOCK_SIZE(sb);

	DBG_PRT(("gds_size=%d gds_offset=%d ngroups=%d blocksize=%d\n",
				e2fs->ngroups * sizeof(struct ext2_group_desc), 
				e2fs->blocksize * (EXT2_MIN_BLOCK_SIZE/e2fs->blocksize + 1),
				e2fs->ngroups, (unsigned int)e2fs->blocksize));

	/* read in the group descriptors (immediately follows superblock) */
	status = read_bytes(e2fs->blocksize * (EXT2_MIN_BLOCK_SIZE/e2fs->blocksize + 1),
			e2fs->gds, e2fs->ngroups * sizeof(struct ext2_group_desc));
	if (status) {
		ERR_PRT(("cannot read gds: %r", status));
		bl_free(e2fs->gds);
		return -1;
	}
#if 0
	{ int i; char *p = (char *)e2fs->gds;
		for(i=e2fs->ngroups*sizeof(*e2fs->gds); i ; i--, p++) {
			if (i % 16 == 0) Print("\n");
			Print("%02x ", (unsigned int)*p & 0xff);

		}
	}
#endif

	e2fs->cached_diblkno = -1;
	e2fs->cached_iblkno  = -1;

	/* initialize the inode table */
	for (i = 0; i < MAX_OPEN_FILES; i++) {
		e2fs->inode_table[i].bl_free = 1;
		e2fs->inode_table[i].inumber = 0;
	}
	/* clear the root inode pointer (very important!) */
	e2fs->root_inode = NULL;

	/*
	 * Calculate direct/indirect block limits for this file system
	 * (blocksize dependent):
	ext2_blocksize = EXT2_BLOCK_SIZE(&sb);
	 */
	e2fs->directlim    = EXT2_NDIR_BLOCKS - 1;
	e2fs->ptrs_per_blk = e2fs->blocksize/sizeof(unsigned int);
	e2fs->ind1lim      = e2fs->ptrs_per_blk + e2fs->directlim;
	e2fs->ind2lim      = (e2fs->ptrs_per_blk * e2fs->ptrs_per_blk) + e2fs->directlim;

	return 0;
}


 
int ext2fs_fs_open(void)
{
	struct ext2_super_block sb;
	long sb_block = 1;
	int status;


	status = read_bytes(sb_block * EXT2_MIN_BLOCK_SIZE, &sb, sizeof(sb));
	if (status) {
		DBG_PRT(("cannot read superblock: %d\n", status));
		return -1;
	}
	
	if (sb.s_magic != EXT2_SUPER_MAGIC) {
		DBG_PRT(("bad magic 0x%x\n", sb.s_magic));
		return -1;
	}
	
	this=&ext2fs_alloc;

	status = ext2fs_init_state(this, &sb);
	if (status) {
		this=NULL;
		return -1;
	}


#if 0
	VERB_PRT(3,
		{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  dp  = DevicePathFromHandle(dev);
		  str = DevicePathToStr(dp);
		  Print("dev:%s %s detected\n", str, FS_NAME);
		  FreePool(str);
		});
#endif

return 0;
}
	

int ext2fs_fs_close(void)
{
this=NULL;
}
