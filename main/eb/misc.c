/**************************************************************************
MISC Support Routines
**************************************************************************/

#include "etherboot.h"
#ifdef CONSOLE_BTEXT
#include <btext.h>
#endif
#ifdef CONSOLE_PC_KBD
#include <pc_kbd.h>
#endif


/**************************************************************************
IPCHKSUM - Checksum IP Header
**************************************************************************/
uint16_t ipchksum(const void *data, unsigned long length)
{
	unsigned long sum;
	unsigned long i;
	const uint8_t *ptr;

	/* In the most straight forward way possible,
	 * compute an ip style checksum.
	 */
	sum = 0;
	ptr = data;
	for(i = 0; i < length; i++) {
		unsigned long value;
		value = ptr[i];
		if (i & 1) {
			value <<= 8;
		}
		/* Add the new value */
		sum += value;
		/* Wrap around the carry */
		if (sum > 0xFFFF) {
			sum = (sum + (sum >> 16)) & 0xFFFF;
		}
	}
	return (~cpu_to_le16(sum)) & 0xFFFF;
}

uint16_t add_ipchksums(unsigned long offset, uint16_t sum, uint16_t new)
{
	unsigned long checksum;
	sum = ~sum & 0xFFFF;
	new = ~new & 0xFFFF;
	if (offset & 1) {
		/* byte swap the sum if it came from an odd offset
		 * since the computation is endian independant this
		 * works.
		 */
		new = bswap_16(new);
	}
	checksum = sum + new;
	if (checksum > 0xFFFF) {
		checksum -= 0xFFFF;
	}
	return (~checksum) & 0xFFFF;
}



/**************************************************************************
RANDOM - compute a random number between 0 and 2147483647L or 2147483562?
**************************************************************************/
int32_t random(void)
{
	static int32_t seed = 0;
	int32_t q;
	if (!seed) /* Initialize linear congruential generator */
		seed = currticks() + *(int32_t *)&arptable[ARP_CLIENT].node
		       + ((int16_t *)arptable[ARP_CLIENT].node)[2];
	/* simplified version of the LCG given in Bruce Schneier's
	   "Applied Cryptography" */
	q = seed/53668;
	if ((seed = 40014*(seed-53668*q) - 12211*q) < 0) seed += 2147483563L;
	return seed;
}

/**************************************************************************
POLL INTERRUPTIONS
**************************************************************************/
void poll_interruptions(void)
{
	int ch;
	if ( ! as_main_program ) return;
#if defined(PCBIOS) && defined(POWERSAVE)
	/* Doze for a while (until the next interrupt).  This works
	 * fine, because the timer interrupt (approx. every 50msec)
	 * is there to takes care of the drivers that have interrupts
	 * disabled.  This reduces the power dissipation of a modern
	 * CPU considerably, and also makes Etherboot waiting for user
	 * interaction waste a lot less CPU time in a VMware session.  */
	cpu_nap();
#endif	/* POWERSAVE */
	/* If an interruption has occured restart etherboot */
	if (iskey() && (ch = getchar(), (ch == K_ESC) || (ch == K_EOF) || (ch == K_INTR))) {
		int state = (ch != K_INTR)? -1 : -3;
		flail();
	}
}

/**************************************************************************
SLEEP
**************************************************************************/
#if 0
void sleep(int secs)
{
	unsigned long tmo;

	for (tmo = currticks()+secs*TICKS_PER_SEC; currticks() < tmo; ) {
		poll_interruptions();
	}
}
#endif

/**************************************************************************
INTERRUPTIBLE SLEEP
**************************************************************************/
void interruptible_sleep(int secs)
{
	printf("<sleep>\n");
	return sleep(secs);
}

/**************************************************************************
TWIDDLE
**************************************************************************/
void twiddle(void)
{
#ifdef BAR_PROGRESS
	static int count=0;
	static const char tiddles[]="-\\|/";
	static unsigned long lastticks = 0;
	unsigned long ticks;
#endif
	if ( ! as_main_program ) return;
#ifdef	BAR_PROGRESS
	/* Limit the maximum rate at which characters are printed */
	ticks = currticks();
	if ((lastticks + (TICKS_PER_SEC/18)) > ticks)
		return;
	lastticks = ticks;

	putchar(tiddles[(count++)&3]);
	putchar('\b');
#else
	putchar('.');
#endif	/* BAR_PROGRESS */
}

/**************************************************************************
STRCASECMP (not entirely correct, but this will do for our purposes)
**************************************************************************/
int strcasecmp(const char *a, const char *b)
{
	while (*a && *b && (*a & ~0x20) == (*b & ~0x20)) {a++; b++; }
	return((*a & ~0x20) - (*b & ~0x20));
}

/**************************************************************************
INET_ATON - Convert an ascii x.x.x.x to binary form
**************************************************************************/
int inet_aton(const char *start, in_addr *i)
{
	const char *p = start;
	const char *digits_start;
	unsigned long ip = 0;
	unsigned long val;
	int j;
	for(j = 0; j <= 3; j++) {
		digits_start = p;
		val = strtoul(p, &p, 10);
		if ((p == digits_start) || (val > 255)) return 0;
		if ( ( j < 3 ) && ( *(p++) != '.' ) ) return 0;
		ip = (ip << 8) | val;
	}
	i->s_addr = htonl(ip);
	return p - start;
}


unsigned long strtoul(const char *p, const char **endp, int base)
{
	unsigned long ret = 0;
	if (base != 10) return 0;
	while((*p >= '0') && (*p <= '9')) {
		ret = ret*10 + (*p - '0');
		p++;
	}
	if (endp)
		*endp = p;
	return(ret);
	
}

#define K_RDWR		0x60		/* keyboard data & cmds (read/write) */
#define K_STATUS	0x64		/* keyboard status */
#define K_CMD		0x64		/* keybd ctlr command (write-only) */

#define K_OBUF_FUL	0x01		/* output buffer full */
#define K_IBUF_FUL	0x02		/* input buffer full */

#define KC_CMD_WIN	0xd0		/* read  output port */
#define KC_CMD_WOUT	0xd1		/* write output port */
#define KB_SET_A20	0xdf		/* enable A20,
					   enable output buffer full interrupt
					   enable data line
					   disable clock line */
#define KB_UNSET_A20	0xdd		/* enable A20,
					   enable output buffer full interrupt
					   enable data line
					   disable clock line */

enum { Disable_A20 = 0x2400, Enable_A20 = 0x2401, Query_A20_Status = 0x2402,
	Query_A20_Support = 0x2403 };

#if defined(PCBIOS) && !defined(IBM_L40)
static void empty_8042(void)
{
	unsigned long time;
	char st;

	time = currticks() + TICKS_PER_SEC;	/* max wait of 1 second */
	while ((((st = inb(K_CMD)) & K_OBUF_FUL) ||
	       (st & K_IBUF_FUL)) &&
	       currticks() < time)
		inb(K_RDWR);
}
#endif	/* IBM_L40 */

#if defined(PCBIOS)
/*
 * Gate A20 for high memory
 */
void gateA20_set(void)
{
#warning "gateA20_set should test to see if it is already set"
	if (int15(Enable_A20) == 0) {
		return;
	}
#ifdef	IBM_L40
	outb(0x2, 0x92);
#else	/* IBM_L40 */
	empty_8042();
	outb(KC_CMD_WOUT, K_CMD);
	empty_8042();
	outb(KB_SET_A20, K_RDWR);
	empty_8042();
#endif	/* IBM_L40 */
}
#endif


int last_putchar; // From filo

#if 0
void
putchar(int c)
{
	c &= 0xff;
	last_putchar = c;

	if (c == '\n')
		putchar('\r');
#ifdef	CONSOLE_FIRMWARE
	console_putc(c);
#endif
#ifdef	CONSOLE_DIRECT_VGA
	vga_putc(c);
#endif
#ifdef	CONSOLE_BTEXT
	btext_putc(c);
#endif
#ifdef	CONSOLE_SERIAL
	serial_putc(c);
#endif
}
#endif

/**************************************************************************
GETCHAR - Read the next character from input device WITHOUT ECHO
**************************************************************************/
#if 0
int getchar(void)
{

	int c = 256;

	do {
#if defined(PCBIOS) && defined(POWERSAVE)
		/* Doze for a while (until the next interrupt).  This works
		 * fine, because the keyboard is interrupt-driven, and the
		 * timer interrupt (approx. every 50msec) takes care of the
		 * serial port, which is read by polling.  This reduces the
		 * power dissipation of a modern CPU considerably, and also
		 * makes Etherboot waiting for user interaction waste a lot
		 * less CPU time in a VMware session.  */
		cpu_nap();
#endif	/* POWERSAVE */
#ifdef	CONSOLE_FIRMWARE
		if (console_ischar())
			c = console_getc();
#endif
#ifdef	CONSOLE_SERIAL
		if (serial_ischar())
			c = serial_getc();
#endif
#ifdef CONSOLE_PC_KBD
		if (kbd_ischar())
			c = kbd_getc();
#endif
	} while (c==256);
	if (c == '\r')
		c = '\n';
	return c;
}

int iskey(void)
{
#ifdef	CONSOLE_FIRMWARE
	if (console_ischar())
		return 1;
#endif
#ifdef	CONSOLE_SERIAL
	if (serial_ischar())
		return 1;
#endif
#ifdef CONSOLE_PC_KBD
	if (kbd_ischar())
		return 1;
#endif
	return 0;
}
#endif

#if DEBUG_UTILS

void pause ( void ) {
	printf ( "\nPress a key" );
	getchar();
	printf ( "\r           \r" );
}

void more ( void ) {
	printf ( "---more---" );
	getchar();
	printf ( "\r          \r" );
}

/* Produce a paged hex dump of the specified data and length */
void hex_dump ( const char *data, const unsigned int len ) {
	unsigned int index;
	for ( index = 0; index < len; index++ ) {
		if ( ( index % 16 ) == 0 ) {
			printf ( "\n" );
		}
		if ( ( index % 368 ) == 352 ) {
			more();
		}
		if ( ( index % 16 ) == 0 ) {
			printf ( "%X [%X] : %hX :", data + index,
				 virt_to_phys ( data + index ), index );
		}
		printf ( " %hhX", data[index] );
	}
	printf ( "\n" );
}

#define GUARD_SYMBOL ( ( 'M' << 24 ) | ( 'I' << 16 ) | ( 'N' << 8 ) | 'E' )
/* Fill a region with guard markers.  We use a 4-byte pattern to make
 * it less likely that check_region will find spurious 1-byte regions
 * of non-corruption.
 */
void guard_region ( void *region, size_t len ) {
	uint32_t offset = 0;

	len &= ~0x03;
	for ( offset = 0; offset < len ; offset += 4 ) {
		*((uint32_t *)(region + offset)) = GUARD_SYMBOL;
	}
}

/* Check a region that has been guarded with guard_region() for
 * corruption.
 */
int check_region ( void *region, size_t len ) {
	uint8_t corrupted = 0;
	uint8_t in_corruption = 0;
	uint32_t offset = 0;
	uint32_t test = 0;

	len &= ~0x03;
	for ( offset = 0; offset < len ; offset += 4 ) {
		test = *((uint32_t *)(region + offset)) = GUARD_SYMBOL;
		if ( ( in_corruption == 0 ) &&
		     ( test != GUARD_SYMBOL ) ) {
			/* Start of corruption */
			if ( corrupted == 0 ) {
				corrupted = 1;
				printf ( "Region %#x-%#x (physical %#x-%#x) "
					 "corrupted\n",
					 region, region + len,
					 virt_to_phys ( region ),
					 virt_to_phys ( region + len ) );
			}
			in_corruption = 1;
			printf ( "--- offset %#x ", offset );
		} else if ( ( in_corruption != 0 ) &&
			    ( test == GUARD_SYMBOL ) ) {
			/* End of corruption */
			in_corruption = 0;
			printf ( "to offset %#x", offset );
		}

	}
	if ( in_corruption != 0 ) {
		printf ( "to offset %#x (end of region)\n", len-1 );
	}
	return corrupted;
}

#endif /* DEBUG_UTILS */

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */
