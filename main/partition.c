/*
 * partition.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: partition.c,v 1.4 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: partition.c,v $
 * Revision 1.4  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"


static uint64_t start = 0;
static uint64_t size = 0;

void
partition_set (struct partition *p)
{
  start = p->start;
  size = p->size;
}

int
partition_read (void *_buf, uint64_t sector, uint64_t nsectors)
{
  uint8_t *buf = (uint8_t *) _buf;
  int red = 0;

//printf("partition_read %lld for %lld\n",sector,nsectors);

  if ((sector + nsectors) > size)
    return -1;

  sector += start;

  while (nsectors--)
    {
//printf("partition_read from lba %lld\n",sector);
      if (disk_read (buf, sector))
        {
          return red;
        }
      red++;
      sector++;
      buf += 512;
    }

  return red;
}
