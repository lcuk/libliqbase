
#ifndef liqcell_EASYPAINT_H
#define liqcell_EASYPAINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liqbase.h"
#include "liqcell.h"


void liqcell_easypaint(liqcell *self,liqcliprect *cr,    int x,int y,    int w,int h);


int liqcell_threadloadimage(liqcell *self);		// start the thread running loading am image

unsigned int decodecolor(char *source,unsigned char *ry,unsigned char *ru,unsigned char *rv,unsigned char *ra,unsigned char *rc);

#ifdef __cplusplus
}
#endif

#endif
