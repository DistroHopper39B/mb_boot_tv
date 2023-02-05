/*
 * hexdump.c:
 *
 * code from etherboot
 *
 */

static char rcsid[] = "$Id: hexdump.c,v 1.3 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: hexdump.c,v $
 * Revision 1.3  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"


/* Produce a paged hex dump of the specified data and length */
void
hex_dump (const unsigned char *data, const unsigned int len)
{
  unsigned int index;
  for (index = 0; index < len; index++)
    {
      if ((index % 16) == 0)
        {
          printk ("\n");
        }
#if 0
      if ((index % 368) == 352)
        {
          more ();
        }
#endif
      if ((index % 16) == 0)
        {
          printk ("%05x :", index);
        }
      printk (" %02x", data[index]);
    }
  printk ("\n");
}
