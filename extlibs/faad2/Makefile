include $(PRJ_DIR)/common.mk

LIBFAAD2 = faad2-2.8.8
LIBFAAD2ARC = $(LIBFAAD2).tar.gz
#LIBFAAD2 = faad2-2.7
#LIBFAAD2ARC = $(LIBFAAD2).tar.bz2

LIBFAAD2URI = http://downloads.sourceforge.net/faac/$(LIBFAAD2ARC)
LIBFAAD2DIR = $(LIBFAAD2)/libfaad
LIBFAAD2SRC = $(LIBFAAD2DIR)/bits.c

LIB	= libfaad2.a

CFLAGS += -w -DHAVE_CONFIG_H -iquote $(LIBFAAD2) -iquote $(LIBFAAD2)/libfaad -iquote $(LIBFAAD2)/include

FAAD2OBJS	= \
	$(LIBFAAD2DIR)/bits.o \
	$(LIBFAAD2DIR)/cfft.o \
	$(LIBFAAD2DIR)/common.o \
	$(LIBFAAD2DIR)/decoder.o \
	$(LIBFAAD2DIR)/drc.o \
	$(LIBFAAD2DIR)/drm_dec.o \
	$(LIBFAAD2DIR)/error.o \
	$(LIBFAAD2DIR)/filtbank.o \
	$(LIBFAAD2DIR)/hcr.o \
	$(LIBFAAD2DIR)/huffman.o \
	$(LIBFAAD2DIR)/ic_predict.o \
	$(LIBFAAD2DIR)/is.o \
	$(LIBFAAD2DIR)/lt_predict.o \
	$(LIBFAAD2DIR)/mdct.o \
	$(LIBFAAD2DIR)/mp4.o \
	$(LIBFAAD2DIR)/ms.o \
	$(LIBFAAD2DIR)/output.o \
	$(LIBFAAD2DIR)/pns.o \
	$(LIBFAAD2DIR)/ps_dec.o \
	$(LIBFAAD2DIR)/ps_syntax.o \
	$(LIBFAAD2DIR)/pulse.o \
	$(LIBFAAD2DIR)/rvlc.o \
	$(LIBFAAD2DIR)/sbr_dct.o \
	$(LIBFAAD2DIR)/sbr_dec.o \
	$(LIBFAAD2DIR)/sbr_e_nf.o \
	$(LIBFAAD2DIR)/sbr_fbt.o \
	$(LIBFAAD2DIR)/sbr_hfadj.o \
	$(LIBFAAD2DIR)/sbr_hfgen.o \
	$(LIBFAAD2DIR)/sbr_huff.o \
	$(LIBFAAD2DIR)/sbr_qmf.o \
	$(LIBFAAD2DIR)/sbr_syntax.o \
	$(LIBFAAD2DIR)/sbr_tf_grid.o \
	$(LIBFAAD2DIR)/specrec.o \
	$(LIBFAAD2DIR)/ssr.o \
	$(LIBFAAD2DIR)/ssr_fb.o \
	$(LIBFAAD2DIR)/ssr_ipqf.o \
	$(LIBFAAD2DIR)/syntax.o \
	$(LIBFAAD2DIR)/tns.o

OBJS = $(FAAD2OBJS)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(LIBFAAD2SRC): $(LIBFAAD2ARC)
	tar xvfz $(LIBFAAD2ARC)
#	tar xvfj $(LIBFAAD2ARC)
	touch $(LIBFAAD2SRC)

$(LIBFAAD2ARC):
	wget $(LIBFAAD2URI)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	rm -f -r $(LIBFAAD2ARC) $(LIBFAAD2) $(DEPEND) $(LIB)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
