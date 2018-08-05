DEPEND	= .depend

$(DEPEND): $(OBJS:.o=.c)
	-@ rm -f $(DEPEND)
	-@ for i in $^; do\
		$(CC) -MM $(CFLAGS) $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> $(DEPEND);\
	done
