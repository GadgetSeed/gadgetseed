include $(PRJ_DIR)/common.mk

TXT2BITMAP = $(PRJ_DIR)/tools/txt2bitmap

LIB	= filemanager.a

OBJS	= filemanager.o \
	  filelist_data.o \
	  filelist_view.o \
	  clock_view.o \
	  exec_file_opration.o

ifdef DEV_ENABLE_AUDIO
OBJS	+= volume_view.o
endif

ifeq ($(APPLICATION),soundplay)
OBJS	+= mini_musicplay_view.o
endif

ifdef FONTS_ENABLE_FONT_JISKAN24
OBJS	+= file_icon.o
else
OBJS	+= file_icon_16.o
endif

CFLAGS += -I$(PRJ_DIR)/$(UILIB_DIR)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

file_icon_16.c: file_icon_16.txt
	$(TXT2BITMAP) $*.txt > $*.c

file_icon.c: file_icon.txt
	$(TXT2BITMAP) $*.txt > $*.c

clean:
	rm -f -r file_icon.c file_icon_16.c $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
