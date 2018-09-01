/** @file
 * @brief	lwipopts.h for GadgetSeed
 *
 * @date	2017.08.15
 * @date	2012.06.09
 * @author	Takashi SHUDO
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "sysconfig.h"
#include "random.h"
#include "tkprintf.h"

#if 1

#define LWIP_SO_SNDTIMEO		1
#define LWIP_SO_RCVTIMEO		1

/* Prevent having to link sys_arch.c (we don't test the API layers in unit tests) */
#define NO_SYS				0
#define SYS_LIGHTWEIGHT_PROT		0
#define LWIP_NETCONN			1
#define LWIP_SOCKET			1
#define LWIP_COMPAT_SOCKETS		0

/* Enable DHCP to test it, disable UDP checksum to easier inject packets */
#define LWIP_DHCP			1

/* Minimal changes to opt.h required for tcp unit tests: */
#define MEM_SIZE			16000
#define TCP_SND_BUF			(12 * TCP_MSS)
#define TCP_WND				(10 * TCP_MSS)
#define TCP_SND_QUEUELEN		(4 * TCP_SND_BUF/TCP_MSS)
#define MEMP_NUM_TCP_SEG		TCP_SND_QUEUELEN
#define LWIP_WND_SCALE			1
#define TCP_RCV_SCALE			0
#define PBUF_POOL_SIZE			16 /* pbuf tests need ~200KByte */

/* Enable IGMP and MDNS for MDNS tests */
#define LWIP_IGMP			1
#define LWIP_MDNS_RESPONDER		1
#define LWIP_NUM_NETIF_CLIENT_DATA	(LWIP_MDNS_RESPONDER)

/* Minimal changes to opt.h required for etharp unit tests: */
#define ETHARP_SUPPORT_STATIC_ENTRIES	1

#define	LWIP_ETHERNET			1
#define LWIP_ARP			1
#define LWIP_ICMP			1
#define LWIP_DNS			1
#define LWIP_NETIF_STATUS_CALLBACK	1
#define LWIP_COMPAT_MUTEX_ALLOWED	1
#define LWIP_COMPAT_MUTEX		0
#define LWIP_PROVIDE_ERRNO		1
#define LWIP_TIMEVAL_PRIVATE		1

#define LWIP_TCPIP_TIMEOUT		1

#define TCPIP_THREAD_STACKSIZE	(1024*8)

#define MEM_ALIGNMENT			4

#ifdef GSC_ETHERDEV_HARDWARE_CHECKSUM
#define LWIP_CHECKSUM_CTRL_PER_NETIF	1
#define CHECKSUM_GEN_ICMP		0
#endif

//#define LWIP_DEBUG_ON
#ifdef LWIP_DEBUG_ON
#define LWIP_DEBUG	1
#define MEM_DEBUG	LWIP_DBG_ON
#define MEMP_DEBUG	LWIP_DBG_ON
#define SYS_DEBUG	LWIP_DBG_ON
#define PBUF_DEBUG	LWIP_DBG_ON
#define API_LIB_DEBUG	LWIP_DBG_ON
#define API_MSG_DEBUG	LWIP_DBG_ON
#define ETHARP_DEBUG	LWIP_DBG_ON
#define TCPIP_DEBUG	LWIP_DBG_ON
#define NETIF_DEBUG	LWIP_DBG_ON
#define SOCKETS_DEBUG	LWIP_DBG_ON
#define INET_DEBUG	LWIP_DBG_ON
#define IP_DEBUG	LWIP_DBG_ON
#define IP_REASS_DEBUG	LWIP_DBG_ON
#define RAW_DEBUG	LWIP_DBG_ON
#define ICMP_DEBUG	LWIP_DBG_ON
#define UDP_DEBUG	LWIP_DBG_ON
#define TCP_DEBUG	LWIP_DBG_ON
#define TCP_INPUT_DEBUG	LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG	LWIP_DBG_ON
#define TCP_RTO_DEBUG	LWIP_DBG_ON
#define TCP_CWND_DEBUG	LWIP_DBG_ON
#define TCP_WND_DEBUG	LWIP_DBG_ON
#define TCP_FR_DEBUG	LWIP_DBG_ON
#define TCP_QLEN_DEBUG	LWIP_DBG_ON
#define TCP_RST_DEBUG	LWIP_DBG_ON
#define HTTPD_DEBUG	LWIP_DBG_ON
#endif

#define LWIP_PLATFORM_DIAG(x)	do {tkprintf x;} while(0)
#define LWIP_PLATFORM_ASSERT(x) do {tkprintf("Assertion \"%s\" failed at line %d in %s\n", \
					     x, __LINE__, __FILE__);} while(0)

#define LWIP_RAND() ((u32_t)genrand_int32())

#else // #if 0/1

#define TCPIP_THREAD_STACKSIZE	(1024*8)

#define LWIP_PROVIDE_ERRNO

#define LWIP_DBG_MIN_LEVEL 0
#define LWIP_COMPAT_SOCKETS 1
#define TAPIF_DEBUG LWIP_DBG_ON
#define TUNIF_DEBUG LWIP_DBG_OFF
#define UNIXIF_DEBUG LWIP_DBG_OFF
#define DELIF_DEBUG LWIP_DBG_OFF
#define SIO_FIFO_DEBUG LWIP_DBG_OFF
#define TCPDUMP_DEBUG LWIP_DBG_ON

#define PPP_DEBUG        LWIP_DBG_OFF
#define MEM_DEBUG        LWIP_DBG_ON
#define MEMP_DEBUG       LWIP_DBG_ON
#define PBUF_DEBUG       LWIP_DBG_ON
#define API_LIB_DEBUG    LWIP_DBG_ON
#define API_MSG_DEBUG    LWIP_DBG_ON
#define TCPIP_DEBUG      LWIP_DBG_ON
#define NETIF_DEBUG      LWIP_DBG_ON
#define SOCKETS_DEBUG    LWIP_DBG_ON
#define DEMO_DEBUG       LWIP_DBG_ON
#define IP_DEBUG         LWIP_DBG_ON
#define IP_REASS_DEBUG   LWIP_DBG_ON
#define RAW_DEBUG        LWIP_DBG_ON
#define ICMP_DEBUG       LWIP_DBG_ON
#define UDP_DEBUG        LWIP_DBG_ON
#define TCP_DEBUG        LWIP_DBG_ON
#define TCP_INPUT_DEBUG  LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG LWIP_DBG_ON
#define TCP_RTO_DEBUG    LWIP_DBG_ON
#define TCP_CWND_DEBUG   LWIP_DBG_ON
#define TCP_WND_DEBUG    LWIP_DBG_ON
#define TCP_FR_DEBUG     LWIP_DBG_ON
#define TCP_QLEN_DEBUG   LWIP_DBG_ON
#define TCP_RST_DEBUG    LWIP_DBG_ON

//extern unsigned char debug_flags;
//#define LWIP_DBG_TYPES_ON debug_flags

#define NO_SYS                     0
#define LWIP_SOCKET                (NO_SYS==0)
#define LWIP_NETCONN               (NO_SYS==0)
#define LWIP_COMPAT_MUTEX_ALLOWED

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
/* MSVC port: intel processors don't need 4-byte alignment,
   but are faster that way! */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE               10240 

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           16
/* MEMP_NUM_RAW_PCB: the number of UDP protocol control blocks. One
   per active RAW "connection". */
#define MEMP_NUM_RAW_PCB        3
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        4
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        5
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 8
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        16
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    3

/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         2
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        10
/* MEMP_NUM_TCPIP_MSG_*: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG_API   16
#define MEMP_NUM_TCPIP_MSG_INPKT 16

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          120

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       128

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
   link level header. */
#define PBUF_LINK_HLEN          16

/** SYS_LIGHTWEIGHT_PROT
 * define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT           1

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP Maximum segment size. */
#define TCP_MSS                 1024

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             2048

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        (4 * TCP_SND_BUF/TCP_MSS)

/* TCP writable space (bytes). This must be less than or equal
   to TCP_SND_BUF. It is the amount of space which must be
   available in the tcp snd_buf for select to return writable */
#define TCP_SNDLOWAT		(TCP_SND_BUF/2)

/* TCP receive window. */
#define TCP_WND                 8096

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              12

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           4

/* ---------- ARP options ---------- */
#define LWIP_ARP                1
#define ARP_TABLE_SIZE          10
#define ARP_QUEUEING            1

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD              1


/* IP reassembly and segmentation.These are orthogonal even
 * if they both deal with IP fragments */
#define IP_REASSEMBLY     1
#define IP_REASS_MAX_PBUFS      10
#define MEMP_NUM_REASSDATA      10
#define IP_FRAG           1

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. */
#define LWIP_DHCP               0

/* 1 if you want to do an ARP check on the offered address
   (recommended if using DHCP). */
#define DHCP_DOES_ARP_CHECK     (LWIP_DHCP)

/* ---------- AUTOIP options ------- */
#define LWIP_AUTOIP             0

/* ---------- SNMP options ---------- */
/** @todo SNMP is experimental for now
    @note UDP must be available for SNMP transport */
#ifndef LWIP_SNMP
#define LWIP_SNMP               0
#endif

#ifndef SNMP_PRIVATE_MIB
#define SNMP_PRIVATE_MIB        0
#endif

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255

/* ---------- RAW options ---------- */
#define LWIP_RAW                1
#define RAW_TTL                 255

/* ---------- Statistics options ---------- */
/* individual STATS options can be turned off by defining them to 0
 * (e.g #define TCP_STATS 0). All of them are turned off if LWIP_STATS
 * is 0
 * */

#define LWIP_STATS	1

/* ---------- PPP options ---------- */

#define PPP_SUPPORT      0      /* Set > 0 for PPP */

#if PPP_SUPPORT > 0

#define NUM_PPP 1           /* Max PPP sessions. */


/* Select modules to enable.  Ideally these would be set in the makefile but
 * we're limited by the command line length so you need to modify the settings
 * in this file.
 */
#define PAP_SUPPORT      1      /* Set > 0 for PAP. */
#define CHAP_SUPPORT     1      /* Set > 0 for CHAP. */
#define MSCHAP_SUPPORT   0      /* Set > 0 for MSCHAP (NOT FUNCTIONAL!) */
#define CBCP_SUPPORT     0      /* Set > 0 for CBCP (NOT FUNCTIONAL!) */
#define CCP_SUPPORT      0      /* Set > 0 for CCP (NOT FUNCTIONAL!) */
#define VJ_SUPPORT       1      /* Set > 0 for VJ header compression. */
#define MD5_SUPPORT      1      /* Set > 0 for MD5 (see also CHAP) */


/*
 * Timeouts.
 */
#define FSM_DEFTIMEOUT		6	/* Timeout time in seconds */
#define FSM_DEFMAXTERMREQS	2	/* Maximum Terminate-Request transmissions */
#define FSM_DEFMAXCONFREQS	10	/* Maximum Configure-Request transmissions */
#define FSM_DEFMAXNAKLOOPS	5	/* Maximum number of nak loops */

#define UPAP_DEFTIMEOUT		6	/* Timeout (seconds) for retransmitting req */
#define UPAP_DEFREQTIME		30	/* Time to wait for auth-req from peer */

#define CHAP_DEFTIMEOUT		6	/* Timeout time in seconds */
#define CHAP_DEFTRANSMITS	10	/* max # times to send challenge */


/* Interval in seconds between keepalive echo requests, 0 to disable. */
#if 1
#define LCP_ECHOINTERVAL 0
#else
#define LCP_ECHOINTERVAL 10
#endif

/* Number of unanswered echo requests before failure. */
#define LCP_MAXECHOFAILS 3

/* Max Xmit idle time (in jiffies) before resend flag char. */
#define PPP_MAXIDLEFLAG 100

/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *    limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *    of living in lcp.h)
 */
#define PPP_MTU     1500     /* Default MTU (size of Info field) */
#if 0
#define PPP_MAXMTU  65535 - (PPP_HDRLEN + PPP_FCSLEN)
#else
#define PPP_MAXMTU  1500 /* Largest MTU we allow */
#endif
#define PPP_MINMTU  64
#define PPP_MRU     1500     /* default MRU = max length of info field */
#define PPP_MAXMRU  1500     /* Largest MRU we allow */
#define PPP_DEFMRU	296		/* Try for this */
#define PPP_MINMRU	128		/* No MRUs below this */


#define MAXNAMELEN      256     /* max length of hostname or name for auth */
#define MAXSECRETLEN    256     /* max length of password or secret */

#endif /* PPP_SUPPORT > 0 */

#define LWIP_DNS			0
#define LWIP_TIMEVAL_PRIVATE		1

#define LWIP_PLATFORM_DIAG(x)	do {eprintf x;} while(0)
#define LWIP_PLATFORM_ASSERT(x) do {eprintf("Assertion \"%s\" failed at line %d in %s\n", \
					    x, __LINE__, __FILE__);} while(0)

#define LWIP_RAND() ((u32_t)genrand_int32())

#endif

#endif /* __LWIPOPTS_H__ */
