PROG        = libliqbase
VERS        = 0.3.0
CC          = gcc
LD          = gcc


OPT_FLAGS   =  -O3
#DESTDIR	?= tmp



PREFIX	    = $(DESTDIR)/usr

.PHONY:     clean distclean
all:        
	$(MAKE) -C src

clean:
	$(MAKE) clean -C src

.PHONY:     clean

install:

	mkdir -p                                    $(PREFIX)/lib
	cp src/libliqbase.so                        $(PREFIX)/lib/
	cp src/libliqbase.so                        $(PREFIX)/lib/libliqbase.so.0

	mkdir -p                                    $(PREFIX)/include
	mkdir -p                                    $(PREFIX)/include/liqbase
	cp -r include/*.h                           $(PREFIX)/include/liqbase/




