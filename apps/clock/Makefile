include $(PRJ_DIR)/common.mk

LIB	= clock.a

OBJS	= clock.o

CFLAGS += -I$(PRJ_DIR)/$(UILIB_DIR)

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
