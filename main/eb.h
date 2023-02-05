/*
 * eb.h:
 *
 * Incorporates code from etherboot
 *
 */

/*
 * $Id: eb.h,v 1.6 2007/04/26 11:03:07 james Exp $
 */

/*
 * $Log: eb.h,v $
 * Revision 1.6  2007/04/26 11:03:07  james
 * *** empty log message ***
 *
 */

#ifndef __EB_H__
#define __EB_H__

#include "project.h"
#include "io.h"

#define CONFIG_PCI

#define inb in_8
#define inw in_16
#define inl in_32

#define outb out_8
#define outw out_16
#define outl out_32

#define insw in_string_16

#include "mem.h"




typedef uint64_t sector_t;

#define printf printk

#define __unused

#define __aligned __attribute__((aligned(16)))
#define PACKED __attribute__((packed))

#define bswap_16 __bswap_16


#define K_ESC           '\033'
#define K_EOF           '\04'   /* Ctrl-D */
#define K_INTR          '\03'   /* Ctrl-C */


#define KERNEL_BUF      (bootp_data.bootp_reply.bp_file)

#define DOWNLOAD_PROTO_TFTP

#define VERSION_MAJOR 1
#define VERSION_MINOR 1


#ifndef MAX_TFTP_RETRIES
#define MAX_TFTP_RETRIES        20
#endif

#ifndef MAX_BOOTP_RETRIES
#define MAX_BOOTP_RETRIES       4
#endif

#define MAX_BOOTP_EXTLEN        (ETH_MAX_MTU-sizeof(struct bootpip_t))

#ifndef MAX_ARP_RETRIES
#define MAX_ARP_RETRIES         20
#endif

#ifndef MAX_RPC_RETRIES
#define MAX_RPC_RETRIES         20
#endif

/* Link configuration time in tenths of a second */
#ifndef VALID_LINK_TIMEOUT
#define VALID_LINK_TIMEOUT      100 /* 10.0 seconds */
#endif

/* Inter-packet retry in ticks */
#ifndef TIMEOUT
#define TIMEOUT                 (10*TICKS_PER_SEC)
#endif

#ifndef BOOTP_TIMEOUT
#define BOOTP_TIMEOUT           (2*TICKS_PER_SEC)
#endif

/* Max interval between IGMP packets */
#define IGMP_INTERVAL                   (10*TICKS_PER_SEC)
#define IGMPv1_ROUTER_PRESENT_TIMEOUT   (400*TICKS_PER_SEC)

/* These settings have sense only if compiled with -DCONGESTED */
/* total retransmission timeout in ticks */
#define TFTP_TIMEOUT            (30*TICKS_PER_SEC)
/* packet retransmission timeout in ticks */
#ifdef CONGESTED
#define TFTP_REXMT              (3*TICKS_PER_SEC)
#else
#define TFTP_REXMT              TIMEOUT
#endif


/* Helper macros used to identify when DHCP options are valid/invalid in/outside of encapsulation */
/* helpers for decoding RFC1533_VENDOR encapsulated options */
#ifdef PXE_DHCP_STRICT
#define PXE_ENCAP_OPT in_pxe_encapsulated_options == 1 &&
#define PXE_NONENCAP_OPT in_pxe_encapsulated_options == 0 &&
#else
#define PXE_ENCAP_OPT
#define PXE_NONENCAP_OPT
#endif /* PXE_DHCP_STRICT */

#define NON_ENCAP_OPT in_encapsulated_options == 0 && PXE_NONENCAP_OPT
#ifdef ALLOW_ONLY_ENCAPSULATED
#define ENCAP_OPT in_encapsulated_options == 1 &&
#else
#define ENCAP_OPT PXE_NONENCAP_OPT
#endif

#define RAND_MAX 2147483647L


#include "eb/dev.h"
#include "eb/nic.h"
#include "eb/pci.h"
#include "eb/byteswap.h"
#include "eb/little_bswap.h"
#include "eb/in.h"
#include "eb/udp.h"
#include "eb/tcp.h"
#include "eb/tftp.h"
#include "eb/bootp.h"
#include "eb/elf.h"
#include "eb/if_ether.h"
#include "eb/if_arp.h"


typedef int (*reply_t) (int ival, void *ptr, unsigned short ptype,
                        struct iphdr * ip, struct udphdr * udp,
                        struct tcphdr * tcp);
extern int await_reply P ((reply_t reply, int ival, void *ptr, long timeout));


enum
{
  ARP_CLIENT, ARP_SERVER, ARP_GATEWAY,
  MAX_ARP
};

struct rom_info
{
  unsigned short rom_segment;
  unsigned short rom_length;
};
struct arptable_t
{
  in_addr ipaddr;
  uint8_t node[6];
} PACKED;

extern long rfc2131_sleep_interval (long base, int exp);


extern struct arptable_t arptable[MAX_ARP];
extern char as_main_program;
extern unsigned long strtoul (const char *p, const char **endp, int base);

extern int tftp (const char *name,
                 int (*fnc) (unsigned char *, unsigned int, unsigned int,
                             int));
#endif /* __EB_H__ */
