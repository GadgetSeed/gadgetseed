include $(PRJ_DIR)/common.mk

CFLAGS += -I$(PRJ_DIR)/$(FATFS_DIR)/source

LIB	= fs.a

OBJS	= \
	  fs.o \
	  file.o \
	  storage.o \
	  batch.o

ifdef COMP_ENABLE_PIPEFS
OBJS	+= pipefs.o
endif

ifdef COMP_ENABLE_GSFFS
OBJS	+= gsffs.o
endif

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

.PHONY: distclean
distclean:
	rm -f -r $(FFARC) $(FFSRCDIR) $(DEPEND) doc

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
