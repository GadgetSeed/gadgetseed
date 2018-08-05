include $(PRJ_DIR)/common.mk

FFURL = http://elm-chan.org/fsw/ff/arc
FFARC = ff13b.zip
FFSRCDIR = source
FFSRC  = $(FFSRCDIR)/ff.c
FFOBJS = $(FFSRCDIR)/ff.o $(FFSRCDIR)/ffunicode.o

LIB	= libfatfs.a

OBJS    = $(FFOBJS)

$(OBJS): $(SRCS)

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(FFSRC): $(FFARC)
	unzip -o $(FFARC)
	cp $(FFSRCDIR)/ffconf.h $(FFSRCDIR)/ffconf.h.org
	patch --binary -p0 < ffconf.h.patch
	touch $(FFSRC)

$(FFARC):
	wget $(FFURL)/$(FFARC)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	rm -f -r $(LIB) $(FFARC) $(FFSRCDIR) $(DEPEND) documents

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)