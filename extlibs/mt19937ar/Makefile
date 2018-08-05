include $(PRJ_DIR)/common.mk

OBJS	= random.o

RANDOMSRC = random.c

RANDOMORGSRC = mt19937ar.c

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(OBJS)

$(RANDOMSRC): $(RANDOMORGSRC)
	patch -o $(RANDOMSRC) $(RANDOMORGSRC) random.patch

$(RANDOMORGSRC):
	wget http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/CODES/$(RANDOMORGSRC)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	make clean
	rm -f -r $(RANDOMORGSRC) $(RANDOMSRC) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
