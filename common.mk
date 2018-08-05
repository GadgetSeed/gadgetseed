include $(TARGETCONFIGMAKE)
include $(ARCHCONFIGMAKE)
include $(SYSTEMCONFIGMAKE)

.PHONY: clean distclean

CFLAGS	+= -I$(PRJ_DIR)/include

ifeq ($(COMP_ENABLE_FATFS),YES)
CFLAGS	+= -I$(PRJ_DIR)/$(FATFS_DIR)/source
endif

ifndef KERNEL_DRIVERS	# $gsc カーネル動作に必要なデバイスドライバ(タイマ、シリアル等)
KERNEL_DRIVERS = systick uart
endif

KDOBJS  = ${addprefix drivers/, $(addsuffix .o, $(KERNEL_DRIVERS) $(ARCH_DRIVERS))}
OBJS += $(KDOBJS)

SDOBJS  = ${addprefix systems/$(SYSTEM)/drivers/, $(addsuffix .o, $(SYS_DRIVERS))}
OBJS += $(SDOBJS)
