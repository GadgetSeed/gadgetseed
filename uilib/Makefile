include $(PRJ_DIR)/common.mk

TXT2BITMAP = $(PRJ_DIR)/tools/txt2bitmap

LIB	= uilib.a

OBJS	= ui_button.o \
	  ui_selectlist.o \
	  ui_scrollbar.o \
	  ui_seekbar.o \
	  ui_edittext.o \
	  ui_switch.o

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
