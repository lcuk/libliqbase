
#ifndef liqui_H
#define liqui_H

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqbase.h"


// zach: no more textbox here... nothing needs to go in here atm


int textbox_clear(liqcell *textbox);
int textbox_selectall(liqcell *textbox);
int textbox_fakebackspace(liqcell *textbox);

#endif
