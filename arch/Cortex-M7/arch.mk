CROSS	= arm-none-eabi-

AS	= $(CROSS)as
CC	= $(CROSS)gcc
AR	= $(CROSS)ar
RANLIB	= $(CROSS)ranlib
LD	= $(CROSS)gcc
OBJCOPY	= $(CROSS)objcopy
STRIP	= $(CROSS)strip
OBJDUMP	= $(CROSS)objdump

#CFLAGS	= -g -O2 -Wall -Werror
ifeq ($(STM32CUBE_HAL_ARCH),STM32F746xx)
CFLAGS	= -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra \
	-mfpu=fpv5-sp-d16 -mfloat-abi=hard
else
CFLAGS	= -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra \
	-mfpu=fpv5-d16 -mfloat-abi=hard
endif
#	-mfpu=fpv4-sp-d16 -mfloat-abi=softfp

CFLAGS	+= -O2
#CFLAGS	+= -O0

LDFLAGS += -Wl,-static -Wl,--gc-sections -nostartfiles
LDFLAGS += -o $*.elf -T$(LDSCRIPT) -Wl,-Map=$*.map

LD_LIBS  = -lm
