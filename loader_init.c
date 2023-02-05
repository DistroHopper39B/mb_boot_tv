#include "types.h"
#include "sysdeps.h"
#include "sha1.h"
#include <string.h>

int sprintf(char * buf, const char *fmt, ...);


INTN
sysdeps_create_boot_params(
	boot_params_t *bp,void *ramdisk,int ramdisk_len,
	unsigned char *cmdline);

extern void start_kernel(VOID *kentry, boot_params_t *bp);

char * strcpy(char * dest,const char *src)
{
	int d0, d1, d2;
	__asm__ __volatile__(
       		"1:\tlodsb\n\t"
        	"stosb\n\t"
       		"testb %%al,%%al\n\t"
       		"jne 1b"
    		: "=&S" (d0), "=&D" (d1), "=&a" (d2)
     		:"0" (src),"1" (dest) : "memory");
	return dest;
}

size_t strlen(const char * s)
{
	int d0;
	register int __res;
	__asm__ __volatile__(
        	"repne\n\t"
	       	"scasb\n\t"
        	"notl %0\n\t"
       		"decl %0"
		:"=c" (__res), "=&D" (d0) :"1" (s),"a" (0), "0" (0xffffffffu));
	return __res;
}

void * memcpy(void * to, const void * from, size_t n)
{
	int d0, d1, d2;
	__asm__ __volatile__(
       		"rep ; movsl\n\t"
       		"testb $2,%b4\n\t"
      		"je 1f\n\t"
      	 	"movsw\n"
      		"1:\ttestb $1,%b4\n\t"
     		"je 2f\n\t"
     		"movsb\n"
     	  	"2:"
     		: "=&c" (d0), "=&D" (d1), "=&S" (d2)
		:"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
	       	: "memory");
	return (to);
}

void * memset(void *s, int c,  size_t count)
{
  	int d0, d1;
	__asm__ __volatile__(
	        "rep\n\t"
	        "stosb"
	        : "=&c" (d0), "=&D" (d1)
	        :"a" (c),"1" (s),"0" (count)
	        :"memory");
	return s;
}

boot_parms *bootparms;

int memcmp(const void * cs,const void * ct,size_t count)
{
        const unsigned char *su1, *su2;
        int res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0) break;
	return res;
}

void shax(unsigned char *result, unsigned char *data, unsigned int len)
{
	struct SHA1Context context;
	SHA1Reset(&context);
	SHA1Input(&context, (unsigned char *)&len, 4);
	SHA1Input(&context, data, len);
	SHA1Result(&context,result);	
}

#define KERNEL_BUFFER		0x400000
#define BOOT_PARAM_MEMSIZE      16384


void boot_kernel(void *k,int k_len,void *r,int r_len,char *o)
{
uint8_t *kc=(uint8_t *)k;
	unsigned char szBootSect[BOOT_PARAM_MEMSIZE];
	boot_params_t *bp = (boot_params_t *) szBootSect;

	kdesc_t kd;

	if (kc[0x1FE] != 0x55 || kc[0x1FF] != 0xAA) {
		printk("Kernel is not a bzImage kernel image.\n");
		return;
	}

	printk("reading setup data...\n");

        kd.kstart = kd.kentry = kernel_start;
        kd.kend = ((UINT8 *)kd.kstart) + KERNEL_BUFFER;

        printk("kstart=0x%x  kentry=0x%x  kend=0x%x\n", kd.kstart, kd.kentry, kd.kend);
	
	MEMSET(kernel_start, KERNEL_BUFFER, 0);

	memcpy(kernel_start, &kc[(kc[0x1F1] + 1) * 512], k_len - ((kc[0x1F1] + 1) * 512));
	MEMSET(bp, BOOT_PARAM_MEMSIZE, 0);
	memcpy(bp, k, 0x2000);

	sprintf(&szBootSect[BOOT_PARAM_MEMSIZE - 2048], "%s video=imacfb:appletv,width:%d,height:%d,linelength:%d,base:%d", 
		o, bootparms->video.width, bootparms->video.height, bootparms->video.rowb, bootparms->video.addr);

	if (sysdeps_create_boot_params(bp, r,r_len,&szBootSect[BOOT_PARAM_MEMSIZE - 2048]) == -1) {
		while(1);
	}

	printk("efi_mem_map         0x%08X\n", bp->s.efi_mem_map);
	printk("efi_mem_map_size    0x%08X\n", bp->s.efi_mem_map_size);
	printk("efi_mem_desc_size   0x%08X\n", bp->s.efi_mem_desc_size);
	printk("efi_mem_desc_ver    0x%08X\n", bp->s.efi_mem_desc_ver);
        printk("efi_sys_tbl         0x%08X\n", bp->s.efi_sys_tbl);

	EFI_SYSTEM_TABLE *system_table = (EFI_SYSTEM_TABLE *) bootparms->efi_sys_tbl;
	EFI_RUNTIME_SERVICES *runtime = (EFI_RUNTIME_SERVICES *) system_table->RuntimeServices;	

	start_kernel(kd.kentry, bp);

}



void loader_init(unsigned int args)
{
	int i;
	char *ptr;
	unsigned char sha_Message_Digest[SHA1HashSize];
	kdesc_t kd;
	memdesc_t imem, mmem;

	bootparms = (boot_parms *)args;

	vmode.width = bootparms->video.rowb / 4;
	vmode.height = bootparms->video.height;
	MEMSET(bootparms->video.addr, vmode.width * vmode.height *4, 0x00);
	vmode.xmargin = 0;
	VIDEO_CURSOR_POSX = 0;
	VIDEO_CURSOR_POSY = 0;
	VIDEO_ATTR=0xffc8c8c8;

	printk("Hello AppleTV\n");
	printk("FB Start 0x%08X, with %d height %d rowb %d depth %d\n", bootparms->video.addr, bootparms->video.width,  bootparms->video.height, bootparms->video.rowb, bootparms->video.depth);

	{
	extern void do_main(void);
	do_main();
	}

	printk("etherboot code returned\n");

	while(1);
}


