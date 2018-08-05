include $(PRJ_DIR)/common.mk

CFLAGS += -I$(PRJ_DIR)/$(FATFS_DIR)/source

LIB	= fs.a

OBJS	= \
	  file.o \
	  diskio.o \
	  fattime.o \
	  storage.o \
	  batch.o

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
