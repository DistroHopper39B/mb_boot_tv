/*
 * diskboot.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: diskboot.c,v 1.5 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: diskboot.c,v $
 * Revision 1.5  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"


#define BOOTCONF "/mb_boot_tv.conf"


void *
disk_read_file (char *name, int *size)
{
  uint32_t fd;
  uint32_t sz;
  int ret;
  void *buf;
  static char namebuf[1024]; //ext2 code trashes names


  strcpy(namebuf,name);

  if (ext2fs_open (namebuf, &fd))
    return NULL;

  sz = (uint32_t) ext2fs_len (fd);

  buf = bl_malloc (sz);
  ret = ext2fs_read (fd, buf, &sz);
  ext2fs_close (fd);

  if (size)
    *size = sz;

  return buf;
}

static void
diskboot (char *config, int len, int allowed_trynetwork)
{
  char options[1024] = "";
  char kernel[1024] = "";
  char ramdisk[1024] = "";

  void *ramdisk_buf = NULL, *kernel_buf = NULL;
  int kernel_len, ramdisk_len;
  int trynetboot = 0;

  parse_config (config, len, kernel, options, ramdisk, &trynetboot, NULL);

  if (allowed_trynetwork && trynetboot)
    {
      printk ("Trying a network boot");

      ext2fs_fs_close ();
      disk_close ();

      try_network_boot ();      //doesn't return - if fails calls us here again
      return;
    }

  if (!kernel[0])
    {
      printk ("no kernel - returning\n");
      return;
    }

  kernel_buf = disk_read_file (kernel, &kernel_len);

  if (!kernel_buf)
    {
      printk ("kernel load failed -returning\n");
      return;
    }

  if (ramdisk[0])
    {

      ramdisk_buf = disk_read_file (ramdisk, &ramdisk_len);

      if (!ramdisk_buf)
        {
          printk ("ramdisk load failed - returning\n");
          return;
        }

    }

  printk ("files loaded - booting\n");


  ext2fs_fs_close ();
  disk_close ();


  sleep (2);
  boot_kernel (kernel_buf, kernel_len, ramdisk_buf, ramdisk_len, options);
  printk ("booting failed\n");


}


#define NP 16

void
try_disk_boot (int trynetwork)
{
  struct partition p[NP];
  int n = NP;

  int i;
  char *config;
  int len;


  if (disk_open (0))
    {
      printk ("Failed to open disk\n");
      return;
    }

  if (efi_find_partitions (p, &n))
    {
      disk_close ();
      printk ("Failed to find an EFI GPT\n");
      return;
    }

  for (i = 0; i < n; ++i)
    {

      partition_set (&p[i]);

      if (ext2fs_fs_open ())
        {
          printk ("part%d: no ext2fs filesystem\n", i);
          continue;
        }

      config = disk_read_file (BOOTCONF, &len);

      if (!config)
        {
          config = disk_read_file ("/boot" BOOTCONF, &len);
          if (config)
            {
              printk ("part%d: /boot" BOOTCONF " found\n", i);
            }
        }
      else
        {
          printk ("part%d: " BOOTCONF " found\n", i);
        }

      if (!config)
        {
          printk ("part%d: can't open " BOOTCONF " or /boot" BOOTCONF "\n",
                  i);
          ext2fs_fs_close ();
          continue;
        }


      diskboot (config, len, trynetwork);


      ext2fs_fs_close ();
      break;
    }


  disk_close ();

}
