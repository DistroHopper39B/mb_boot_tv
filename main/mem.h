/*
 * mem.h:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

/*
 * $Id: mem.h,v 1.3 2007/04/26 11:03:07 james Exp $
 */

/*
 * $Log: mem.h,v $
 * Revision 1.3  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#ifndef __MEM_H__
#define __MEM_H__

/* Amount of relocation etherboot is experiencing */
extern unsigned long virt_offset;

/* Don't require identity mapped physical memory,
 * osloader.c is the only valid user at the moment.
 */
static inline unsigned long
virt_to_phys (volatile const void *virt_addr)
{
  return ((unsigned long) virt_addr) + virt_offset;
}

static inline void *
phys_to_virt (unsigned long phys_addr)
{
  return (void *) (phys_addr - virt_offset);
}

/* virt_to_bus converts an addresss inside of etherboot [_start, _end]
 * into a memory access cards can use.
 */
#define virt_to_bus virt_to_phys


/* bus_to_virt reverses virt_to_bus, the address must be output
 * from virt_to_bus to be valid.  This function does not work on
 * all bus addresses.
 */
#define bus_to_virt phys_to_virt

/* ioremap converts a random 32bit bus address into something
 * etherboot can access.
 */
static inline void *
ioremap (unsigned long bus_addr, unsigned long length)
{
  return bus_to_virt (bus_addr);
}

/* iounmap cleans up anything ioremap had to setup */
static inline void
iounmap (void *virt_addr)
{
  return;
}
#endif /* __MEM_H__ */
