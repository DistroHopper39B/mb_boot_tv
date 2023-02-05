/*
 * time.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: time.c,v 1.2 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: time.c,v $
 * Revision 1.2  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"


static uint32_t l0, h0;
static int set = 0;

int
mseconds (void)
{
  uint32_t l, q;
  uint32_t h;

  int r;

  if (!set)
    {
      asm __volatile__ ("rdtsc\n":"=a" (l0), "=d" (h0):);
      set++;
    }

  asm __volatile__ ("rdtsc\n":"=a" (l), "=d" (h):);


  r = (h - h0) << 12;

  q = l;
  q >>= 20;
  r += q;

  q = l0;
  q >>= 20;
  r -= q;


//printk("%x:%08x %x:%08x %d\n",h0,l0,h,l,r);

  return r;
}

int
seconds (void)
{
  return mseconds () / 1000;
}
