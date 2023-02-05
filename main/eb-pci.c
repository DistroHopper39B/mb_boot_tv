/*
 * eb-pci.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: eb-pci.c,v 1.2 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: eb-pci.c,v $
 * Revision 1.2  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "eb.h"

extern const struct pci_driver *pci_drivers_p[];
extern const struct pci_driver **pci_drivers_p_end;

#include "eb/pci.c"
