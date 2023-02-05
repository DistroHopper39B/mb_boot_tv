/*
 * sleep.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: sleep.c,v 1.2 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: sleep.c,v $
 * Revision 1.2  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"

void
msleep (int s)
{
  s += mseconds ();

  while (mseconds () < s);

}


void
sleep (int s)
{
  msleep (s * 1000);
}
