/*
 * disk.c:
 *
 * incorporates code from etherboot
 *
 */

static char rcsid[] = "$Id: disk.c,v 1.3 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: disk.c,v $
 * Revision 1.3  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "eb.h"
#include "eb/disk.h"
#undef disk_disable

static int
dummy (void *unused)
{
  return (0);
}

static unsigned char disk_buffer[DISK_BUFFER_SIZE];
struct disk disk = {
  {
   0,                           /* dev.disable */
   {
    0,
    0,
    PCI_BUS_TYPE,
    },                          /* dev.devid */
   0,                           /* index */
   0,                           /* type */
   PROBE_FIRST,                 /* how_probe */
   PROBE_NONE,                  /* to_probe */
   0,                           /* failsafe */
   0,                           /* type_index */
   {},                          /* state */
   },
  (int (*)(struct disk *, sector_t)) dummy, /* read */
  0 - 1,                        /* drive */
  0,                            /* hw_sector_size */
  0,                            /* sectors_per_read */
  0,                            /* bytes */
  0,                            /* sectors */
  0,                            /* sector */
  disk_buffer,                  /* buffer */
  0,                            /* priv */

  0,                            /* disk_offset */
  0,                            /* direction */
};


static int
do_disk_read (struct disk *disk, unsigned char *buffer, uint64_t sector)
{
  int result;
  sector_t base_sector;

  /* Note: I do not handle disk wrap around here! */

  /* Compute the start of the track cache */
  base_sector = sector;
  /* Support sectors_per_read > 1 only on small disks */
  if ((sizeof (sector_t) > sizeof (unsigned long)) &&
      (disk->sectors_per_read > 1))
    {
      unsigned long offset;
      offset = ((unsigned long) sector) % disk->sectors_per_read;
      base_sector -= offset;
    }

  /* See if I need to update the track cache */
  if ((sector < disk->sector) || sector >= disk->sector + (disk->bytes >> 9))
    {
      twiddle ();
      result = disk->read (disk, base_sector);
      if (result < 0)
        return result;
    }
  /* Service the request from the track cache */
  memcpy (buffer, disk->buffer + ((sector - base_sector) << 9), SECTOR_SIZE);
  return 0;
}


int
disk_read (void *buf, int64_t sector)
{
  return do_disk_read (&disk, (uint8_t *) buf, sector);
}


int
disk_open (int drive)
{

  memset (&disk, 0, sizeof (disk));
  disk.buffer = disk_buffer;
  disk.drive = 0;
  disk.dev.how_probe = PROBE_FIRST;
  disk.dev.type = DISK_DRIVER;

  do
    {
      disk_disable ();
      disk.dev.how_probe = disk_probe (&disk.dev);
      if (disk.dev.how_probe == PROBE_FAILED)
        {
          printf ("Not that many drives\n");
          return -1;
        }
    }
  while (disk.drive < drive);

  return 0;

}

void
disk_close (void)
{
  disk_disable ();
}

uint64_t
disk_last_sector (void)
{
  return disk.sectors - 1ULL;
}
