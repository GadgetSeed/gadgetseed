include $(PRJ_DIR)/common.mk

LIB	= drivers.a

LWIP = lwip-2.0.2
CFLAGS	+= -I$(PRJ_DIR)/net -I$(PRJ_DIR)/net/$(LWIP)/src/include

OBJS = ${addsuffix .o, $(DRIVERS)}

all:	$(LIB)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
