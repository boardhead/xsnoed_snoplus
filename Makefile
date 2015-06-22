#
# Makefile to build xsnoed related programs for any supported OSTYPE/MACHTYPE
#
# Phil Harvey - 12/2/98
#
# Notes:
#
# 1) Do a do a 'make clean' between builds of different xsnoed versions
#    (normal, root and library versions), because these versions share
#    the same object files but are compiled with different options.
#
# 2) Set the XSNOED_OPTIONS environment variable to change the XSNOED
#    version compiler options.  Here are some examples:
#
#      setenv XSNOED_OPTIONS -UFITTR       // compiles without fitter
#      setenv XSNOED_OPTIONS -DNO_DISPATCH // compiles without dispatcher
#      setenv XSNOED_OPTIONS -DSNOPLUS     // compiles for SNO+
#

VER_NORM      = $(XSNOED_OPTIONS) -DFITTR -DSNOPLUS
VER_ROOT      = $(XSNOED_OPTIONS) -DFITTR -DROOT_FILE -DROOT_SYSTEM -DROOT_APP
VER_LIB       = $(XSNOED_OPTIONS) -DFITTR -DROOT_FILE -DROOT_SYSTEM -DNO_MAIN
#-NT
VER_SNOMAN    = $(XSNOED_OPTIONS) -DFITTR -DNO_MAIN -DXSNOMAN -DNO_DISPATCH

PLATFORM      = $(OSTYPE)_$(MACHTYPE)$(XSNOED_MAKE_VER)

PROGS         = xsnoed

all:	$(PROGS)

# Set all these dependencies to files that don't exist,
# thereby forcing the appropriate make to be performed each time.

#nt
xsnoed: xsnoed_
	@echo Done.

xsnoed.root: xsnoed.root_
	@echo Done.

redispatch: redispatch_
	@echo Done.

# Call make again for the proper platform

xsnoed.root_:
	@echo ==== Making xsnoed.root ====
	make "VERFLAGS=$(VER_ROOT)" -f Makefile.$(PLATFORM) xsnoed.root

xsnoed_:
	@echo ==== Making xsnoed ====
	make "VERFLAGS=$(VER_NORM)" -f Makefile.$(PLATFORM) xsnoed

redispatch_:
	make -f Makefile.$(PLATFORM) redispatch

root:
	@echo ==== Making root.exe ====
	make -f Makefile.$(PLATFORM) root.exe

libqsnoed:
	@echo ==== Making libqsnoed.so ====
	make "VERFLAGS=$(VER_LIB)" -f Makefile.$(PLATFORM) libqsnoed

# Utilities

clean:
	rm -f *.o *.C *.log core
	@echo Clean.

install: libqsnoed
	cp -p libqsnoed.so ../lib
	cp -p Q*.cxx ../src
	cp -p Q*.h ../src
	cp -p Q*.h ../include
	cp -p include/*.h ../include/include
	cp -p XSnoed ../parameters
	cp -p *.geo ../parameters
	cp -p database.dat ../parameters
	cp -p fitter.dat ../parameters
	make -f Makefile.$(PLATFORM) root.exe
	cp -p root.exe ../bin/root
	strip ../bin/root
	@echo Installed.

zip:
	rm -f xsnoed.tar xsnoed.tar.gz xsnoed_headers.dat
	ls *.h include/*.h linux_patch/*.h qplib/*.h > xsnoed_headers.dat
	tar -cf xsnoed.tar *.cxx *.h *.com Makefile* XSnoed.resource README include/* linux_patch/* qplib/* html/* *.geo *.dat *.doc
	gzip xsnoed.tar
	@echo Done.


unzip:
	cp xsnoed.tar.gz tmp_xsnoed.tar.gz
	gunzip tmp_xsnoed.tar.gz
	tar -xf tmp_xsnoed.tar
	rm -f tmp_xsnoed.tar
	@echo Done.


