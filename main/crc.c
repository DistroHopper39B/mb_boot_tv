/*
 * crc.c:
 *
 * Code taken from linux kernel
 *
 */

static char rcsid[] = "$Id: crc.c,v 1.4 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: crc.c,v $
 * Revision 1.4  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"

#define CRCPOLY_LE 0xedb88320


uint32_t
crc32_le (uint32_t crc, unsigned char const *p, uint32_t len)
{
  int i;
  while (len--)
    {
      crc ^= *p++;
      for (i = 0; i < 8; i++)
        crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
    }
  return crc;
}
