#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

#define CMDLINE	1024

struct viedo_parms {
	uint32_t	addr;
	uint32_t	disp;
	uint32_t	rowb;
	uint32_t	width;
	uint32_t	height;
	uint32_t	depth;
} __attribute__((aligned(4)));

typedef struct viedo_parms viedo_parms;

typedef struct boot_parms {
    uint16_t	rev;
    uint16_t	ver;
    char	cmdline[CMDLINE];
    uint32_t    efi_mem_map;
    uint32_t    efi_mem_map_size;
    uint32_t    efi_mem_desc_size;
    uint32_t    efi_mem_desc_ver;
    viedo_parms	video;
    uint32_t    devtree;
    uint32_t	devtreel;
    uint32_t    kaddr;
    uint32_t    ksize;
    uint32_t    efi_runtime_page;
    uint32_t    efi_runtime_page_count;
    uint32_t    efi_sys_tbl;
    uint8_t     efi_mode;
    uint8_t     __reserved1[3];
    uint32_t    __reserved2[7];

} __attribute__((aligned(4))) boot_parms;

typedef int64_t INT64;
typedef unsigned int          UINT32;
typedef short                 CHAR16;
typedef void *                EFI_HANDLE;
typedef void 		      VOID;
typedef unsigned long           UINTN;
typedef int16_t INT16;
typedef unsigned char u8;
typedef unsigned int u32;
typedef int bool;
typedef unsigned short u16;
#define false 0
#define true 1
typedef unsigned long RGBA; 

typedef uint16_t   UINT16;
typedef uint8_t    UINT8;
typedef unsigned long long      UINT64;
typedef int32_t    INTN;

typedef UINT8           CHAR8;

#define NULL 0

typedef struct {
	u32 width; // everything else filled by BootVgaInitializationKernel() on return
	u32 height;
	u32 xmargin;
} CURRENT_VIDEO_MODE_DETAILS;

volatile CURRENT_VIDEO_MODE_DETAILS vmode;

void BootVgaInitializationKernelNG(CURRENT_VIDEO_MODE_DETAILS * pvmode);

#define ELILO_LOAD_SUCCESS        0
#define ELILO_LOAD_ABORTED        1
#define ELILO_LOAD_ERROR          2

typedef struct {
        void *kstart;
        void *kend;
        void *kentry;
} kdesc_t;

typedef uint32_t   fops_fd_t;

typedef struct {
        void    *start_addr; 
        uint32_t   pgcnt;
        uint32_t   size;
} memdesc_t;

#define PADDR_MASK 0xfffffff

#define VERB_PRT(n,cmd)

#define __KERNEL_DS   0x18


typedef unsigned int    boolean_t;

volatile uint32_t VIDEO_CURSOR_POSY;
volatile uint32_t VIDEO_CURSOR_POSX;
volatile uint32_t VIDEO_ATTR;

extern boot_parms *bootparms;

typedef UINT64          EFI_PHYSICAL_ADDRESS;
typedef UINT64          EFI_VIRTUAL_ADDRESS;

typedef struct {
    UINT32                          Type;           // Field size is 32 bits followed by 32 bit pad
    EFI_PHYSICAL_ADDRESS            PhysicalStart;  // Field size is 64 bits
    EFI_VIRTUAL_ADDRESS             VirtualStart;   // Field size is 64 bits
    UINT64                          NumberOfPages;  // Field size is 64 bits
    UINT64                          Attribute;      // Field size is 64 bits
} EFI_MEMORY_DESCRIPTOR;

typedef struct _EFI_TABLE_HEARDER {
    UINT64                      Signature;
    UINT32                      Revision;
    UINT32                      HeaderSize;
    UINT32                      CRC32;
    UINT32                      Reserved;
} EFI_TABLE_HEADER;

typedef struct  {
    EFI_TABLE_HEADER                Hdr;

} EFI_RUNTIME_SERVICES;

typedef struct _EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;

    CHAR16                          *FirmwareVendor;
    UINT32                          FirmwareRevision;

    EFI_HANDLE                      ConsoleInHandle;
    VOID                            *ConIn;

    EFI_HANDLE                      ConsoleOutHandle;
    VOID                            *ConOut;

    EFI_HANDLE                      StandardErrorHandle;
    VOID                            *StdErr;

    EFI_RUNTIME_SERVICES            *RuntimeServices;
    VOID                            *BootServices;

    UINTN                           NumberOfTableEntries;
    VOID                            *ConfigurationTable;

} EFI_SYSTEM_TABLE;

#endif
