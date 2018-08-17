ifeq ($(DEV_ENABLE_RTC),YES)
ARCH_DRIVERS += rtc
endif

ifeq ($(DEV_ENABLE_LED),YES)
ARCH_DRIVERS += 32f469i-disc_led
endif

ifeq ($(COMP_ENABLE_GRAPHICS),YES)
ARCH_DRIVERS += 32f469i-disc_lcd
DRIVERS += framebuf
ifeq ($(DEV_ENABLE_GRCONSOLE),YES)
DRIVERS += grconsole
endif
endif

ifeq ($(DEV_ENABLE_TOUCHSENSOR),YES)
ARCH_DRIVERS += 32f469i-disc_ts
endif

ifeq ($(DEV_ENABLE_STORAGE),YES)
ARCH_DRIVERS += 32f469i-disc_sd
endif

ifeq ($(DEV_ENABLE_NULL),YES)
DRIVERS += null
endif
