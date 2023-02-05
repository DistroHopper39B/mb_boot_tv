/*
 *  linux/include/linux/ext2_fs_i.h
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs_i.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#ifndef _LINUX_EXT2_FS_I
#define _LINUX_EXT2_FS_I

/*
 * second extended file system inode data in memory
 */
struct ext2_inode_info {
	uint32_t	i_data[15];
	uint32_t	i_flags;
	uint32_t	i_faddr;
	uint8_t	i_frag_no;
	uint8_t	i_frag_size;
	uint16_t	i_osync;
	uint32_t	i_file_acl;
	uint32_t	i_dir_acl;
	uint32_t	i_dtime;
	uint32_t	not_used_1;	/* FIX: not used/ 2.2 placeholder */
	uint32_t	i_block_group;
	uint32_t	i_next_alloc_block;
	uint32_t	i_next_alloc_goal;
	uint32_t	i_prealloc_block;
	uint32_t	i_prealloc_count;
	uint32_t	i_high_size;
	int	i_new_inode:1;	/* Is a freshly allocated inode */
};

#endif	/* _LINUX_EXT2_FS_I */
