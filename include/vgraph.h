#ifndef VGRAPH_H
#define VGRAPH_H



// virtual graphics and drawing
// routines to manage display of a UI

#include "liqbase.h"
#include "liqcell.h"

	

typedef unsigned int vcolor;

//#define vcolor_YUV(y,u,v)  ((unsigned int)( (((unsigned int)(y))<<24) | (((unsigned int)(u))<<16) | (((unsigned int)(v))<<8) | 0) )

// this is endian specific and horrible, it just means if memory converted to char * its ordered "YUVA"
#define vcolor_YUV(y,u,v)  ((unsigned int)( (((unsigned int)(v))<<16) | (((unsigned int)(u))<<8) | (((unsigned int)(y))) ) )

typedef
struct vgraph
{
	int usagecount;
	
	
	liqcliprect *cliprect;
	liqimage *target;

	int scalex;
	int scaley;
	
	int scalew;
	int scaleh;
	
	
	liqcell *window;

	
	unsigned int backcolor;
	unsigned int pencolor;
	unsigned int penthick;
	
	liqfont *font;		// i just release the one I have when I change fonts...   maybe even use a central fontlibrary :)
	
	int scaleaspectlock;
	
}
	vgraph;
	
	
vgraph *vgraph_new();
vgraph *vgraph_hold(					vgraph *self);
void    vgraph_release(					vgraph *self);
//void 	vgraph_free(vgraph *self);
	
int		vgraph_settarget(      			vgraph *self, liqimage *target );
int		vgraph_setwindow(      			vgraph *self, liqcell *window);// int x,int y,    int w,int h );
void    vgraph_convert_target2window(	vgraph *self, int tx,int ty,  int *wx, int *wy);
void    vgraph_convert_window2target(	vgraph *self, int wx,int wy,  int *tx, int *ty);
void    vgraph_setcliprect(      		vgraph *self, liqcliprect *cliprect );
liqcliprect *vgraph_getcliprect( 		vgraph *self);



int vgraph_setscaleaspectlock(vgraph *self,int newscaleaspectlock);
int vgraph_getscaleaspectlock(vgraph *self);


int		vgraph_setbackcolor(   			vgraph *self, vcolor backcolor );
int		vgraph_setpencolor(    			vgraph *self, vcolor pencolor );
int		vgraph_setpenthick(    			vgraph *self, int penthick );
int		vgraph_setfont(        			vgraph *self, liqfont *font);			//  char *fontname, int fontsize, int fontattributes
int		vgraph_drawclear(      			vgraph *self                                  );
int		vgraph_drawpoint(      			vgraph *self, int x, int y                    );
int		vgraph_drawline(       			vgraph *self, int x, int y, int ex,int ey     );
int		vgraph_drawbox(        			vgraph *self, int x, int y, int w,int h       );
int		vgraph_drawrect(       			vgraph *self, int x, int y, int w,int h       );
int		vgraph_drawcircle(     			vgraph *self, int x, int y, int radius        );
int		vgraph_drawellipse(    			vgraph *self, int x, int y, int rx,int ry     );
int		vgraph_drawtext(       			vgraph *self, int x, int y, char *text        );
int		vgraph_drawsketch(     			vgraph *self, int x, int y, int w,int h , liqsketch *sketch      );
int		vgraph_drawimage(      			vgraph *self, int x, int y, int w,int h , liqimage  *image      );
int		vgraph_drawcell(       			vgraph *self, int x, int y, int w,int h , liqcell   *cell      );

#endif
