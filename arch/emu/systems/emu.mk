ifeq ($(DEV_ENABLE_RTC),YES)
ARCH_DRIVERS += vrtc
endif

ifeq ($(DEV_ENABLE_STORAGE),YES)
ARCH_DRIVERS += vmmc
endif

ifeq ($(COMP_ENABLE_GRAPHICS),YES)
ARCH_DRIVERS += lcdwindow vlcd
DRIVERS += framebuf
ifeq ($(DEV_ENABLE_GRCONSOLE),YES)
DRIVERS += grconsole
endif
endif

ifeq ($(DEV_ENABLE_AUDIO),YES)
ARCH_DRIVERS += vaudio
endif

ifeq ($(COMP_ENABLE_TCPIP),YES)
ARCH_DRIVERS += vether
endif

ifeq ($(DEV_ENABLE_NULL),YES)
DRIVERS += null
endif
