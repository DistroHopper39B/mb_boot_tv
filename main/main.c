/*
 * main.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: main.c,v 1.9 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: main.c,v $
 * Revision 1.9  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"

void
spin (void)
{
  printk ("failed to boot, spinning\n");
  for (;;);
}


void
abort (void)
{
  printk ("Something aborted - spining \n");
  for (;;);

}


static void
do_file (char *l, char *m, char *s)
{
  char *ptr;
  char *ret;

  if (ptr = match (l, m))
    {
      while (*ptr && is_space (*ptr))
        ptr++;
      ret = ptr;
      while (*ptr && !is_space (*ptr))
        ptr++;
      *ptr = 0;

      strcpy (s, ret);
    }

}


static void
do_line (char *l, char *m, char *s)
{
  char *ptr;

  if (ptr = match (l, m))
    {
      while (*ptr && is_space (*ptr))
        ptr++;
      strcpy (s, ptr);
    }

}



static void
parse_line (char *line, char *kernel, char *options, char *ramdisk,
            int *trynetboot, int *forcediskboot)
{

  printk ("config: %s\n", line);

  do_file (line, "kernel", kernel);
  do_line (line, "options", options);
  do_line (line, "append", options);
  do_file (line, "ramdisk", ramdisk);
  do_file (line, "initrd", ramdisk);

  if (trynetboot && match (line, "try-net-boot"))
    {
      (*trynetboot)++;
    }
  if (forcediskboot && match (line, "force-disk-boot"))
    {
      (*forcediskboot)++;
    }

}

void
parse_config (char *config, int len, char *kernel, char *options,
              char *ramdisk, int *trynetboot, int *forcediskboot)
{

  int i;
  int j = 0;

  config[len] = '\0';
  len++;

  for (i = 0; i < len; ++i)
    {
      if ((config[i] == '\n') || (config[i] == '\0'))
        {
          config[i] = '\0';
          parse_line (&config[j], kernel, options, ramdisk, trynetboot,
                      forcediskboot);
          j = i + 1;
        }
    }

  printk ("kernel: %s\n", kernel);
  printk ("options: %s\n", options);
  printk ("ramdisk: %s\n", ramdisk);
  if (trynetboot)
    {
      printk ("trynetboot: %d\n", *trynetboot);
    }
  if (forcediskboot)
    {
      printk ("forcediskboot: %d\n", *forcediskboot);
    }
}


void
pci_scan ()
{
  int fn;
  int devfn;
  uint32_t id;
  int slot;
  int bus;


  for (bus = 0; bus < 8; ++bus)
    {
      for (slot = 0; slot < 0x20; ++slot)
        {
          for (fn = 0; fn < 8; ++fn)
            {
              devfn = (slot << 3) | fn;
              id = conf1_read_32 (bus, devfn, PCI_VENDOR_ID);
              if (id == 0xffffffff)
                continue;
              if (id == 0x00000000)
                continue;
              printk ("%d:%d.%d %04x:%04x\n", bus, slot, fn, id & 0xffff,
                      id >> 16);
            }
        }
    }
}
void
network_boot_failed (void)
{
  sleep (5);
  try_disk_boot (0);
}

void
do_main (void)
{
  printk ("mb_boot_tv: mythic-beasts.com appletv bootloader\n");
  printk ("%s\n", get_version ());

  try_disk_boot (1);
  sleep (5);
  try_network_boot ();
  spin ();
}
