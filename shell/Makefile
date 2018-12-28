include $(PRJ_DIR)/common.mk

LIB	= shell.a

OBJS	= shell_task.o \
	  shell.o \
	  lineedit.o \
	  history.o \
	  com_mem.o \
	  com_device.o \
	  com_system.o \
	  com_task.o

CFLAGS += -I$(PRJ_DIR)/shell

ifeq ($(KERNEL_MESSAGEOUT_LOG),YES)
	OBJS += com_log.o
endif

ifeq ($(COMP_ENABLE_GRAPHICS),YES)
	OBJS += com_graphics.o
ifeq ($(LIB_ENABLE_PICOJPEG),YES)
	CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/picojpeg
endif
ifeq ($(LIB_ENABLE_LIBPNG),YES)
	CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/libpng
endif
endif

ifeq ($(DEV_ENABLE_I2C),YES)
	OBJS += com_i2c.o
endif

ifeq ($(COMP_ENABLE_FATFS),YES)
	OBJS += com_file.o file_operation.o
endif

ifeq ($(COMP_ENABLE_TCPIP),YES)
	CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/lwip
	CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/lwip/lwip-$(LWIP_VER)/src/include
	OBJS += com_net.o
endif


.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
