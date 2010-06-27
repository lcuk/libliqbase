#ifdef __cplusplus
extern "C" {
#endif

//###############################################################################
//############################################################################### Group
//###############################################################################

/**
 * construct a group of non visual cells based on a va_list
 *
 */

liqcell *mkgroupa(char *key,char *classname,liqcell *first,va_list arg)
{
	//app_log("grp %s start",key);
	liqcell *self= liqcell_quickcreatenameclass(key,classname);
	//self->visible=1;
	
	// add some special sauce ;)
	//liqcell_childappend(self,mkhot("hot"));
	
	if(first)
	{
		//app_log("grp %s appending first",key);
		liqcell_child_append(self,first);
		//va_list arg;
		//va_start(arg, first);
		while(1)
		{
			liqcell *c = va_arg(arg, liqcell *);
			if(!c)break;
			//app_log("grp %s appending n",key);
			liqcell_child_append(self,c);
		};
		//va_end(arg);
	}
	return self;
}


/**
 * construct a group of cells based on ...
 *
 */

liqcell *mkgroup(char *key,liqcell *first,...)
{
	va_list arg;
	va_start(arg, first);
	liqcell *self= mkgroupa(key,"group",first,arg);
	//self->layoutmode=1;
	va_end(arg);
	return self;
}

#ifdef __cplusplus
}
#endif

