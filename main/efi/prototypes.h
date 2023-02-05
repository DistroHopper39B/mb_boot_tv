/* crc.c */
uint32_t crc32_le(uint32_t crc, unsigned char const *p, uint32_t len);
/* disk.c */
uint64_t disk_last_sector(void);
int disk_read(void *buf, int64_t sector);
int disk_open(int drive);
void disk_close(void);
/* e2.c */
int ext2fs_seek(unsigned int fd, uint64_t newpos);
int ext2fs_read(unsigned int fd, void *buf, unsigned int *size);
int ext2fs_close(unsigned int fd);
int ext2fs_open(char *name, unsigned int *fd);
int ext2fs_fs_open(void);
int ext2fs_fs_close(void);
/* efi.c */
int efi_find_partitions(struct partition *partitions, int *num);
/* main.c */
int main(void);
/* malloc.c */
void *bl_malloc(size_t bytes);
void bl_free(void *mem);
void *dlcalloc(size_t n_elements, size_t elem_size);
void *dlrealloc(void *oldmem, size_t bytes);
void *dlmemalign(size_t alignment, size_t bytes);
void **dlindependent_calloc(size_t n_elements, size_t elem_size, void *chunks[]);
void **dlindependent_comalloc(size_t n_elements, size_t sizes[], void *chunks[]);
void *dlvalloc(size_t bytes);
void *dlpvalloc(size_t bytes);
int dlmalloc_trim(size_t pad);
size_t dlmalloc_footprint(void);
size_t dlmalloc_max_footprint(void);
struct mallinfo dlmallinfo(void);
void dlmalloc_stats(void);
size_t dlmalloc_usable_size(void *mem);
int dlmallopt(int param_number, int value);
/* partition.c */
void partition_set(struct partition *p);
int partition_read(void *_buf, uint64_t sector, uint64_t nsectors);
/* string.c */
char *strtok_simple(char *in, char c);
