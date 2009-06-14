#ifndef liqcell_PROP_H
#define liqcell_PROP_H

#include <unistd.h>
#include <stdarg.h>
//#include "liqcell.h"


int     liqcell_propgeti(liqcell *self,char *name,int valueifnotfound);
char*   liqcell_propgets(liqcell *self,char *name,char *valueifnotfound);
int     liqcell_propseti(liqcell *self,char *name,int value);
char*   liqcell_propsets(liqcell *self,char *name,char *value);


int 	liqcell_propsets_vprintf(liqcell *self,char *name,char *format, va_list arg);
int 	liqcell_propsets_printf(liqcell *self,char *name,char *format, ...);





int liqcell_propremoves(liqcell *self,char *name);
int liqcell_propremovei(liqcell *self,char *name);



#endif
