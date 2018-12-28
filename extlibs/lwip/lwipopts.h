/** @file
    @brief	lwipopts.h for GadgetSeed

    @date	2017.08.15
    @date	2012.06.09
    @author	Takashi SHUDO
*/
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "sysconfig.h"
#include "random.h"
#include "tkprintf.h"
#include "task/task.h"

#define TCPIP_THREAD_PRIO	TASK_PRIORITY_NETWORK

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  0

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (10*1024)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           10
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        6
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 5
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        8
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    10


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          8

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       1524

/* ---------- IPv4 options ---------- */
#define LWIP_IPV4                1

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (4*TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
  as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */

#define TCP_SND_QUEUELEN        (2* TCP_SND_BUF/TCP_MSS)

/* TCP receive window. */
//#define TCP_WND                 (2*TCP_MSS)
#define TCP_WND                 (3*TCP_MSS)


/* ---------- ICMP options ---------- */
#define LWIP_ICMP                       1


/* ---------- DHCP options ---------- */
#define LWIP_DHCP               1


/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255


/* ---------- Statistics options ---------- */
#define LWIP_STATS 0

/* ---------- link callback options ---------- */
/* LWIP_NETIF_LINK_CALLBACK==1: Support a callback function from an interface
 * whenever the link changes (i.e., link down)
 */
#define LWIP_NETIF_LINK_CALLBACK        1

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/*
The STM32F7xx allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#ifdef GSC_ETHERDEV_HARDWARE_CHECKSUM
#define CHECKSUM_BY_HARDWARE
#endif

#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     1

#define LWIP_PROVIDE_ERRNO		1
#define LWIP_DNS			1

#define LWIP_SO_SNDTIMEO		1
#define LWIP_SO_RCVTIMEO		1

#define LWIP_SOCKET_SET_ERRNO		1
#define set_errno	lwip_set_errno
extern void lwip_set_errno(int err);

//#define LWIP_DEBUG_ON
#ifdef LWIP_DEBUG_ON
#define LWIP_DEBUG	1
#define NETIF_DEBUG	LWIP_DBG_ON
#define MEM_DEBUG	LWIP_DBG_ON
#define MEMP_DEBUG	LWIP_DBG_ON
#define SYS_DEBUG	LWIP_DBG_ON
#define PBUF_DEBUG	LWIP_DBG_ON
#define API_LIB_DEBUG	LWIP_DBG_ON
#define API_MSG_DEBUG	LWIP_DBG_ON
#define ETHARP_DEBUG	LWIP_DBG_ON
#define TCPIP_DEBUG	LWIP_DBG_ON
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

#ifndef GSC_KERNEL_MESSAGEOUT_LOG
#define LWIP_PLATFORM_DIAG(x)	do {tkprintf x;} while(0)
#else
#define LWIP_PLATFORM_DIAG(x)	do {gslogn x;} while(0)
#endif
#define LWIP_PLATFORM_ASSERT(x) do {tkprintf("Assertion \"%s\" failed at line %d in %s\n", \
					     x, __LINE__, __FILE__);} while(0)

#define LWIP_RAND() ((u32_t)genrand_int32())

#endif /* __LWIPOPTS_H__ */
