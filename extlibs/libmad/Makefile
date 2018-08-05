include $(PRJ_DIR)/common.mk

LIBMAD = libmad-0.15.1b
LIBMADARC = $(LIBMAD).tar.gz
LIBMADURI = ftp://ftp.mars.org/pub/mpeg/$(LIBMADARC)
LIBMADDIR = $(LIBMAD)
LIBMADSRC = $(LIBMADDIR)/decoder.c

LIB	= libmad.a

CFLAGS += -DFPM_DEFAULT -DHAVE_CONFIG_H -I.

MADOBJS	= \
	$(LIBMADDIR)/decoder.o \
	$(LIBMADDIR)/bit.o \
	$(LIBMADDIR)/fixed.o \
	$(LIBMADDIR)/frame.o \
	$(LIBMADDIR)/huffman.o \
	$(LIBMADDIR)/layer12.o \
	$(LIBMADDIR)/layer3.o \
	$(LIBMADDIR)/stream.o \
	$(LIBMADDIR)/synth.o \
	$(LIBMADDIR)/timer.o \
	$(LIBMADDIR)/version.o

OBJS = $(MADOBJS)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(LIBMADSRC): $(LIBMADARC)
	tar xvfz $(LIBMADARC)
	touch $(LIBMADSRC)

$(LIBMADARC):
	wget $(LIBMADURI)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	rm -f -r $(LIBMADARC) $(LIBMADDIR) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
