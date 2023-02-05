#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <unistd.h>

#include "project.h"

static int fd=0;

uint64_t disk_last_sector(void)
{
uint64_t o=0;
o=lseek(fd,0,SEEK_END);

o=o >> 9;

o--;
return o;
}

int  disk_read(void *buf,int64_t sector)
{
bzero(buf,512);
uint64_t o;
o=512;
o=o*sector;

if (lseek(fd,o,SEEK_SET)!=o) return 1;
if (read(fd,buf,512)!=512) return 1;

return 0;
}

int disk_open(int drive)
{
fd=open("/dev/sda",O_RDONLY);
}

void disk_close(void)
{
close(fd);
}

