/*
 * mbr.h:
 *
 * code from kernel
 *
 */

/*
 * $Id: mbr.h,v 1.3 2007/04/26 11:03:07 james Exp $
 */

/*
 * $Log: mbr.h,v $
 * Revision 1.3  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#ifndef __MBR_H__
#define __MBR_H__

struct mbr_partition
{
  unsigned char boot_ind;       /* 0x80 - active */
  unsigned char head;           /* starting head */
  unsigned char sector;         /* starting sector */
  unsigned char cyl;            /* starting cylinder */
  unsigned char sys_ind;        /* What partition type */
  unsigned char end_head;       /* end head */
  unsigned char end_sector;     /* end sector */
  unsigned char end_cyl;        /* end cylinder */
  uint32_t start_sect;          /* starting sector counting from 0 */
  uint32_t nr_sects;            /* nr of sectors in partition */
} __attribute__ ((packed));
#endif /* __MBR_H__ */
