# Makefile for ...par/main
# Just say "make"

include $(CWPROOT)/src/Makefile.config

D = $I/cwp.h $I/par.h $L/libcwp.a $L/libpar.a

LFLAGS= $(PRELFLAGS) -L$L -lpar -lcwp -lm $(POSTLFLAGS)

PROGS =			\
	$B/hti2stiff	\
	$B/hudson	\
	$B/linrort	\
	$B/refRealAziHTI\
	$B/refRealVTI	\
	$B/thom2hti	\
	$B/thom2stiff

INSTALL:	$(PROGS)
	@-rm -f INSTALL
	@touch $@

$(PROGS):	$(CTARGET)
	$(CC) $(CFLAGS) $(@F).c $(LFLAGS) -o $@
	$(MCHMODLINE)
	@echo $(@F) installed in $B

remake:
	@-rm -f $(PROGS) INSTALL
	@$(MAKE) INSTALL

clean	::
	rm -f junk* core a.out *.o
