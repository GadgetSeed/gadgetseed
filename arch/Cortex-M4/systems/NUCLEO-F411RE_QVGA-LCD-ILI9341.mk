include $(PRJ_DIR)/arch/Cortex-M4/systems/NUCLEO-F411RE.mk

ifeq ($(DEV_ENABLE_STORAGE),YES)
ARCH_DRIVERS += spi stm32f4xx_nucleo_mmc
DRIVERS += spi_mmc
endif

ifeq ($(COMP_ENABLE_GRAPHICS),YES)
ARCH_DRIVERS += gpio_lcd_8bit
DRIVERS += ili9341_lcd
endif
