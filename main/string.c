/*
 * string.c:
 *
 * Copyright (c) 2007 Mythic-Beasts LTD ( http://www.mythic-beasts.com ),
 * All rights reserved. Written by James McKenzie <macmini@madingley.org>.
 *
 */

static char rcsid[] = "$Id: string.c,v 1.5 2007/04/26 11:03:07 james Exp $";

/*
 * $Log: string.c,v $
 * Revision 1.5  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#include "project.h"


int
is_space (char l)
{
  if (l == ' ')
    return 1;
  if (l == '\t')
    return 1;
  if (l == '\n')
    return 1;
  if (l == '\r')
    return 1;
  return 0;
}


char *
match (char *l, char *m)
{
  while (*m)
    {
      if (*(l++) != *(m++))
        return NULL;
    }

  return l;
}


char *
strncpy (char *dst, const char *src, size_t n)
{
  char *ret = dst;

  while (*src && n--)
    {
      *(dst++) = *(src++);
    }

  while (n--)
    *(dst++) = '\0';

  return ret;
}


char *
strcat (char *dst, const char *src)
{
  char *ret = dst;


  while (*dst)
    dst++;
  while (*src)
    *(dst++) = *(src++);
  *dst = '\0';

  return ret;
}

int
strncmp (const char *s1, const char *s2, int n)
{
  while (n--)
    {
      if (*s1 > *s2)
        return 1;
      if (*s1 < *s2)
        return -1;
      if (!*s1)
        break;
      s1++, s2++;
    }
  return 0;
}

char *
strchr (const char *s, int c)
{
  while (*s)
    {
      if (c == *s)
        return (char *) s;
      s++;
    }

  return NULL;
}


char *
strrchr (const char *s, int c)
{
  const char *e = s;
  while (*e)
    e++;

  for (; e != s; e--)
    {
      if (c == *e)
        return (char *) e;
    }

  return NULL;
}


char *
strtok_simple (char *in, char c)
{
  static char *last;
  char *tmp;

  if (in == NULL)
    in = last;

  if (in == NULL)
    return NULL;

  if (*in == c)
    in++;

  tmp = strchr (in, c);
  if (tmp)
    {
      *tmp = '\0';
      last = tmp + 1;
    }
  else
    {
      last = NULL;
    }
  return in;
}
