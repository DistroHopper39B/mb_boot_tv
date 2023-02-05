/*
 * eb-pci_drivers.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: eb-pci_drivers.c,v 1.3 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: eb-pci_drivers.c,v $
 * Revision 1.3  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "eb.h"

extern struct pci_driver rtl8139_driver;
extern struct pci_driver ide_driver;

const struct pci_driver *pci_drivers_p[] = {
  &rtl8139_driver,
  &ide_driver
};

const struct pci_driver **pci_drivers_p_end =
  pci_drivers_p + (sizeof (pci_drivers_p) / sizeof (struct pci_driver *));
