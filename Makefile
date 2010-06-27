PROG        = libliqbase
VERS        = 0.3.0
CC          = g++
LD          = g++


OPT_FLAGS   =  -O3
#DESTDIR	?= tmp

ETCDIR	    = $(DESTDIR)/etc
PREFIX	    = $(DESTDIR)/usr
SUDODIR	    = $(ETCDIR)/sudoers.d
BINDIR	    = $(PREFIX)/bin

PGDIR		    = $(PREFIX)/share/liqbase



PREFIX	    = $(DESTDIR)/usr

.PHONY:     clean distclean
all:        
	$(MAKE) -C src

clean:
	$(MAKE) clean -C src

.PHONY:     clean

install:


	install -d $(BINDIR)
	install -d $(SUDODIR)

	mkdir -p                                    $(PREFIX)/include
	mkdir -p                                    $(PREFIX)/include/liqbase
	cp -r include/*.h                           $(PREFIX)/include/liqbase/

	mkdir -p                                    $(PREFIX)/lib
	install src/libliqbase.so.1                 $(PREFIX)/lib/

	mkdir -p						  $(PREFIX)/share/liqbase
	mkdir -p					 	  $(PREFIX)/share/liqbase/libliqbase
	mkdir -p						  $(PREFIX)/share/liqbase/libliqbase/media
	cp -r media/*                           	  $(PREFIX)/share/liqbase/libliqbase/media/




	install -m 0755 liqbase_base_fs/usr/bin/liqbase-playground-cpu-ondemand                         $(BINDIR)
	install -m 0755 liqbase_base_fs/usr/bin/liqbase-playground-cpu-powersave                        $(BINDIR)
	install -m 0755 liqbase_base_fs/usr/bin/liqbase-playground-cpu-performance                      $(BINDIR)
	install -m 0755 liqbase_base_fs/etc/sudoers.d/libliqbase.sudoers                                $(SUDODIR)

