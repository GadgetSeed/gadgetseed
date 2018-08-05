include $(PRJ_DIR)/common.mk

LIB	= graphics.a

OBJS	= \
	graphics_op.o \
	graphics_object.o \
	graphics.o

ifeq ($(COMP_ENABLE_FONTS),YES)
	OBJS += font.o
endif

ifeq ($(LIB_ENABLE_PICOJPEG),YES)
	OBJS += jpegdec.o
	CFLAGS += -I $(PRJ_DIR)/$(EXTLIBS_DIR)/picojpeg/picojpeg-master
endif

ifeq ($(LIB_ENABLE_LIBPNG),YES)
	OBJS += pngdec.o
	CFLAGS += -I $(PRJ_DIR)/$(EXTLIBS_DIR)/libpng
	CFLAGS += -I $(PRJ_DIR)/$(EXTLIBS_DIR)/libpng/libpng-1.6.34
endif

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(LIB) $(LIBWOF)

$(LIB): $(OBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	make clean

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
