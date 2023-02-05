/*
 * partition.h:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

/*
 * $Id: partition.h,v 1.3 2007/04/26 11:03:07 james Exp $
 */

/*
 * $Log: partition.h,v $
 * Revision 1.3  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#ifndef __PARTITION_H__
#define __PARTITION_H__

struct partition
{
  uint64_t start;
  uint64_t size;
};
#endif /* __PARTITION_H__ */
