include $(PRJ_DIR)/common.mk

LIB	= liblwip.a

LWIP = lwip-2.1.2

#LWIPURL = http://download.savannah.nongnu.org/releases/lwip/
LWIPURL = http://mirror.lihnidos.org/GNU/savannah/lwip

LWIPARC = $(LWIP).zip

LWIPSRCDIR = $(LWIP)/src

LWIPSRCS = $(LWIPSRCDIR)/api/netdb.c

CFLAGS	+= -I. -I$(LWIP)/src/include -I$(LWIP)/src/include/ipv4

OBJS    = \
	$(LWIPSRCDIR)/api/netdb.o \
	$(LWIPSRCDIR)/core/init.o \
	$(LWIPSRCDIR)/core/def.o \
	$(LWIPSRCDIR)/core/dns.o \
	$(LWIPSRCDIR)/core/inet_chksum.o \
	$(LWIPSRCDIR)/core/ip.o \
	$(LWIPSRCDIR)/core/mem.o \
	$(LWIPSRCDIR)/core/memp.o \
	$(LWIPSRCDIR)/core/netif.o \
	$(LWIPSRCDIR)/core/pbuf.o \
	$(LWIPSRCDIR)/core/raw.o \
	$(LWIPSRCDIR)/core/stats.o \
	$(LWIPSRCDIR)/core/sys.o \
	$(LWIPSRCDIR)/core/tcp.o \
	$(LWIPSRCDIR)/core/tcp_in.o \
	$(LWIPSRCDIR)/core/tcp_out.o \
	$(LWIPSRCDIR)/core/timeouts.o \
	$(LWIPSRCDIR)/core/udp.o \
	$(LWIPSRCDIR)/core/ipv4/autoip.o \
	$(LWIPSRCDIR)/core/ipv4/dhcp.o \
	$(LWIPSRCDIR)/core/ipv4/etharp.o \
	$(LWIPSRCDIR)/core/ipv4/icmp.o \
	$(LWIPSRCDIR)/core/ipv4/igmp.o \
	$(LWIPSRCDIR)/core/ipv4/ip4.o \
	$(LWIPSRCDIR)/core/ipv4/ip4_addr.o \
	$(LWIPSRCDIR)/core/ipv4/ip4_frag.o \
	$(LWIPSRCDIR)/netif/ethernet.o \
	$(LWIPSRCDIR)/netif/lowpan6.o \
	$(LWIPSRCDIR)/netif/slipif.o \
	$(LWIPSRCDIR)/api/api_lib.o \
	$(LWIPSRCDIR)/api/api_msg.o \
	$(LWIPSRCDIR)/api/err.o \
	$(LWIPSRCDIR)/api/sockets.o \
	$(LWIPSRCDIR)/api/netbuf.o \
	$(LWIPSRCDIR)/api/netifapi.o \
	$(LWIPSRCDIR)/api/sockets.o \
	$(LWIPSRCDIR)/api/tcpip.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(LWIPSRCS): $(LWIPARC)
	unzip -o $(LWIPARC)
	patch --binary -p0 < netdb.c.patch

$(LWIPARC):
	wget $(LWIPURL)/$(LWIPARC)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	make clean
	rm -f -r $(LWIPARC) $(LWIP) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
