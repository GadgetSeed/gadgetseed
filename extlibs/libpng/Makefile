include $(PRJ_DIR)/common.mk

LIBPNG = libpng-1.6.34
LIBPNGARC = libpng-1.6.34.tar.xz
LIBPNGURI = http://prdownloads.sourceforge.net/libpng/
LIBPNGDIR = $(LIBPNG)
LIBPNGSRC = $(LIBPNGDIR)/png.c

LIB	= libpng.a

CFLAGS += -I. -iquote $(LIBPNGDIR) -iquote $(PRJ_DIR)/$(LIBZLIB_DIR)/zlib-1.2.11

PNGOBJS	= \
	$(LIBPNGDIR)/png.o \
	$(LIBPNGDIR)/pngerror.o \
	$(LIBPNGDIR)/pngget.o \
	$(LIBPNGDIR)/pngmem.o \
	$(LIBPNGDIR)/pngpread.o \
	$(LIBPNGDIR)/pngread.o \
	$(LIBPNGDIR)/pngrio.o \
	$(LIBPNGDIR)/pngrtran.o \
	$(LIBPNGDIR)/pngrutil.o \
	$(LIBPNGDIR)/pngset.o \
	$(LIBPNGDIR)/pngtrans.o \
	$(LIBPNGDIR)/pngwio.o \
	$(LIBPNGDIR)/pngwrite.o \
	$(LIBPNGDIR)/pngwtran.o \
	$(LIBPNGDIR)/pngwutil.o

OBJS =	$(PNGOBJS)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(LIBPNGSRC): $(LIBPNGARC)
	tar xvfJ $(LIBPNGARC)
	touch $(LIBPNGSRC)

$(LIBPNGARC):
	wget $(LIBPNGURI)/$(LIBPNGARC)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	rm -f -r $(LIBPNGARC) $(LIBPNGDIR) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
