include $(PRJ_DIR)/common.mk

LIBZLIB = zlib-1.2.11
LIBZLIBARC = zlib-1.2.11.tar.xz
LIBZLIBURI = https://zlib.net
LIBZLIBDIR = $(LIBZLIB)
LIBZLIBSRC = $(LIBZLIBDIR)/gzlib.c

LIB	= libzlib.a

CFLAGS += -w -I. -I$(LIBZLIBDIR)

ZLIBOBJS	= \
	$(LIBZLIBDIR)/gzlib.o \
	$(LIBZLIBDIR)/adler32.o \
	$(LIBZLIBDIR)/compress.o \
	$(LIBZLIBDIR)/crc32.o \
	$(LIBZLIBDIR)/deflate.o \
	$(LIBZLIBDIR)/gzclose.o \
	$(LIBZLIBDIR)/gzread.o \
	$(LIBZLIBDIR)/gzwrite.o \
	$(LIBZLIBDIR)/infback.o \
	$(LIBZLIBDIR)/inffast.o \
	$(LIBZLIBDIR)/inflate.o \
	$(LIBZLIBDIR)/inftrees.o \
	$(LIBZLIBDIR)/trees.o \
	$(LIBZLIBDIR)/uncompr.o \
	$(LIBZLIBDIR)/zutil.o

OBJS = $(ZLIBOBJS)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(LIBZLIBSRC): $(LIBZLIBARC)
	tar xvfJ $(LIBZLIBARC)
	touch $(LIBZLIBSRC)

$(LIBZLIBARC):
	wget $(LIBZLIBURI)/$(LIBZLIBARC)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	rm -f -r $(LIBZLIBARC) $(LIBZLIBDIR) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
