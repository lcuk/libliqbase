LIBRARY     = libliqbase
VERS        = 0.3.0
CC          = g++
LD          = g++

OPT_FLAGS   =  -O3 
 

USE_GST = 1


UNAMEISARM := $(shell uname -m | grep arm)

ifeq ($(UNAMEISARM),arm)
# comment out next line for harmattan, no osso is used there.
	USE_OSSO = 1
endif


LIBRARIES   =  libpng12

ifdef USE_OSSO
	LIBRARIES += libosso
endif
ifdef USE_GST
	LIBRARIES += gstreamer-0.10 
endif

MAEMO  = 
ifdef USE_OSSO
	MAEMO = -DUSE_MAEMO
endif
CFLAGS      = $(MAEMO) -fPIC -Wall `freetype-config --cflags` `pkg-config --cflags  $(LIBRARIES)` -I../include 
LDFLAGS     = -lX11 -lXext -lXv -lm -lcurl `freetype-config --libs` -ljpeg -lpthread `pkg-config --libs $(LIBRARIES)`
RM          = /bin/rm -f

OBJSUI      = liqx11overlay.o liqx11_cover.o liqdialog_showtree.o liqcameraface.o liqimage_thumbnail.o liqimage_rotate.o liqlist.o liqcell_child_select.o liqx11info.o filebuf.o vgraph.o liqcell.o liqcell_arrange.o liqcell_prop.o liqcell_easyrun.o liqcell_easypaint.o liqcell_parse_filename.o liqcell_parse_liqbrain.o liqcell_easyhandler_kinetic.o liqui.o liqcell_dllcache.o md5.o textbox.o liqkeyboard.o liqaccel.o  liqcell_historystore.o liqsketchedit.o liqimagescan_hotspot.o
OBJS        = liqtimer.o liqcell_easyrun_multitouch.o liqrecentphotoselect.o liqapp.o liqapp_hildon.o liqapp_prefs.o liqapp_filecache.o liqcanvas_firstrun_splash.o liq_xsurface.o liqcanvas.o liqimage.o liqfont.o liqfontview.o liqsketch.o liqsketchpagefilename.o liqcliprect.o liqapp_turbo.o $(OBJSUI) dialog_selectimage.o dialog_selectimage_grid.o dialog_selectcolor.o dialog_selectcolor_greycube.o dialog_selectcolor_colorcube.o liqcell_mk_star.o liqtag.o liqsketchfont.o liqdoc.o liqdialog.o 


ifdef USE_OSSO
	CFLAGS += -DUSE_OSSO
endif

ifdef USE_GST
	OBJS  += liqcamera.o 
endif


.PHONY:     clean distclean

all:        $(LIBRARY)

$(LIBRARY): $(OBJS) 
		$(LD) -shared -Wl,-soname,libliqbase.so.1 -o libliqbase.so.1 $(LDFLAGS)  $(OBJS)


.c.o:
	      $(CC) $(OPT_FLAGS) $(CFLAGS) -c $< -o $*.o
clean:
	      $(RM) *~ $(OBJS) $(PROG) libliqbase.so
.PHONY:     clean

