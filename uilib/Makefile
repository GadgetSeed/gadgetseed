include $(PRJ_DIR)/common.mk

LIB	= uilib.a

OBJS	= ui_button.o \
	  ui_selectlist.o \
	  ui_scrollbar.o \
	  ui_seekbar.o \
	  ui_statictext.o \
	  ui_edittext.o \
	  ui_switch.o \
	  ui_progressbar.o

ifeq ($(ENABLE_UI_DIALOG_TIMESET),YES)	# $gsc 時刻設定ダイアログを使用する
	OBJS += dialogbox/timeset/timeset.o
	CFLAGS += -I$(PRJ_DIR)/uilib
	OBJS += dialogbox/tenkey/tenkey.o
	CFLAGS += -I$(PRJ_DIR)/uilib/dialogbox/tenkey
endif

ifeq ($(COMP_ENABLE_TCPIP),YES)
	CFLAGS += -I$(PRJ_DIR)/$(LWIP_DIR) -I$(PRJ_DIR)/$(LWIP_DIR)/lwip-$(LWIP_VER)/src/include
endif

ifeq ($(ENABLE_UI_DIALOG_NETSET),YES)	# $gsc ネットワーク設定ダイアログを使用する
	OBJS += dialogbox/netset/netset.o
	CFLAGS += -I$(PRJ_DIR)/uilib
	OBJS += dialogbox/tenkey/tenkey.o
	CFLAGS += -I$(PRJ_DIR)/uilib/dialogbox/tenkey
endif

ifeq ($(COMP_ENABLE_FATFS),YES)
	OBJS += appsetting.o
endif

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
