
#ifndef liqcell_ARRANGE_H
#define liqcell_ARRANGE_H 1

#ifdef __cplusplus
extern "C" {
#endif



// this arrange module contains functions for arranging clusters of visual cells


#include "liqcell.h"
#include "liqcell_prop.h"


#define ABS(X) ((X)<0 ? -(X) : (X))
#define SGN(X) ((X)<0 ? -(1) : (1))


//##################################################################
//################################################################## dimension base
//##################################################################
// dimension isnt really a class, its just a span of Start..End and the handling of it
//#####################################################################
static inline void dimension_forceinbound(register int *s,register int *m,register int l,register int r);
//#####################################################################
static inline int dimension_gapcalc(register int cs,register int ce,register int ds,register int de,int *gapres);
//#####################################################################
static inline int dimension_overlapcalc(register int cs,register int ce,register int ds,register int de,int *overlapres);
//##################################################################
//##################################################################
//##################################################################
//##################################################################
//##################################################################
int     liqcell_child_countvisible(liqcell *self);
int     liqcell_child_countselected(liqcell *self);


void liqcell_forceinboundparent(liqcell *self);

int liqcell_child_arrange_autoflow(liqcell *self);			// flowing document text;
int liqcell_child_arrange_nooverlap(liqcell *self,liqcell *currentselection);
int liqcell_child_arrange_makegrid(liqcell *self,int viscolcount,int visrowcount);
int liqcell_child_arrange_makegrid_fly(liqcell *self,int viscolcount,int visrowcount);	// special flyto mode
int liqcell_child_arrange_easytile(liqcell *self);
int liqcell_child_arrange_easyrow(liqcell *self);
int liqcell_child_arrange_easycol(liqcell *self);

#ifdef __cplusplus
}
#endif

#endif
