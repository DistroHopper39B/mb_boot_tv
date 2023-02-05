/*
 * eb-pci_bios.c:
 *
 * code from etherboot
 *
 */

static char rcsid[] = "$Id: eb-pci_bios.c,v 1.2 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: eb-pci_bios.c,v $
 * Revision 1.2  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "eb.h"

#define  PCIBIOS_SUCCESSFUL                0x00

#define CONFIG_CMD(bus, device_fn, where)   (0x80000000 | (bus << 16) | (device_fn << 8) | (where & ~3))

extern unsigned long
pcibios_bus_base (unsigned int bus)
{
  return 0;
}


int
pcibios_read_config_byte (unsigned int bus, unsigned int device_fn,
                          unsigned int where, uint8_t * value)
{
  outl (CONFIG_CMD (bus, device_fn, where), 0xCF8);
  *value = inb (0xCFC + (where & 3));
  return PCIBIOS_SUCCESSFUL;
}

int
pcibios_read_config_word (unsigned int bus,
                          unsigned int device_fn, unsigned int where,
                          uint16_t * value)
{
  outl (CONFIG_CMD (bus, device_fn, where), 0xCF8);
  *value = inw (0xCFC + (where & 2));
  return PCIBIOS_SUCCESSFUL;
}

int
pcibios_read_config_dword (unsigned int bus, unsigned int device_fn,
                           unsigned int where, uint32_t * value)
{
  outl (CONFIG_CMD (bus, device_fn, where), 0xCF8);
  *value = inl (0xCFC);
  return PCIBIOS_SUCCESSFUL;
}

int
pcibios_write_config_byte (unsigned int bus, unsigned int device_fn,
                           unsigned int where, uint8_t value)
{
  outl (CONFIG_CMD (bus, device_fn, where), 0xCF8);
  outb (value, 0xCFC + (where & 3));
  return PCIBIOS_SUCCESSFUL;
}

int
pcibios_write_config_word (unsigned int bus, unsigned int device_fn,
                           unsigned int where, uint16_t value)
{
  outl (CONFIG_CMD (bus, device_fn, where), 0xCF8);
  outw (value, 0xCFC + (where & 2));
  return PCIBIOS_SUCCESSFUL;
}

int
pcibios_write_config_dword (unsigned int bus, unsigned int device_fn,
                            unsigned int where, uint32_t value)
{
  outl (CONFIG_CMD (bus, device_fn, where), 0xCF8);
  outl (value, 0xCFC);
  return PCIBIOS_SUCCESSFUL;
}
