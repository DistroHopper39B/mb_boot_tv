#include "project.h"

#define NP 16

char buf[1024*1024*8];

main()
{
struct partition p[NP];
int n=NP;
int fd;
int sz,ret;
disk_open(0);

if (efi_find_partitions(p,&n)) {
	disk_close();
	return -1;
}

partition_set(&p[1]);

ext2fs_fs_open();
ext2fs_open("/vmlinuz",&fd);

sz=sizeof(buf);
ret=ext2fs_read(fd,buf,&sz);

ext2fs_close(fd);

printf("Read %d bytes,%d\n",sz,ret);



disk_close();
}
