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
	HAL_DIR =  C:/$(HOMEPATH)/STM32Cube/Repository/STM32Cube_FW_F7_V1.11.0
else
	HAL_DIR = $(HOME)/STM32Cube/Repository/STM32Cube_FW_F7_V1.11.0
endif

HAL_SRCS_DIR = $(HAL_DIR)/Drivers/STM32F7xx_HAL_Driver/Src

CFLAGS  += -I$(PRJ_DIR)/arch/$(ARCH)
CFLAGS  += -I$(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F7xx/Include
CFLAGS  += -I$(HAL_DIR)/Drivers/CMSIS/Include
CFLAGS  += -I$(HAL_DIR)/Drivers/STM32F7xx_HAL_Driver/Inc

ifeq ($(TARGET_SYSTEM_STM32F769IDISCOVERY),YES)
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/STM32F769I-Discovery
#CFLAGS  += -I$(HAL_DIR)/Projects/STM32F769I-Discovery/Templates/Inc	# for stm32f7xx_hal_conf.h
#CFLAGS  += -I$(HAL_DIR)/Projects/STM32F769I-Discovery/Demonstration/Config	# for stm32f7xx_hal_conf.h
#CFLAGS  += -I$(HAL_DIR)/Projects/STM32F769I-Discovery/Applications/LwIP/LwIP_HTTP_Server_Netconn_RTOS/Inc	# for stm32f7xx_hal_conf.h

CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/ft6x06
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/otm8009a
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/wm8994
endif

ifeq ($(TARGET_SYSTEM_STM32F746GDISCOVERY),YES)
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/STM32746G-Discovery
#CFLAGS  += -I$(HAL_DIR)/Projects/STM32746G-Discovery/Templates/Inc	# for stm32f7xx_hal_conf.h

CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/rk043fn48h
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/ft5336
endif

HAL_OBJS_DIR   = ./hal_objs

HAL_SRCS = $(HAL_SRCS_DIR)/stm32f7xx_hal.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_cortex.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_adc.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_adc_ex.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_rcc.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_rcc_ex.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_gpio.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_uart.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_i2c.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_i2c_ex.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_rtc.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_dma.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_tim.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_tim_ex.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_pwr_ex.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_dsi.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_ltdc.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_ltdc_ex.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_dma2d.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_sdram.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_sd.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_ll_sdmmc.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_ll_fmc.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_eth.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_sai.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_dfsdm.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_iwdg.c

HAL_OBJS  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(HAL_SRCS:.c=.o)))
OBJS     += $(HAL_OBJS)

BSP_OBJS_DIR   = ./bsp_objs

ifeq ($(TARGET_SYSTEM_STM32F769IDISCOVERY),YES)
BSP_SRCS_DIR = $(HAL_DIR)/Drivers/BSP/STM32F769I-Discovery
BSP_SRCS = $(BSP_SRCS_DIR)/stm32f769i_discovery.c \
	   $(BSP_SRCS_DIR)/stm32f769i_discovery_sdram.c \
	   $(BSP_SRCS_DIR)/stm32f769i_discovery_lcd.c \
	   $(BSP_SRCS_DIR)/stm32f769i_discovery_ts.c \
	   $(BSP_SRCS_DIR)/stm32f769i_discovery_audio.c \
	   $(BSP_SRCS_DIR)/stm32f769i_discovery_sd.c
BSP_OBJS = $(addprefix $(BSP_OBJS_DIR)/, $(notdir $(BSP_SRCS:.c=.o)))
endif

ifeq ($(TARGET_SYSTEM_STM32F746GDISCOVERY),YES)
BSP_SRCS_DIR = $(HAL_DIR)/Drivers/BSP/STM32746G-Discovery
BSP_SRCS = $(BSP_SRCS_DIR)/stm32746g_discovery.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_sdram.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_lcd.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_ts.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_audio.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_sd.c
BSP_OBJS = $(addprefix $(BSP_OBJS_DIR)/, $(notdir $(BSP_SRCS:.c=.o)))
endif

OBJS     += $(BSP_OBJS)

SYS_SRC  = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
SYS_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(SYS_SRC:.c=.o)))
OBJS     += $(SYS_OBJ)

ifeq ($(TARGET_SYSTEM_STM32F769IDISCOVERY),YES)
TSD_SRC  = $(HAL_DIR)/Drivers/BSP/Components/ft6x06/ft6x06.c
TSD_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(TSD_SRC:.c=.o)))
OBJS     += $(TSD_OBJ)
endif

ifeq ($(TARGET_SYSTEM_STM32F746GDISCOVERY),YES)
TSD_SRC  = $(HAL_DIR)/Drivers/BSP/Components/ft5336/ft5336.c
TSD_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(TSD_SRC:.c=.o)))
OBJS     += $(TSD_OBJ)
endif

LCD_SRC  = $(HAL_DIR)/Drivers/BSP/Components/otm8009a/otm8009a.c
LCD_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(LCD_SRC:.c=.o)))
OBJS     += $(LCD_OBJ)

ACD_SRC  = $(HAL_DIR)/Drivers/BSP/Components/wm8994/wm8994.c
ACD_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(ACD_SRC:.c=.o)))
OBJS     += $(ACD_OBJ)

all:	$(LIB)

$(HAL_OBJS_DIR)/%.o: $(HAL_SRCS_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(BSP_OBJS_DIR)/%.o: $(BSP_SRCS_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(SYS_OBJ): $(SYS_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TSD_OBJ): $(TSD_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LCD_OBJ): $(LCD_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ACD_OBJ): $(ACD_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.s.o:
	$(CC) $(CFLAGS) -o $@ -c $<

inthdr.s: mkinthdr.awk
	awk -f mkinthdr.awk > $@

$(LIB): $(OBJS) $(SOBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS) $(SOBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r inthdr.[os] $(OBJS) $(SOBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
