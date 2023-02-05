/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *	Contributed by Mike Johnston <johnston@intel.com>
 *	Contributed by Chris Ahna <christopher.j.ahna@intel.com>
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

/*
 * This file contains all the IA-32 specific code expected by generic loader
 */

#include "types.h"
#include "sysdeps.h"
#include <string.h>

typedef struct {
        EFI_MEMORY_DESCRIPTOR   *md;
        UINTN                   map_size;
        UINTN                   desc_size; 
        UINTN                   cookie;
        UINT32                  desc_version;
} mmap_desc_t;

/*
 * Descriptor table base addresses & limits for Linux startup.
 */

dt_addr_t gdt_addr = { 0x800, 0x94000 };
dt_addr_t idt_addr = { 0, 0 }; 

/*
 * Initial GDT layout for Linux startup.
 */

UINT16 init_gdt[] = {
	/* gdt[0]: dummy */
	0, 0, 0, 0, 
	
	/* gdt[1]: unused */
	0, 0, 0, 0,

	/* gdt[2]: code */
	0xFFFF,		/* 4Gb - (0x100000*0x1000 = 4Gb) */
	0x0000,		/* base address=0 */
	0x9A00,		/* code read/exec */
	0x00CF,		/* granularity=4096, 386 (+5th nibble of limit) */

	/* gdt[3]: data */
	0xFFFF,		/* 4Gb - (0x100000*0x1000 = 4Gb) */
	0x0000,		/* base address=0 */
	0x9200,		/* data read/write */
	0x00CF,		/* granularity=4096, 386 (+5th nibble of limit) */
};

UINTN sizeof_init_gdt = sizeof init_gdt;

/*
 * Highest available base memory address.
 *
 * For traditional kernels and loaders this is always at 0x90000.
 * For updated kernels and loaders this is computed by taking the
 * highest available base memory address and rounding down to the
 * nearest 64 kB boundary and then subtracting 64 kB.
 *
 * A non-compressed kernel is automatically assumed to be an updated
 * kernel.  A compressed kernel that has bit 6 (0x40) set in the
 * loader_flags field is also assumed to be an updated kernel.
 */

UINTN high_base_mem = 0x90000;

/*
 * Highest available extended memory address.
 *
 * This is computed by taking the highest available extended memory
 * address and rounding down to the nearest EFI_PAGE_SIZE (usually
 * 4 kB) boundary.  
 * This is only used for backward compatibility.
 */

UINTN high_ext_mem = 32 * 1024 * 1024;

/* This starting address will hold true for all of the loader types for now */
VOID *kernel_start = (VOID *)0x100000;	/* 1M */

VOID *initrd_start = NULL;
UINTN initrd_size = 0;

/*
 * IA-32 specific boot parameters initialization routine
 */
INTN
sysdeps_create_boot_params(
	boot_params_t *bp,void *ramdisk,int ramdisk_size,
	unsigned char *cmdline)
{
	mmap_desc_t mdesc;
	UINTN rows, cols;
	UINT8 row, col;
	UINT8 mode;
	UINT16 hdr_version;

	/*
	 * Save off our header revision information.
	 */
	hdr_version = (bp->s.hdr_major << 8) | bp->s.hdr_minor;

	/*
	 * Clear out unused memory in boot sector image.
	 */
	bp->s.unused_1 = 0;
	bp->s.unused_2 = 0;
	MEMSET(bp->s.unused_3, sizeof bp->s.unused_3, 0);
	MEMSET(bp->s.unused_4, sizeof bp->s.unused_4, 0);
	MEMSET(bp->s.unused_5, sizeof bp->s.unused_5, 0);
	bp->s.unused_6 = 0;
	bp->s.unused_7 = 0;

	/*
	 * Tell kernel this was loaded by an advanced loader type.
	 * If this field is zero, the initrd_start and initrd_size
	 * fields are ignored by the kernel.
	 */

	bp->s.loader_type = LDRTYPE_ELILO;

	/*
	 * Setup command line information.
	 */

	bp->s.cmdline_magik = CMDLINE_MAGIK;
	bp->s.cmdline_offset = (UINT8 *)cmdline - (UINT8 *)bp;

	/* 
	 * Clear out the cmdline_addr field so the kernel can find 
	 * the cmdline.
	 */
	bp->s.cmdline_addr = 0x0;

	/*
	 * Setup hard drive parameters.
	 * %%TBD - It should be okay to zero fill the hard drive
	 * info buffers.  The kernel should do its own detection.
	 */

	MEMSET(bp->s.hd0_info, sizeof bp->s.hd0_info, 0);
	MEMSET(bp->s.hd1_info, sizeof bp->s.hd1_info, 0);

	/*
	 * Memory info.
	 */

	bp->s.alt_mem_k = high_ext_mem / 1024;

	if (bp->s.alt_mem_k <= 65535) 
		bp->s.ext_mem_k = (UINT16)bp->s.alt_mem_k;
	else 
		bp->s.ext_mem_k = 65535;

	/*
	 * Initial RAMdisk and root device stuff.
	 */

	/* These RAMdisk flags are not needed, just zero them. */
	bp->s.ramdisk_flags = 0;

	if (ramdisk && ramdisk_size) {
		bp->s.initrd_start = (uint32_t) ramdisk;
		bp->s.initrd_size = ramdisk_size;

		bp->s.orig_root_dev=0x0100;
	} else {
		bp->s.initrd_start = 0;
		bp->s.initrd_size = 0;
	}

	/*
	 * APM BIOS info.
	 */
	bp->s.apm_bios_ver = NO_APM_BIOS;
	bp->s.bios_code_seg = 0;
	bp->s.bios_entry_point = 0;
	bp->s.bios_code_seg16 = 0;
	bp->s.bios_data_seg = 0;
	bp->s.apm_bios_flags = 0;
	bp->s.bios_code_len = 0;
	bp->s.bios_data_len = 0;

	/*
	 * MCA BIOS info (misnomer).
	 */
	bp->s.mca_info_len = 0;
	MEMSET(bp->s.mca_info_buf, sizeof bp->s.mca_info_buf, 0);

	/*
	 * Pointing device presence.  The kernel will detect this.
	 */
	bp->s.aux_dev_info = NO_MOUSE;

	/*
	 * EFI loader signature 
	 */
	memcpy(bp->s.efi_loader_sig, EFI_LOADER_SIG, 4);

	/*
	 * Kernel entry point.
	 */
	bp->s.kernel_start = (UINT32)kernel_start;

	/*
	 * Get video information.
	 * Do this last so that any other cursor positioning done
	 * in the fill routine gets accounted for.
	 */

	mode = 3;
	rows = 25;
	cols = 80;
	row = 24;
	col = 0;

	bp->s.orig_cursor_col = col;
	bp->s.orig_cursor_row = row;
	bp->s.orig_video_page = 0;
	bp->s.orig_video_mode = mode;
	bp->s.orig_video_cols = (UINT8)cols;
	bp->s.orig_video_rows = (UINT8)rows;

	bp->s.orig_ega_bx = 0;
	bp->s.is_vga = 0;
	bp->s.orig_video_points = 16; 

	bp->s.lfb_width = 0;
	bp->s.lfb_height = 0;
	bp->s.lfb_depth = 0;
	bp->s.lfb_base = 0;
	bp->s.lfb_size = 0;
	bp->s.lfb_line_len = 0;
	bp->s.lfb_red_size = 0;
	bp->s.lfb_red_pos = 0;
	bp->s.lfb_green_size = 0;
	bp->s.lfb_green_pos = 0;
	bp->s.lfb_blue_size = 0;
	bp->s.lfb_blue_pos = 0;
	bp->s.lfb_rsvd_size = 0;
	bp->s.lfb_rsvd_pos = 0;
	bp->s.lfb_pages = 0;
	bp->s.vesa_seg = 0;
	bp->s.vesa_off = 0;

	bp->s.efi_mem_map = bootparms->efi_mem_map;
	bp->s.efi_mem_map_size = bootparms->efi_mem_map_size;
	bp->s.efi_mem_desc_size = bootparms->efi_mem_desc_size;
	bp->s.efi_mem_desc_ver = bootparms->efi_mem_desc_ver;
	bp->s.efi_sys_tbl = bootparms->efi_sys_tbl;
	
	return 0;
}
