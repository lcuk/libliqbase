
#ifndef liqui_H
#define liqui_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqbase.h"


// zach: no more textbox here... nothing needs to go in here atm


int textbox_clear(liqcell *textbox);
int textbox_selectall(liqcell *textbox);
int textbox_fakebackspace(liqcell *textbox);



int liqlist_clear(liqcell *liqlist); 
int liqlist_additem(liqcell *liqlist,char *item);
int liqlist_setindex(liqcell *liqlist,int index);
int liqlist_getindex(liqcell *liqlist);
int liqlist_count(liqcell *liqlist);

#ifdef __cplusplus
}
#endif

#endif
