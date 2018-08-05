include $(PRJ_DIR)/common.mk

PICOJPEG = picojpeg-master
PICOJPEGARC = master.zip
PICOJPEGURI = https://github.com/richgel999/picojpeg/archive/$(PICOJPEGARC)
PICOJPEGDIR = $(PICOJPEG)
PICOJPEGSRC = $(PICOJPEGDIR)/picojpeg.c

LIB	= libpicojpeg.a

CFLAGS += -w -DFPM_DEFAULT -DHAVE_CONFIG_H -I. -I picojpeg-master

JPEGBJS	= \
	$(PICOJPEGDIR)/picojpeg.o

OBJS = $(JPEGBJS)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(PICOJPEGSRC): $(PICOJPEGARC)
	unzip $(PICOJPEGARC)
	patch --binary -p0 < picojpeg.c.patch

$(PICOJPEGARC):
	wget $(PICOJPEGURI)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	rm -f -r $(PICOJPEGARC) $(PICOJPEGDIR) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
