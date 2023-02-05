/* crc.c */
extern volatile CURRENT_VIDEO_MODE_DETAILS vmode;
extern volatile uint32_t VIDEO_CURSOR_POSY;
extern volatile uint32_t VIDEO_CURSOR_POSX;
extern volatile uint32_t VIDEO_ATTR;
extern uint32_t crc32_le(uint32_t crc, unsigned char const *p, uint32_t len);
/* disk.c */
extern struct disk disk;
extern int disk_read(void *buf, int64_t sector);
extern int disk_open(int drive);
extern void disk_close(void);
extern uint64_t disk_last_sector(void);
/* diskboot.c */
extern void *disk_read_file(char *name, int *size);
extern void try_disk_boot(int trynetwork);
/* e2.c */
extern uint64_t ext2fs_len(unsigned int fd);
extern int ext2fs_seek(unsigned int fd, uint64_t newpos);
extern int ext2fs_read(unsigned int fd, void *buf, unsigned int *size);
extern int ext2fs_close(unsigned int fd);
extern int ext2fs_open(char *name, unsigned int *fd);
extern int ext2fs_fs_open(void);
extern int ext2fs_fs_close(void);
/* eb.c */
extern unsigned long virt_offset;
extern char as_main_program;
extern void *memmove(void *dest, const void *src, size_t count);
extern void flail(void);
extern int iskey(void);
extern int getchar(void);
extern void loadkernel(char *kernel);
extern void *network_load_file(char *name, int *len);
extern void network_shutdown(void);
extern void network_init(void);
/* efi.c */
extern int efi_find_partitions(struct partition *partitions, int *num);
/* hexdump.c */
extern void hex_dump(const unsigned char *data, const unsigned int len);
/* main.c */
extern void spin(void);
extern void abort(void);
extern void parse_config(char *config, int len, char *kernel, char *options, char *ramdisk, int *trynetboot, int *forcediskboot);
extern void pci_scan(void);
extern void network_boot_failed(void);
extern void do_main(void);
/* malloc.c */
extern void *bl_malloc(size_t s);
extern void bl_free(void *p);
/* netboot.c */
extern void dhcp_callback(char *name);
extern void try_network_boot(void);
/* partition.c */
extern void partition_set(struct partition *p);
extern int partition_read(void *_buf, uint64_t sector, uint64_t nsectors);
/* pci.c */
extern uint8_t conf1_read_8(int bus, int devfn, int off);
extern uint16_t conf1_read_16(int bus, int devfn, int off);
extern uint32_t conf1_read_32(int bus, int devfn, int off);
extern void conf1_write_8(int bus, int devfn, int off, uint8_t val);
/* sleep.c */
extern void msleep(int s);
extern void sleep(int s);
/* string.c */
extern int is_space(char l);
extern char *match(char *l, char *m);
extern char *strncpy(char *dst, const char *src, size_t n);
extern char *strcat(char *dst, const char *src);
extern int strncmp(const char *s1, const char *s2, int n);
extern char *strchr(const char *s, int c);
extern char *strrchr(const char *s, int c);
extern char *strtok_simple(char *in, char c);
/* time.c */
extern int mseconds(void);
extern int seconds(void);
/* timer.c */
extern void load_timer2(unsigned int ticks);
extern int timer2_running(void);
extern void waiton_timer2(unsigned int ticks);
extern void setup_timers(void);
extern void mdelay(unsigned int msecs);
extern void udelay(unsigned int usecs);
extern void ndelay(unsigned int nsecs);
extern unsigned long currticks(void);
/* version.c */
extern char *get_version(void);
