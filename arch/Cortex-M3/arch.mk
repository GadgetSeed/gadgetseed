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
CFLAGS	= -g -Wall -mthumb -mcpu=cortex-m3 -mthumb-interwork \
	-fipa-sra -mtune=cortex-m3 \
	-DLONGLONGSWAP

CFLAGS	+= -O2
#CFLAGS	+= -O0

LDFLAGS += -Wl,-static -Wl,--gc-sections -nostartfiles
LDFLAGS += -o $*.elf -T$(LDSCRIPT) -Wl,-Map=$*.map

LD_LIBS  = -lm
