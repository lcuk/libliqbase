
#ifndef liqcell_DLLCACHE_H
#define liqcell_DLLCACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liqbase.h"
#include "liqcell.h"



//################################################### dllcache class


typedef struct dllcacheitem
{
	char *key;
	char *filename;
	void *dll;
	liqcell *(*constructor)();
}
	dllcacheitem;


//################################################### storage for the cache
//static dllcacheitem *      dllcache     =NULL;
//static int                 dllcache_size=0;
//static int                 dllcache_used=0;

/**
 * prepare the cache
 */

int dllcache_init();

/**
 * free the cache
 */

int dllcache_close();

int dllcache_scan_dllfile(const char *dll_filename);

int dllcache_scan_folder(const char *widgetpath);

/**
 * scan the files to initialize the cache
 */

int dllcache_scan();

/**
 * run a specified constructor and return the result
 */

liqcell *dllcache_runconstructor(const char *classname);

dllcacheitem *dllcache_getbase();
int           dllcache_getsize();
int           dllcache_getused();

#ifdef __cplusplus
}
#endif

#endif
