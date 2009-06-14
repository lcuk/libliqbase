
#ifndef LIQDIALOGS_H
#define LIQDIALOGS_H 1

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"


// small module describing the dialog boxes available in the system

// MOST ARE NOT DONE YET AND REQUIRE COMPLETION, 	TIME GOT AWAY FROM ME


/**
 * Showmessage
 */



//int liqdialog_showmessage(char *key,char *title,char *description,char *bodytext,char *buttonscommadelim);	// returns index of the button clicked


// int res = liqdialog_showmessage("hello","this is a dialogbox","put the main body for the dialog here","ok,cancel");





/**
 * Showtree
 */



int liqdialog_showtree(char *key,char *title,char *description,liqcell *data);


// int res = liqdialog_showtree("hello","this is a dialogbox","put the main body for the dialog here",universe);




/**
 * askquestion
 */



//int liqdialog_askquestion(char *key,char *title,char *description,char *result,int resultlen);	// returns 1 if accepted, 0 if cancelled





/**
 * askcolor
 */



//int liqdialog_askcolor(char *key,char *title,char *result,int resultlen);	// returns 1 if accepted, 0 if cancelled



#endif
