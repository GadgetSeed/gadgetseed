include $(PRJ_DIR)/common.mk

LIB	= musicplay.a

OBJS	= musicplay.o filesearch.o filelist.o play_view.o list_view.o volume_view.o spectrum_view.o

CFLAGS += -I$(PRJ_DIR)/$(UILIB_DIR)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
