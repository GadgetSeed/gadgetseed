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
	halver.o \
	systems/$(SYSTEM).o

SOBJS   = \
	intfunc.o \
	inthdr.o \
	dispatch.o \
	sections.o

### for STM32Cube HAL Drivers

ifeq ($(OS),Windows_NT)
	HAL_DIR =  C:/$(HOMEPATH)/STM32Cube/Repository/$(STM32CUBE_HAL_DIR)
#	HAL_DIR =  C:/$(HOMEPATH)/STM32Cube/Repository/STM32Cube_FW_F7_V1.15.0
else
	HAL_DIR = $(HOME)/STM32Cube/Repository/$(STM32CUBE_HAL_DIR)
#	HAL_DIR = $(HOME)/STM32Cube/Repository/STM32Cube_FW_F7_V1.15.0
endif

HAL_VER = `xmllint --xpath "Package/PackDescription/@Release" $(HAL_DIR)/package.xml | sed 's/[^"]*"\([^"]*\)"[^"]*/\1/g'`

ifeq ($(STM32CUBE_HAL_DIR),STM32CubeF7)	# $gsc STM32 HAL インストールディレクトリ指定
	HAL_SRCS_DIR = $(HAL_DIR)/Drivers/STM32F7xx_HAL_Driver/Src
	HAL_INCS_DIR = $(HAL_DIR)/Drivers/STM32F7xx_HAL_Driver/Inc
	DEV_INCS_DIR = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F7xx/Include
	TARGET_HAL_VER = "FW.F7.1.16.0"
endif
ifeq ($(STM32CUBE_HAL_DIR),STM32CubeH7)
	HAL_SRCS_DIR = $(HAL_DIR)/Drivers/STM32H7xx_HAL_Driver/Src
	HAL_INCS_DIR = $(HAL_DIR)/Drivers/STM32H7xx_HAL_Driver/Inc
	DEV_INCS_DIR = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32H7xx/Include
	TARGET_HAL_VER = "FW.H7.1.9.0"
endif

CFLAGS  += -I$(PRJ_DIR)/arch/$(ARCH)
CFLAGS  += -I$(HAL_DIR)/Drivers/CMSIS/Include
CFLAGS  += -I$(HAL_INCS_DIR)
CFLAGS  += -I$(DEV_INCS_DIR)

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

ifeq ($(TARGET_SYSTEM_STM32H747IDISCOVERY),YES)
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/STM32H747I-DISCO
CFLAGS  += -I$(HAL_DIR)/Projects/STM32H747I-DISCO/Templates/BootCM7_CM4Gated/Common/Inc
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/Common
CFLAGS  += -I$(HAL_DIR)/Projects/STM32H747I-DISCO/Demonstrations/STemWin/Modules/audio_player/Addons/PDM/Inc
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/mt25tl01g
CFLAGS  += -I$(HAL_DIR)/Drivers/BSP/Components/lan8742
CFLAGS  += -DCORE_CM7
endif

LWIP	= $(PRJ_DIR)/$(LWIP_DIR)/lwip-$(LWIP_VER)
CFLAGS	+= -I$(PRJ_DIR)/$(LWIP_DIR) -I$(LWIP)/src/include -I$(LWIP)/src/include/ipv4

HAL_OBJS_DIR   = ./hal_objs

ifneq ($(TARGET_SYSTEM_STM32H747IDISCOVERY),YES)
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
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_qspi.c \
	  $(HAL_SRCS_DIR)/stm32f7xx_hal_iwdg.c
else
HAL_SRCS = $(HAL_SRCS_DIR)/stm32h7xx_hal.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_cortex.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_rcc.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_rcc_ex.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_gpio.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_uart.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_uart_ex.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_i2c.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_i2c_ex.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_rtc.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_pwr.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_pwr_ex.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_dsi.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_ltdc.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_ltdc_ex.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_dma2d.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_sdram.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_sd.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_ll_sdmmc.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_ll_delayblock.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_ll_fmc.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_sai.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_dma.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_mdma.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_qspi.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_exti.c \
	  $(HAL_SRCS_DIR)/stm32h7xx_hal_iwdg.c
#	  $(HAL_SRCS_DIR)/stm32h7xx_hal_eth.c
endif

HAL_OBJS  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(HAL_SRCS:.c=.o)))
OBJS     += $(HAL_OBJS)

BSP_OBJS_DIR   = ./bsp_objs

ifeq ($(TARGET_SYSTEM_STM32F769IDISCOVERY),YES)
BSP_SRCS_DIR = $(HAL_DIR)/Drivers/BSP/STM32F769I-Discovery
BSP_SRCS = $(BSP_SRCS_DIR)/stm32f769i_discovery.c \
	   $(BSP_SRCS_DIR)/stm32f769i_discovery_qspi.c \
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
	   $(BSP_SRCS_DIR)/stm32746g_discovery_qspi.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_sdram.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_lcd.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_ts.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_audio.c \
	   $(BSP_SRCS_DIR)/stm32746g_discovery_sd.c
BSP_OBJS = $(addprefix $(BSP_OBJS_DIR)/, $(notdir $(BSP_SRCS:.c=.o)))
endif

ifeq ($(TARGET_SYSTEM_STM32H747IDISCOVERY),YES)
BSP_SRCS_DIR = $(HAL_DIR)/Drivers/BSP/STM32H747I-DISCO
BSP_SRCS = $(BSP_SRCS_DIR)/stm32h747i_discovery.c \
	   $(BSP_SRCS_DIR)/stm32h747i_discovery_qspi.c \
	   $(BSP_SRCS_DIR)/stm32h747i_discovery_sdram.c \
	   $(BSP_SRCS_DIR)/stm32h747i_discovery_lcd.c \
	   $(BSP_SRCS_DIR)/stm32h747i_discovery_ts.c \
	   $(BSP_SRCS_DIR)/stm32h747i_discovery_audio.c \
	   $(BSP_SRCS_DIR)/stm32h747i_discovery_sd.c \
	   $(BSP_SRCS_DIR)/stm32h747i_discovery_bus.c
BSP_OBJS = $(addprefix $(BSP_OBJS_DIR)/, $(notdir $(BSP_SRCS:.c=.o)))
endif

OBJS     += $(BSP_OBJS)

ifeq ($(TARGET_SYSTEM_STM32H747IDISCOVERY),YES)
#SYS_SRC  = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c
SYS_SRC  = $(HAL_DIR)/Projects/STM32H747I-DISCO/Templates/BootCM7_CM4Gated_RAM/Common/Src/system_stm32h7xx.c
else
SYS_SRC  = $(HAL_DIR)/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
endif
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

ifeq ($(TARGET_SYSTEM_STM32H747IDISCOVERY),YES)
TSD_SRC  = $(HAL_DIR)/Drivers/BSP/Components/ft6x06/ft6x06.c
TSD_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(TSD_SRC:.c=.o)))
OBJS     += $(TSD_OBJ)

PHY_SRC  = $(HAL_DIR)/Drivers/BSP/Components/lan8742/lan8742.c
PHY_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(PHY_SRC:.c=.o)))
OBJS     += $(PHY_OBJ)

TSDR_SRC  = $(HAL_DIR)/Drivers/BSP/Components/ft6x06/ft6x06_reg.c
TSDR_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(TSDR_SRC:.c=.o)))
OBJS     += $(TSDR_OBJ)

QSPIROM_SRC  = $(HAL_DIR)/Drivers/BSP/Components/mt25tl01g/mt25tl01g.c
QSPIROM_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(QSPIROM_SRC:.c=.o)))
OBJS     += $(QSPIROM_OBJ)

SDRAM_SRC  = $(HAL_DIR)/Drivers/BSP/Components/is42s32800j/is42s32800j.c
SDRAM_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(SDRAM_SRC:.c=.o)))
OBJS     += $(SDRAM_OBJ)

SDRAM_SRC  = $(HAL_DIR)/Drivers/BSP/Components/is42s32800j/is42s32800j.c
SDRAM_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(SDRAM_SRC:.c=.o)))
OBJS     += $(SDRAM_OBJ)

DSI_SRC  = $(HAL_DIR)/Drivers/BSP/Components/adv7533/adv7533.c
DSI_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(DSI_SRC:.c=.o)))
OBJS     += $(DSI_OBJ)

DSIR_SRC  = $(HAL_DIR)/Drivers/BSP/Components/adv7533/adv7533_reg.c
DSIR_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(DSIR_SRC:.c=.o)))
OBJS     += $(DSIR_OBJ)

LCDR_SRC  = $(HAL_DIR)/Drivers/BSP/Components/otm8009a/otm8009a_reg.c
LCDR_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(LCDR_SRC:.c=.o)))
OBJS     += $(LCDR_OBJ)

ACDR_SRC  = $(HAL_DIR)/Drivers/BSP/Components/wm8994/wm8994_reg.c
ACDR_OBJ  = $(addprefix $(HAL_OBJS_DIR)/, $(notdir $(ACDR_SRC:.c=.o)))
OBJS     += $(ACDR_OBJ)
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

$(TSDR_OBJ): $(TSDR_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LCD_OBJ): $(LCD_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LCDR_OBJ): $(LCDR_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ACD_OBJ): $(ACD_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ACDR_OBJ): $(ACDR_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(QSPIROM_OBJ): $(QSPIROM_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(SDRAM_OBJ): $(SDRAM_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(DSI_OBJ): $(DSI_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(DSIR_OBJ): $(DSIR_SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(PHY_OBJ): $(PHY_SRC)
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

$(LIB): $(OBJS) $(SOBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS) $(SOBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r inthdr.[os] $(OBJS) $(SOBJS) $(LIB) $(DEPEND) halver.c

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
