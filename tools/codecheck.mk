TARGET		= codecheck.log
HEADERS_DIR	= include
LINT		= splint
LINTFLAGS	+= -I $(HEADERS_DIR)
LINTFLAGS	+= -I $(HEADERS_DIR)/task
#LINTFLAGS	+= -DGSC_COMP_ENABLE_FATFS
LINTFLAGS	+= -I extlibs/fatfs/source
LINTFLAGS	+= -I extlibs/picojpeg
LINTFLAGS	+= -I extlibs/picojpeg/picojpeg-master
LINTFLAGS	+= -I extlibs/libpng/libpng-1.6.34
LINTFLAGS	+= -I extlibs/lwip/arch
LINTFLAGS	+= -I extlibs/lwip/lwip-2.0.3/src/include
LINTFLAGS	+= -I uilib
LINTFLAGS	+= +skip-sys-headers
LINTFLAGS	+= +single-include
LINTFLAGS	+= +posixlib
LINTFLAGS	+= -globstate
LINTFLAGS	+= -castfcnptr
LINTFLAGS	+= -mustfreeonly
LINTFLAGS	+= -temptrans
LINTFLAGS	+= -unqualifiedtrans
LINTFLAGS	+= -exportlocal
LINTFLAGS	+= -nullassign
LINTFLAGS	+= -preproc
LINTFLAGS	+= -warnflags
LINTFLAGS	+= -fullinitblock
LINTFLAGS	+= +ignoresigns
LINTFLAGS	+= -initallelements
LINTFLAGS	+= -type
LINTFLAGS	+= -predboolint
LINTFLAGS	+= -noeffect
LINTFLAGS	+= -compdef
LINTFLAGS	+= -onlytrans
LINTFLAGS	+= -usedef
LINTFLAGS	+= -nullret
LINTFLAGS	+= -nullpass
LINTFLAGS	+= -nestedextern
LINTFLAGS	+= -retvalint
LINTFLAGS	+= -unrecog
LINTFLAGS	+= -paramuse
LINTFLAGS	+= -compdestroy
LINTFLAGS	+= -kepttrans
LINTFLAGS	+= -branchstate
LINTFLAGS	+= -compmempass
LINTFLAGS	+= -shiftimplementation
LINTFLAGS	+= -mutrep
LINTFLAGS	+= -incondefs
LINTFLAGS	+= -mustfreefresh
LINTFLAGS	+= -statictrans
LINTFLAGS	+= -boolops
LINTFLAGS	+= +charindex
LINTFLAGS	+= -usereleased
LINTFLAGS	+= -observertrans
LINTFLAGS	+= -unreachable
LINTFLAGS	+= -infloops
LINTFLAGS	+= -nullstate
LINTFLAGS	+= -shiftnegative
LINTFLAGS	+= -fixedformalarray
LINTFLAGS	+= -immediatetrans

LINTFLAGS	+= -DLINT
LINTFLAGS	+= -DGSC_GRAPHICS_COLOR_16BIT
#LINTFLAGS	+= -weak

SRCS	 = \
	main.c \
	kernel/console.c \
	kernel/datetime.c \
	kernel/device.c \
	kernel/init.c \
	kernel/memory.c \
	kernel/sysevent.c \
	kernel/timer.c \
	kernel/tkprintf.c \
	kernel/task/calltrace.c \
	kernel/task/event.c \
	kernel/task/mutex.c \
	kernel/task/queue.c \
	kernel/task/sleepqueue.c \
	kernel/task/syscall.c \
	kernel/task/syscall_api.c \
	kernel/task/task.c \
	kernel/task/waitqueue.c \
	libs/charcode.c \
	libs/fifo.c \
	libs/str.c \
	libs/tprintf.c \
	libs/vtprintf.c \
	shell/com_device.c \
	shell/com_file.c \
	shell/com_graphics.c \
	shell/com_i2c.c \
	shell/com_mem.c \
	shell/com_net.c \
	shell/com_system.c \
	shell/com_task.c \
	shell/file_operation.c \
	shell/history.c \
	shell/lineedit.c \
	shell/shell.c \
	shell/shell_task.c \
	graphics/font.c \
	graphics/graphics.c \
	graphics/graphics_object.c \
	graphics/graphics_op.c \
	graphics/jpegdec.c \
	graphics/pngdec.c \
	fs/batch.c \
	extlibs/fatfs/diskio.c \
	extlibs/fatfs/fattime.c \
	fs/file.c \
	fs/storage.c \
	net/devif.c \
	net/nettask.c \
	net/sys_arch.c \
	uilib/appsetting.c \
	uilib/ui_button.c \
	uilib/ui_progressbar.c \
	uilib/ui_scrollbar.c \
	uilib/ui_selectlist.c \
	uilib/ui_seekbar.c \
	uilib/ui_edittext.c \
	uilib/ui_switch.c \
#	kernel/syscalls.c \

CHKS	= $(notdir $(SRCS:.c=.k))

$(TARGET): $(CHKS)
	cat $(CHKS) > $@

$(CHKS): $(SRCS)

main.k: main.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: kernel/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: kernel/task/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: libs/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: shell/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: graphics/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: fs/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: extlibs/fatfs/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: net/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

%.k: uilib/%.c
	$(LINT) $(LINTFLAGS) $< 2>&1 | tee $@

clean:
	rm -rf $(CHKS) $(TARGET)
