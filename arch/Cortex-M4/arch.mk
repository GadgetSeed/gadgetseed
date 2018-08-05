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
CFLAGS	= -g -Wall -mthumb -mcpu=cortex-m4 -mthumb-interwork \
	-mfpu=fpv4-sp-d16 -mfloat-abi=softfp -fipa-sra -mtune=cortex-m4

CFLAGS	+= -O2
#CFLAGS	+= -O0

LDFLAGS += -Wl,-static -Wl,--gc-sections -nostartfiles
LDFLAGS += -o $*.elf -T$(LDSCRIPT) -Wl,-Map=$*.map

LD_LIBS  = -lm
