/*
 * eb-disk.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: eb-disk.c,v 1.5 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: eb-disk.c,v $
 * Revision 1.5  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "eb.h"
#include "eb/disk.h"


#undef disk_disable

int
disk_probe (struct dev *dev)
{
  struct disk *disk = (struct disk *) dev;
  if (dev->how_probe == PROBE_NEXT)
    {
      disk->drive += 1;
    }
  return probe (dev);
}


void
disk_disable (void)
{
  disable (&disk.dev);
}
