/*
 * project.h:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

/*
 * $Id: project.h,v 1.8 2007/04/26 11:03:07 james Exp $
 */

/*
 * $Log: project.h,v $
 * Revision 1.8  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#ifndef __PROJECT_H__
#define __PROJECT_H__

#include "../types.h"
#include "../sysdeps.h"
#include "../sha1.h"
#include <stdarg.h>

#define VER "1.23"


typedef uint32_t size_t;

#include "partition.h"

#define P(a...) a

#include "pci.h"
#include "timer.h"

#include "prototypes.h"


#define FILE_BUF_SIZE (256*1024)

int printk (const char *szFormat, ...);
//void *memcpy(void *dest,const void *src,int n);
void *memset (void *s, int c, size_t count);
void *memcpy (void *to, const void *from, size_t n);
char *strcpy (char *dest, const char *src);
size_t strlen (const char *s);
int sprintf (char *buf, const char *fmt, ...);
int printk (const char *szFormat, ...);



void boot_kernel (void *k, int k_len, void *r, int r_len, char *o);

int disk_read (void *buf, int64_t sector);
int disk_open (int drive);
void disk_close (void);
#endif /* __PROJECT_H__ */
