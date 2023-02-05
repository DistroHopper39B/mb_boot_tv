/*
 * netboot.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: netboot.c,v 1.4 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: netboot.c,v $
 * Revision 1.4  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"



void
dhcp_callback (char *name)
{
  int len;
  int i;
  int j = 0;
  char options[1024] = "";
  char kernel[1024] = "";
  char ramdisk[1024] = "";

  void *ramdisk_buf, *kernel_buf;
  int kernel_len, ramdisk_len;

  int forcediskboot = 0;
  char *config;

  printk("netboot: tftping %s\n",name);

  config  = network_load_file (name, &len);

  printk ("netboot: config at %p for %d bytes\n", config,len);

  if (!config || (len < 0))
    return;

  parse_config (config, len, kernel, options, ramdisk, NULL, &forcediskboot);

  if (forcediskboot)
    {
      printk ("disk boot forced\n");
      return;
    }

  if (!kernel[0])
    {
      printk ("no kernel - returning\n");
      return;
    }

  kernel_buf = network_load_file (kernel, &kernel_len);

  if (!kernel_buf)
    {
      printk ("kernel load failed -returning\n");
      return;
    }


  if (ramdisk[0])
    {

      ramdisk_buf = network_load_file (ramdisk, &ramdisk_len);

      if (!ramdisk_buf)
        {
          printk ("ramdisk load failed - returning\n");
          return;
        }

    }

  printk ("files loaded - booting\n");

  network_shutdown ();
  sleep (2);
  boot_kernel (kernel_buf, kernel_len, ramdisk_buf, ramdisk_len, options);
  printk ("booting failed\n");

}



void
try_network_boot (void)
{
  static int tried_network_boot = 0;

  if (tried_network_boot)
    {
      printk ("Tried a network boot already\n");
      spin ();
    }

  tried_network_boot++;

  network_init ();
}
