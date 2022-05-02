include $(PRJ_DIR)/common.mk

TARGET_HAL_VER = "FW.F4.1.27.0"

ifdef STM32CUBE_HAL_ARCH	# $gsc STM32Cubeターゲット指定
CFLAGS	+= -D$(STM32CUBE_HAL_ARCH)
endif

LIB	= arch.a

OBJS	+= \
	start.o \
	initsect.o \
	interrupt.o \
	cpu.o \
	halver.o \
	systems/$(SYSTEM).o

SOBJS   = \
	intfunc.o \
	inthdr.o \
	dispatch.o \
	sections.o

### for STM32Cube HAL Drivers

ifeq ($(OS),Windows_NT)
	HAL_DIR =  C:/$(HOMEPATH)/STM32Cube/Repository/STM32CubeF4
#	HAL_DIR =  C:/$(HOMEPATH)/STM32Cube/Repository/STM32Cube_FW_F4_V1.24.1
else
	HAL_DIR = $(HOME)/STM32Cube/Repository/STM32CubeF4
#	HAL_DIR = $(HOME)/STM32Cube/Repository/STM32Cube_FW_F4_V1.24.1
endif

HAL_VER = `xmllint --xpath "Package/PackDescription/@Release" $(HAL_DIR)/package.xml | sed 's/[^"]*"\([^"]*\)"[^"]*/\1/g'`

HAL_DRIVERS_DIR = $(HAL_DIR)/Drivers/STM32F4xx_HAL_Driver/Src

BSP_OBJS_DIR   = ./bsp_objs

CFLAGS  += -I$(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS  += -I$(HAL_DIR)/Drivers/CMSIS/Include
CFLAGS  += -I$(HAL_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc
ifeq ($(TARGET_SYSTEM_NUCLEO_F411RE),YES)
CFLAGS  += -I$(HAL_DIR)/Projects/STM32F411RE-Nucleo/Templates/Inc
SYSSRC   = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
endif
ifeq ($(TARGET_SYSTEM_32F469IDISCOVERY),YES)
CFLAGS  += -I$(HAL_DIR)/Projects/STM32469I-Discovery/Templates/Inc
#SYSSRC   = $(HAL_DIR)/Projects/STM32469I-Discovery/Templates/Src/system_stm32f4xx.c
#CFLAGS  += -I$(HAL_DIR)/Projects/STM32469I-Discovery/Applications/Display/LCD_Paint/Inc
#SYSSRC   =   $(HAL_DIR)/Projects/STM32469I-Discovery/Applications/Display/LCD_Paint/Src/system_stm32f4xx.c
#CFLAGS  += -I$(HAL_DIR)/Projects/STM32469I-Discovery/Applications/Display/LCD_DSI_ImagesSlider/Inc
#SYSSRC   =   $(HAL_DIR)/Projects/STM32469I-Discovery/Applications/Display/LCD_DSI_ImagesSlider/Src/system_stm32f4xx.c
#CFLAGS  += -I$(HAL_DIR)/Projects/STM32469I-Discovery/Applications/Display/LCD_AnimatedPictureFromSDCard/Inc
SYSSRC   =   $(HAL_DIR)/Projects/STM32469I-Discovery/Applications/Display/LCD_AnimatedPictureFromSDCard/Src/system_stm32f4xx.c
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/STM32469I-Discovery
BSP_SRCS_DIR = $(HAL_DIR)/Drivers/BSP/STM32469I-Discovery
BSP_SRCS = $(BSP_SRCS_DIR)/stm32469i_discovery.c \
	   $(BSP_SRCS_DIR)/stm32469i_discovery_sdram.c \
	   $(BSP_SRCS_DIR)/stm32469i_discovery_lcd.c \
	   $(BSP_SRCS_DIR)/stm32469i_discovery_ts.c \
	   $(BSP_SRCS_DIR)/stm32469i_discovery_sd.c
BSP_OBJS = $(addprefix $(BSP_OBJS_DIR)/, $(notdir $(BSP_SRCS:.c=.o)))
endif

OBJS     += $(BSP_OBJS)

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
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_iwdg.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_sdram.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_ll_fmc.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_ltdc.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_ltdc_ex.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_dma2d.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_dsi.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_pwr_ex.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_hal_sd.c \
	  $(HAL_DRIVERS_DIR)/stm32f4xx_ll_sdmmc.c

OBJDIR   = ./hal_objs
SYSOBJ   = $(addprefix $(OBJDIR)/, $(notdir $(SYSSRC:.c=.o)))
HALOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(HALSRCS:.c=.o)))

ifeq ($(TARGET_SYSTEM_32F469IDISCOVERY),YES)
LCD_SRC  = $(HAL_DIR)/Drivers/BSP/Components/otm8009a/otm8009a.c
LCD_OBJ  = $(addprefix $(OBJDIR)/, $(notdir $(LCD_SRC:.c=.o)))
OBJS     += $(LCD_OBJ)
LCD2_SRC  = $(HAL_DIR)/Drivers/BSP/Components/nt35510/nt35510.c
LCD2_OBJ  = $(addprefix $(OBJDIR)/, $(notdir $(LCD2_SRC:.c=.o)))
OBJS     += $(LCD2_OBJ)

TSD_SRC  = $(HAL_DIR)/Drivers/BSP/Components/ft6x06/ft6x06.c
TSD_OBJ  = $(addprefix $(OBJDIR)/, $(notdir $(TSD_SRC:.c=.o)))
OBJS     += $(TSD_OBJ)
endif

all:	$(LIB)

$(SYSOBJ): $(SYSSRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(HAL_DRIVERS_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BSP_OBJS_DIR)/%.o: $(BSP_SRCS_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(TSD_OBJ): $(TSD_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LCD_OBJ): $(LCD_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LCD2_OBJ): $(LCD2_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.s.o:
	$(CC) $(CFLAGS) -o $@ -c $<

inthdr.s: mkinthdr.awk
	awk -f mkinthdr.awk > $@

halver.c:
	echo $(TARGET_HAL_VER)
	echo "const char hal_ver[] = \"$(HAL_VER)\";" > halver.c
	@if grep $(TARGET_HAL_VER) halver.c ; then echo "HAL version OK"; else echo "\033[0;31mwarning: HAL version is unmatch\n" \
		"Target HAL Version      :" $(TARGET_HAL_VER) "\n" \
		"Development environment :" $(HAL_VER) "\033[0;39m" ; fi

$(LIB): $(OBJS) $(SOBJS) $(SYSOBJ) $(HALOBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS) $(SOBJS) $(SYSOBJ) $(HALOBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r inthdr.[os] $(OBJS) $(SOBJS) $(SYSOBJ) $(HALOBJS) $(LIB) $(DEPEND) halver.c

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
