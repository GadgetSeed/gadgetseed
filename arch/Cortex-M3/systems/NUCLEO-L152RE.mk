ifeq ($(DEV_ENABLE_LED),YES)
ARCH_DRIVERS += stm32l1xx_nucleo_led
endif

ifeq ($(DEV_ENABLE_NULL),YES)
DRIVERS += null
endif
