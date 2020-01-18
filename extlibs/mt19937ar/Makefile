include $(PRJ_DIR)/common.mk

OBJS	= mt19937ar.o

SRC = mt19937ar.c

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(OBJS)

$(SRC):
	wget http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/CODES/$(SRC)
	patch -p0 < random.patch

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	make clean
	rm -f -r $(SRC) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
