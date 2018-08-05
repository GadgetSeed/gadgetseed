ifeq ($(DEV_ENABLE_ADC),YES)
ARCH_DRIVERS += adc
endif

ifeq ($(DEV_ENABLE_I2C),YES)
ARCH_DRIVERS += i2c
endif

ifeq ($(DEV_ENABLE_RTC),YES)
ARCH_DRIVERS += rtc
endif

ifeq ($(DEV_ENABLE_STORAGE),YES)
ARCH_DRIVERS += stm32f7xxx-disc_sdmmc
endif

ifeq ($(COMP_ENABLE_GRAPHICS),YES)
ARCH_DRIVERS += stm32f746g-disc_lcd
DRIVERS += framebuf
ifeq ($(DEV_ENABLE_GRCONSOLE),YES)
DRIVERS += grconsole
endif
endif

ifeq ($(DEV_ENABLE_AUDIO),YES)
ARCH_DRIVERS += stm32f7xxx-disc_audio
endif

ifeq ($(COMP_ENABLE_TCPIP),YES)
ARCH_DRIVERS += stm32f7xxx-disc_ether
endif

ifeq ($(DEV_ENABLE_TOUCHSENSOR),YES)
ARCH_DRIVERS += stm32f7xxx-disc_ts
endif

ifeq ($(DEV_ENABLE_BUZZER),YES)
ARCH_DRIVERS += tim12_buzzer
endif

ifeq ($(DEV_ENABLE_NULL),YES)
DRIVERS += null
endif
