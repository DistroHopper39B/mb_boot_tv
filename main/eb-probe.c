/*
 * eb-probe.c:
 *
 * code from etherboot
 *
 */

static char rcsid[] = "$Id: eb-probe.c,v 1.2 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: eb-probe.c,v $
 * Revision 1.2  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "eb.h"

static const char *driver_name[] = {
  "nic",
  "disk",
  "floppy",
};

int
probe (struct dev *dev)
{
  const char *type_name;
  type_name = "";
  if ((dev->type >= 0) &&
      ((unsigned) dev->type < sizeof (driver_name) / sizeof (driver_name[0])))
    {
      type_name = driver_name[dev->type];
    }
  if (dev->how_probe == PROBE_FIRST)
    {
      dev->to_probe = PROBE_PCI;
      memset (&dev->state, 0, sizeof (dev->state));
    }
  if (dev->to_probe == PROBE_PCI)
    {
      dev->how_probe = pci_probe (dev, type_name);
      if (dev->how_probe == PROBE_FAILED)
        {
          dev->to_probe = PROBE_ISA;
        }
    }
  return dev->how_probe;
}

void
disable (struct dev *dev)
{
  if (dev->disable)
    {
      dev->disable (dev);
      dev->disable = 0;
    }
}
