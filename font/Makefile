include $(PRJ_DIR)/common.mk

LIB	= font.a

OBJS	= fontlist.o

all: $(LIB)

$(LIB): $(OBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(SRCS) $(LIB) $(DEPEND) *~

distclean:
	make clean

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
