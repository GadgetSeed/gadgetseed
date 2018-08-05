include $(PRJ_DIR)/common.mk

ifdef STM32CUBE_HAL_ARCH	# $gsc STM32Cubeターゲット指定
CFLAGS	+= -D$(STM32CUBE_HAL_ARCH)
endif

LIB	= arch.a

OBJS	+= \
	start.o \
	initsect.o \
	interrupt.o \
	cpu.o \
	systems/$(SYSTEM).o

SOBJS   = \
	intfunc.o \
	inthdr.o \
	dispatch.o \
	sections.o

### for STM32Cube HAL Drivers

ifeq ($(OS),Windows_NT)
	HAL_DIR =  C:/$(HOMEPATH)/STM32Cube/Repository/STM32Cube_FW_F4_V1.21.0
else
	HAL_DIR = $(HOME)/STM32Cube/Repository/STM32Cube_FW_F4_V1.21.0
endif

HAL_DRIVERS_DIR = $(HAL_DIR)/Drivers/STM32F4xx_HAL_Driver/Src

CFLAGS  += -I$(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS  += -I$(HAL_DIR)/Drivers/CMSIS/Include
CFLAGS  += -I$(HAL_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc
CFLAGS  += -I$(HAL_DIR)/Projects/STM32F411RE-Nucleo/Templates/Inc

SYSSRC   = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

HALSRCS = $(HAL_DRIVERS_DIR)/stm32f4xx_hal.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_cortex.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_adc.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_adc_ex.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_rcc.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_rcc_ex.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_gpio.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_uart.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_i2c.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_rtc.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_dma.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_tim.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_tim_ex.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_iwdg.c

OBJDIR   = ./hal_objs
SYSOBJ   = $(addprefix $(OBJDIR)/, $(notdir $(SYSSRC:.c=.o)))
HALOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(HALSRCS:.c=.o)))

all:	$(LIB)

$(SYSOBJ): $(SYSSRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(HAL_DRIVERS_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.s.o:
	$(CC) $(CFLAGS) -o $@ -c $<

inthdr.s: mkinthdr.awk
	awk -f mkinthdr.awk > $@

$(LIB): $(OBJS) $(SOBJS) $(SYSOBJ) $(HALOBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS) $(SOBJS) $(SYSOBJ) $(HALOBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r inthdr.[os] $(OBJS) $(SOBJS) $(SYSOBJ) $(HALOBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
