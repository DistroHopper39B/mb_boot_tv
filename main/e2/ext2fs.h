/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 * This file is part of the ELILO, the EFI Linux boot loader.
 *
 *  ELILO is free software; you can redistribute it and/or modify
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
 */
#ifndef __EXT2FS_H__
#define __EXT2FS_H__

//INTERFACE_DECL(_ext2fs_interface_t);

/*
 * simplified stat structure
 * XXX: need to cleanup types !
 */
typedef struct {
	unsigned long	st_ino;
	unsigned long	st_nlink;
	unsigned int	st_mode;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned long	st_size;
	unsigned long	st_atime;
	unsigned long	st_mtime;
	unsigned long	st_ctime;
} ext2fs_stat_t;


typedef struct _ext2fs_interface_t {
	int (*ext2fs_name)(struct _ext2fs_interface_t *this, char *name, unsigned int maxlen);
	int (*ext2fs_open)(struct _ext2fs_interface_t *this, char *name, unsigned int *fd);
	int (*ext2fs_read)(struct _ext2fs_interface_t *this, unsigned int fd, void *buf, unsigned int *size);
	int (*ext2fs_close)(struct _ext2fs_interface_t *this, unsigned int fd);
	int (*ext2fs_fstat)(struct _ext2fs_interface_t *this, unsigned int fd, ext2fs_stat_t *st);
	int (*ext2fs_seek)(struct _ext2fs_interface_t *this, unsigned int fd, uint64_t newpos);
} ext2fs_interface_t;


#endif /* __EXT2FS_H__ */
