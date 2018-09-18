-include config.mk

ifeq ($(OS),Windows_NT)
#	CYGWIN_ROOT = C:/gnupack_devel-13.06-2015.11.08
	PRJ_DIR = $(CYGWIN_ROOT)$(PWD)
else
	PRJ_DIR = $(PWD)
endif
export PRJ_DIR

VERSION = $(shell cat VERSION)

ifndef SYSCONF_DIR
SYSCONF_DIR = configs/systems
endif
export SYSCONF_DIR

ifndef CONFIGS_DIR
CONFIGS_DIR = configs
endif
export CONFIGS_DIR

ifndef APPS_DIR
APPS_DIR = apps
endif
export APPS_DIR

TARGETCONFIGMAKE = $(PWD)/targetconfig.mk
export TARGETCONFIGMAKE

ARCHCONFIGMAKE = $(PRJ_DIR)/arch/$(ARCH)/arch.mk
export ARCHCONFIGMAKE

SYSTEMCONFIGMAKE = $(PRJ_DIR)/arch/$(ARCH)/systems/$(SYSTEM_CONFIG).mk
export SYSTEMCONFIGMAKE

EXTLIBS_DIR = extlibs
export EXTLIBS_DIR

-include $(TARGETCONFIGMAKE)
-include $(ARCHCONFIGMAKE)
-include $(SYSTEMCONFIGMAKE)

#
HEADERS_DIR = $(PRJ_DIR)/include
HEADERS = $(HEADERS_DIR)/*.h

ARCHHEADER = $(HEADERS_DIR)/asm.h
SYSCONFHEADER = $(HEADERS_DIR)/sysconfig.h

LDSCRIPT= arch/$(ARCH)/systems/$(SYSTEM).lds

CFLAGS += -I$(HEADERS_DIR)

DEPEND	= .depend

PROGLINK = gadgetseed

ifndef APPNAME	# $gsc 実行ファイルのアプリケーションフィールド名
APPNAME = noname
endif

ifeq ($(ARCH),emu)
PROGRAM = gs-$(ARCH)-$(SYSTEM)-$(APPNAME)-$(VERSION)
else
PROGRAM = gs-$(ARCH)-$(SYSTEM)-$(APPNAME)-$(VERSION).elf
endif

KERNEL_DIR = kernel
KERNEL_LIB = $(KERNEL_DIR)/kernel.a

ARCH_DIR = arch/$(ARCH)
ARCH_LIB = $(ARCH_DIR)/arch.a

DRIVERS_DIR = drivers
DRIVERS_LIB = $(DRIVERS_DIR)/drivers.a

LIBS_DIR = libs
LIBS_LIB = $(LIBS_DIR)/libs.a

TASK_DIR = $(KERNEL_DIR)/task
TASK_LIB = $(TASK_DIR)/task.a

TOOLS_DIR = tools

BDF2FONTSRC = $(TOOLS_DIR)/bdf2fontc
TXT2FONTSRC = $(TOOLS_DIR)/txt2fontc
BMP2TXT = $(TOOLS_DIR)/bmp2txt
TXT2BITMAP = $(TOOLS_DIR)/txt2bitmap


LIBS	+= $(KERNEL_LIB) \
	   $(ARCH_LIB) \
	   $(DRIVERS_LIB) \
	   $(LIBS_LIB)

HEADERS += $(HEADERS_DIR)/task/*.h
LIBS += $(TASK_LIB)

# fs
FATFS_DIR = $(EXTLIBS_DIR)/fatfs
FATFS_LIB = $(FATFS_DIR)/libfatfs.a
ifeq ($(COMP_ENABLE_FATFS),YES)
LIBS_EXT_LIB += $(FATFS_LIB)
endif
export FATFS_DIR
FS_DIR = fs
FS_LIB = $(FS_DIR)/fs.a
ifeq ($(COMP_ENABLE_FATFS),YES)
	LIBS += $(FATFS_LIB)
	LIBS += $(FS_LIB)
endif

# uilib
UILIB_DIR = uilib
export UILIB_DIR
UILIB_LIB= $(UILIB_DIR)/uilib.a
ifeq ($(ENABLE_UILIB),YES)	# $gsc ユーザインタフェースライブラリを有効にする
	LIBS += $(UILIB_LIB)
endif

# random.o
RANDOM_DIR = $(EXTLIBS_DIR)/mt19937ar
RANDOM_LIB = $(RANDOM_DIR)/random.o
export RANDOM_LIB
ifeq ($(LIB_ENABLE_RANDOM),YES)	# $gsc 乱数ライブラリを有効にする
#	OBJS += $(RANDOM_LIB)
endif

# graphics
GRAPHICS_DIR = graphics
GRAPHICS_LIB = $(GRAPHICS_DIR)/graphics.a
ifeq ($(COMP_ENABLE_GRAPHICS),YES)
	LIBS += $(GRAPHICS_LIB)
endif

# font, fontdata
FONT_DIR = font
FONT_LIB = $(FONT_DIR)/font.a
LIBS += $(FONT_LIB)
FONTDATA_DIR = fontdata
export FONTDATA_DIR
FONTDATA_LIB = $(FONTDATA_DIR)/fontdata.a
ifeq ($(COMP_ENABLE_FONTS),YES)
	LIBS += $(FONTDATA_LIB)
endif

# net, lwip
LWIP_DIR = $(EXTLIBS_DIR)/lwip
LWIP_LIB = $(LWIP_DIR)/liblwip.a
export LWIP_DIR
NET_DIR = net
NET_LIB = $(NET_DIR)/net.a
ifeq ($(COMP_ENABLE_TCPIP),YES)
	HEADERS += $(HEADERS_DIR)/net/*.h
	LIBS += $(LWIP_LIB)
	LIBS += $(NET_LIB)
endif

# shell
SHELL_DIR = shell
SHELL_LIB = $(SHELL_DIR)/shell.a
ifeq ($(COMP_ENABLE_SHELL),YES)
	LIBS += $(SHELL_LIB)
endif

# libmad
LIBMAD_DIR = $(EXTLIBS_DIR)/libmad
LIBMAD_LIB = $(LIBMAD_DIR)/libmad.a
ifeq ($(LIB_ENABLE_LIBMAD),YES)	# $gsc libmad(MPEG audio decoder)ライブラリを有効にする
	LIBS += $(LIBMAD_LIB)
endif

# libfaad2
LIBFAAD2_DIR = $(EXTLIBS_DIR)/faad2
LIBFAAD2_LIB = $(LIBFAAD2_DIR)/libfaad2.a
ifeq ($(LIB_ENABLE_LIBFAAD2),YES)	# $gsc libfaad2(MPEG-4 and MPEG-2 AAC decoder)ライブラリを有効にする
	LIBS += $(LIBFAAD2_LIB)
endif

# picojpeg
PICOJPEG_DIR = $(EXTLIBS_DIR)/picojpeg
PICOJPEG_LIB = $(PICOJPEG_DIR)/libpicojpeg.a
ifeq ($(LIB_ENABLE_PICOJPEG),YES)	# $gsc picojpeg(JPEG decoder)ライブラリを有効にする
	LIBS += $(PICOJPEG_LIB)
	SHELL_EXT_LIB += $(PICOJPEG_LIB)
	GRAPH_EXT_LIB += $(PICOJPEG_LIB)
endif

# libpng
LIBPNG_DIR = $(EXTLIBS_DIR)/libpng
LIBPNG_LIB = $(LIBPNG_DIR)/libpng.a
ifeq ($(LIB_ENABLE_LIBPNG),YES)	# $gsc libpngライブラリを有効にする
	LIBS += $(LIBPNG_LIB)
	SHELL_EXT_LIB += $(LIBPNG_LIB)
	GRAPH_EXT_LIB += $(LIBPNG_LIB)
endif

# zlib
LIBZLIB_DIR = $(EXTLIBS_DIR)/zlib
LIBZLIB_LIB = $(LIBZLIB_DIR)/libzlib.a
export LIBZLIB_DIR
ifeq ($(LIB_ENABLE_ZLIB),YES)	# $gsc zlibライブラリを有効にする
	LIBS += $(LIBZLIB_LIB)
	SHELL_EXT_LIB += $(LIBZLIB_LIB)
endif

ifdef APPLICATION	# $gsc アプリケーションディレクトリ
	APP_LIB = $(APPS_DIR)/$(APPLICATION)/$(APPLICATION).a
	ALIBS += $(APP_LIB)
endif

ifdef APPLICATION2	# $gsc アプリケーションディレクトリ(2)
	APP_LIB2 = $(APPS_DIR)/$(APPLICATION2)/$(APPLICATION2).a
	ALIBS += $(APP_LIB2)
endif

ifdef APPLICATION3	# $gsc アプリケーションディレクトリ(3)
	APP_LIB3 = $(APPS_DIR)/$(APPLICATION3)/$(APPLICATION3).a
	ALIBS += $(APP_LIB3)
endif

ifdef APPLICATION4	# $gsc アプリケーションディレクトリ(4)
	APP_LIB4 = $(APPS_DIR)/$(APPLICATION4)/$(APPLICATION4).a
	ALIBS += $(APP_LIB4)
endif


all: $(PROGRAM)

config.mk:
	@ls -F $(SYSCONF_DIR) | grep -v / > sys_list
	@awk -f $(TOOLS_DIR)/system_select.awk sys_list
	@awk -f $(TOOLS_DIR)/config_select.awk conf_list
	@rm -rf sys_list conf_list
	@mv config.tmp config.mk

$(TARGETCONFIGMAKE): $(SYSCONF_DIR)/$(SYSTEM_CONFIG).conf $(CONFIGS_DIR)/$(APPLICATION_CONFIG).conf
	awk -f $(TOOLS_DIR)/mktargetconfig_mk.awk $(SYSCONF_DIR)/$(SYSTEM_CONFIG).conf $(CONFIGS_DIR)/$(APPLICATION_CONFIG).conf > $(TARGETCONFIGMAKE)

$(ARCHHEADER): $(HEADERS_DIR)/asm-$(ARCH).h $(TARGETCONFIGMAKE)
	cp $(HEADERS_DIR)/asm-$(ARCH).h $(ARCHHEADER)

$(SYSCONFHEADER): $(SYSCONF_DIR)/$(SYSTEM_CONFIG).conf $(CONFIGS_DIR)/$(APPLICATION_CONFIG).conf
	awk -f $(TOOLS_DIR)/mksysconfig_h.awk $(SYSCONF_DIR)/$(SYSTEM_CONFIG).conf $(CONFIGS_DIR)/$(APPLICATION_CONFIG).conf > $(SYSCONFHEADER)

main.o: main.c $(ARCHHEADER) $(SYSCONFHEADER)

$(KERNEL_LIB): $(KERNEL_DIR)/*.c
	make -C $(KERNEL_DIR)

ifeq ($(ARCH),emu)
$(ARCH_LIB): $(ARCH_DIR)/*.c $(ARCH_DIR)/drivers/*.c
	make -C $(ARCH_DIR)
else
$(ARCH_LIB): $(ARCH_DIR)/*.c $(ARCH_DIR)/*.s $(ARCH_DIR)/drivers/*.c $(ARCH_DIR)/systems/*.c
	make -C $(ARCH_DIR)
endif

# ツール関連
$(BDF2FONTSRC): $(TOOLS_DIR)/bdf2fontc.c
	make -C $(TOOLS_DIR) $(notdir $@)

$(TXT2FONTSRC): $(TOOLS_DIR)/txt2fontc.c
	make -C $(TOOLS_DIR) $(notdir $@)

$(TXT2BITMAP): $(TOOLS_DIR)/txt2bitmap.c
	make -C $(TOOLS_DIR) $(notdir $@)

$(BMP2TXT): $(TOOLS_DIR)/bmp2txt.c
	make -C $(TOOLS_DIR) $(notdir $@)


# 外部ライブラリ
$(FATFS_LIB): $(FATFS_DIR)/Makefile
	make -C $(FATFS_DIR) all

$(LIBMAD_LIB): $(LIBMAD_DIR)/Makefile
	make -C $(LIBMAD_DIR)

$(LIBFAAD2_LIB): $(LIBFAAD2_DIR)/Makefile
	make -C $(LIBFAAD2_DIR)

$(LWIP_LIB): $(LWIP_DIR)/Makefile
	make -C $(LWIP_DIR)

$(PICOJPEG_LIB): $(PICOJPEG_DIR)/Makefile
	make -C $(PICOJPEG_DIR)

$(LIBPNG_LIB): $(LIBPNG_DIR)/Makefile $(LIBZLIB_LIB)
	make -C $(LIBPNG_DIR)

$(LIBZLIB_LIB): $(LIBZLIB_DIR)/Makefile
	make -C $(LIBZLIB_DIR)

$(RANDOM_LIB): $(RANDOM_DIR)/Makefile
	make -C $(RANDOM_DIR)

#
$(DRIVERS_LIB): $(DRIVERS_DIR)/*.c
	make -C $(DRIVERS_DIR)

$(LIBS_LIB): $(LIBS_DIR)/*.c $(LIBS_EXT_LIB) $(RANDOM_LIB)
	make -C $(LIBS_DIR)

$(GRAPHICS_LIB): $(GRAPHICS_DIR)/*.c $(GRAPH_EXT_LIB)
	make -C $(GRAPHICS_DIR)

$(FONT_LIB): $(FONT_DIR)/*.c
	make -C $(FONT_DIR)

$(FONTDATA_LIB): $(FONTDATA_DIR)/*/*.txt $(BDF2FONTSRC) $(TXT2FONTSRC)
	make -C $(FONTDATA_DIR)

$(UILIB_LIB): $(UILIB_DIR)/*.[ch]
	make -C $(UILIB_DIR)

$(FS_LIB): $(FS_DIR)/*.c
	make -C $(FS_DIR) all

$(TASK_LIB): $(TASK_DIR)/*.c
	make -C $(TASK_DIR)

$(NET_LIB): $(NET_DIR)/*.c
	make -C $(NET_DIR)

$(SHELL_LIB): $(SHELL_DIR)/*.c $(SHELL_EXT_LIB)
	make -C $(SHELL_DIR)

# アプリケーション
$(APP_LIB): $(APPS_DIR)/$(APPLICATION)/*.[ch] $(LIBS)
	make -C $(APPS_DIR)/$(APPLICATION)

$(APP_LIB2): $(APPS_DIR)/$(APPLICATION2)/*.[ch] $(LIBS)
	make -C $(APPS_DIR)/$(APPLICATION2)

$(APP_LIB3): $(APPS_DIR)/$(APPLICATION3)/*.[ch] $(LIBS)
	make -C $(APPS_DIR)/$(APPLICATION3)

$(APP_LIB4): $(APPS_DIR)/$(APPLICATION4)/*.[ch] $(LIBS)
	make -C $(APPS_DIR)/$(APPLICATION4)


OBJS	= main.o

SRCS    = main.c

.SUFFIXES:	.elf .a .o .c .s .h .txt .bmp

.c.o:
	$(CC) $(CFLAGS) -c $<

ifeq ($(COMP_ENABLE_GRAPHICS),YES)
LOGOBMP	= gs_logo.bmp
LOGOTXT	= gs_logo.txt
LOGOSRC	= gs_logo.c

SRCS += $(LOGOSRC)
OBJS += gs_logo.o
$(LOGOTXT): $(LOGOBMP) $(BMP2TXT)
	$(BMP2TXT)  $*.bmp > $*.txt

$(LOGOSRC): $(LOGOTXT) $(TXT2BITMAP)
	$(TXT2BITMAP)  $*.txt > $*.c
endif

ifeq ($(ARCH),emu)
$(PROGRAM): $(OBJS) $(SYSCONFHEADER) $(ALIBS) $(LIBS) emu.a
	$(LD) -o $@ $(OBJS) $(ALIBS) $(LIBS) $(LIBS_LIB) $(ARCH_LIB) emu.a $(LIBS) \
	-lm -lpthread -lcurses -lasound -lrt `pkg-config --cflags --libs gtk+-2.0`
	ln -f -s $(PROGRAM) $(PROGLINK)

emu.a: $(ARCHHEADER) $(SYSCONFHEADER) $(LIBS)
	echo "const char os_version[] = \"$(VERSION)\";" > version.c
	echo "const char build_date[] = __DATE__;" >> version.c
	echo "const char build_time[] = __TIME__;" >> version.c
	$(CC) $(CFLAGS) -c version.c
	$(AR) rc $@ version.o
else
$(PROGRAM): $(OBJS) $(ALIBS) $(LIBS) $(LDSCRIPT)
	echo "const char os_version[] = \"$(VERSION)\";" > version.c
	echo "const char build_date[] = __DATE__;" >> version.c
	echo "const char build_time[] = __TIME__;" >> version.c
	$(CC) $(CFLAGS) -c version.c
	$(LD) $(CFLAGS) $(LDFLAGS) $(ARCH_DIR)/start.o version.o \
	$(OBJS) $(ALIBS) $(LIBS) $(ARCH_LIB) $(LD_LIBS) $(LIBS)
	ln -f -s $(PROGRAM) $(PROGLINK)
	$(OBJDUMP) -h --section=.VECTORS --section=.text --section=.data \
	--section=.bss --section=.stack $(PROGRAM)
endif

tags: TAGS
TAGS:
	gtags -v

configclean:
	rm -f $(ARCHHEADER) $(SYSCONFHEADER) $(TARGETCONFIGMAKE)

objclean:
ifdef APPLICATION
	make -C $(APPS_DIR)/$(APPLICATION) clean
endif
ifdef APPLICATION2
	make -C $(APPS_DIR)/$(APPLICATION2) clean
endif
ifdef APPLICATION3
	make -C $(APPS_DIR)/$(APPLICATION3) clean
endif
ifdef APPLICATION4
	make -C $(APPS_DIR)/$(APPLICATION4) clean
endif
	make -C $(KERNEL_DIR) clean
	make -C $(TASK_DIR) clean
	make -C $(ARCH_DIR) clean
	make -C $(DRIVERS_DIR) clean
	make -C $(LIBS_DIR) clean
ifeq ($(LIB_ENABLE_RANDOM),YES)
	make -C $(RANDOM_DIR) clean
endif
ifeq ($(LIB_ENABLE_LIBMAD),YES)
	make -C $(LIBMAD_DIR) clean
endif
ifeq ($(LIB_ENABLE_LIBFAAD2),YES)
	make -C $(LIBFAAD2_DIR) clean
endif
ifeq ($(LIB_ENABLE_PICOJPEG),YES)
	make -C $(PICOJPEG_DIR) clean
endif
ifeq ($(LIB_ENABLE_LIBPNG),YES)
	make -C $(LIBPNG_DIR) clean
endif
ifeq ($(LIB_ENABLE_ZLIB),YES)
	make -C $(LIBZLIB_DIR) clean
endif
ifeq ($(COMP_ENABLE_GRAPHICS),YES)
	make -C $(GRAPHICS_DIR) clean
endif
ifeq ($(COMP_ENABLE_FONTS),YES)
	make -C $(FONT_DIR) clean
	make -C $(FONTDATA_DIR) clean
endif
ifeq ($(COMP_ENABLE_FATFS),YES)
	make -C $(FATFS_DIR) clean
	make -C $(FS_DIR) clean
endif
ifeq ($(COMP_ENABLE_TCPIP),YES)
	make -C $(LWIP_DIR) clean
	make -C $(NET_DIR) clean
endif
ifeq ($(COMP_ENABLE_SHELL),YES)
	make -C $(SHELL_DIR) clean
endif
ifeq ($(ENABLE_UILIB),YES)
	make -C $(UILIB_DIR) clean
endif
	make -C $(TOOLS_DIR) clean
	rm -f version.c version.o
	rm -f -r $(OBJS) $(PROGRAM) *.map \
	$(LOGOTXT) $(LOGOSRC) emu.a
	rm -f $(PROGLINK)

clean:
	make objclean
	make configclean
	rm -f -r $(DEPEND)

allobjclean:
	find . -name "*.[oa]" | xargs rm -f
	find . -name ".depend" | xargs rm -f

reset:
	make clean
	make allobjclean
	rm -f config.mk $(TARGETCONFIGMAKE) include/sysconfig.h

distclean:
	make -C $(FONTDATA_DIR) distclean
	make -C $(LIBS_DIR) distclean
	make -C $(FATFS_DIR) distclean
	make -C $(LWIP_DIR) distclean
	make -C $(LIBMAD_DIR) distclean
	make -C $(LIBFAAD2_DIR) distclean
	make -C $(PICOJPEG_DIR) distclean
	make -C $(LIBPNG_DIR) distclean
	make -C $(LIBZLIB_DIR) distclean
	make -C $(RANDOM_DIR) distclean
	make configclean
	rm -f config.mk
	rm -f -r $(DEPEND)

$(DEPEND): $(HEADERS_DIR)/asm.h $(SYSCONFHEADER)
	$(CC) -M $(CFLAGS) main.c > $(DEPEND)

.PHONY: clean docs distclean configlist

docs:
	doxygen docs/Doxyfile
	dot -T svg docs/task_status.dot -o docs/html/task_status.svg

docsclean:
	rm -f -r docs/html

configlist:
	sh tools/configlist.sh m > configs/CONFIG.jp.md

enmd:
	sh ./tools/translation/trans_md.sh README.jp.md > README.md
	sh ./tools/translation/trans_md.sh apps/APPLICATIONS.jp.md > apps/APPLICATIONS.md
	sh ./tools/translation/trans_md.sh shell/SHELL.jp.md > shell/SHELL.md
	sh ./tools/translation/trans_md.sh configs/CONFIG.jp.md > CONFIG.tmp
	awk '{if($$3=="|")$$2=toupper($$2);print $$0}' CONFIG.tmp > configs/CONFIG.md
	rm -f CONFIG.tmp

-include $(DEPEND)
