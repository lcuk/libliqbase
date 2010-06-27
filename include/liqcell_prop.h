#ifndef liqcell_PROP_H
#define liqcell_PROP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdarg.h>
//#include "liqcell.h"


int     liqcell_propgeti(liqcell *self, const char *name,int valueifnotfound);
char*   liqcell_propgets(liqcell *self, const char *name, const char *valueifnotfound);

int     liqcell_propseti(liqcell *self, const char *name,int value);
char*   liqcell_propsets(liqcell *self, const char *name, const char *value);

void*   liqcell_propgetp(liqcell *self, const char *name,void *valueifnotfound);
void *  liqcell_propsetp(liqcell *self, const char *name,void * value);

int 	liqcell_propsets_vprintf(liqcell *self, const char *name, const char *format, va_list arg);
int 	liqcell_propsets_printf(liqcell *self, const char *name, const char *format, ...);

int liqcell_propremoves(liqcell *self, const char *name);
int liqcell_propremovei(liqcell *self, const char *name);
int liqcell_propremovep(liqcell *self, const char *name);

#ifdef __cplusplus
}
#endif

#endif
