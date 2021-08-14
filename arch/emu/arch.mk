AS	= as
CC	= gcc
AR	= ar
RANLIB	= ranlib
LD	= gcc
OBJCOPY	= objcopy
STRIP	= strip

CFLAGS = -g -O2 -Wall -DGTK_DISABLE_DEPRECATED `pkg-config --cflags --libs gtk+-2.0`
UNAME = ${shell uname}
ifeq ($(UNAME),Darwin)
# for MacOSX
CFLAGS += -D_XOPEN_SOURCE
endif
