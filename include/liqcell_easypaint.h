
#ifndef liqcell_EASYPAINT_H
#define liqcell_EASYPAINT_H

#include "liqbase.h"
#include "liqcell.h"


void liqcell_easypaint(liqcell *self,liqcliprect *cr,    int x,int y,    int w,int h);


int liqcell_threadloadimage(liqcell *self);		// start the thread running loading am image




// liqcell_propseti(  self ,  "imagefloat", "yes" );

#endif
