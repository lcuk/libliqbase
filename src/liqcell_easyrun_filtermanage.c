



//# this is a standard filter operation

static int liqcell_filter_run(liqcell *c,char *searchterm)
{
    liqcellfiltereventargs filterargs;
    filterargs.filterinuse= (searchterm ? 1 : 0);
    filterargs.searchterm = searchterm;
    filterargs.searchtags = NULL;
    filterargs.resultoutof= 1;
    filterargs.resultshown= 0;
    
    if(!filterargs.resultshown)
    {
        if(searchterm && *searchterm)
        {
        }
        else
        {
            filterargs.resultshown = 1;
        }
    }
    
    if(!filterargs.resultshown)
    {
        if(c->name)
            filterargs.resultshown = ( stristr(c->name,searchterm) != NULL );
    }

    if(!filterargs.resultshown)
    {
        if(c->classname)
            filterargs.resultshown = ( stristr(c->classname,searchterm) != NULL );
    }
    
    liqcell *ccontent = liqcell_getcontent( c );
    if(ccontent)
    {
        liqapp_log("searching in cont '%s', %i",c->name,filterargs.resultshown);
        liqcell_handlerrun( ccontent , "filter", &filterargs );
    }
    else
    {
        liqapp_log("searching in flat '%s', %i",c->name,filterargs.resultshown);
        liqcell_handlerrun( c , "filter", &filterargs );
    }

    if(filterargs.resultshown)
    {
        liqcell_setvisible(c,1);
        return 1;
    }
    else
    {
        liqcell_setvisible(c,0);
        return 0;
    }
}









//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################
	static int widget_search_click(liqcell *self, liqcelleventargs *args, liqcell *widget)
	{
		// try to add this tag :)
		liqcell *body = liqcell_child_lookup(widget,"body");
		char *searchterm = liqcell_getcaption(self);
		if(!searchterm || !*searchterm) return 1;
		//liqcell *body = liqcell_child_lookup(widget,"body");
	    //widget_insert(body,searchterm, 45);
		
		liqcell_setcaption(self,"");
		return 1;
	}
    
    
	static int widget_search_change(liqcell *self, liqcelleventargs *args, liqcell *widget)
	{
		// examine each tag and if matches the search show it, otherwise dont..
		liqcell *body = liqcell_child_lookup(widget,"body");
		char *searchterm = liqcell_getcaption(self);
		
        liqcell *searchinprogress = liqcell_child_lookup(body,"searchinprogress");
        
		liqcell *c = liqcell_getlinkchild_visual(body);
		while(c)
		{
			liqcell_filter_run(c,searchterm);
			c=liqcell_getlinknext_visual(c);
		}

        liqcell_setvisible(searchinprogress,0);
            
        if(liqcell_child_countvisible(body)==0)
        {
            liqcell_setcaption_printf(searchinprogress,"No results found");
            liqcell_propsets(  searchinprogress, "backcolor",   "xrgb(40,0,0)" );
        }
        else
        {
            liqcell_setcaption_printf(searchinprogress,"Search results:",liqcell_child_countvisible(body) );
            liqcell_propsets(  searchinprogress, "backcolor",   "xrgb(0,40,0)" );
        }
        
        if(!searchterm || !*searchterm)
            liqcell_setvisible(searchinprogress,0);
        else
            liqcell_setvisible(searchinprogress,1);
        
		//liqcell_setrect(body,   0,40,widget->w,widget->h-40);
		liqcell_setrect(body,   0,0,widget->w,widget->h);
		liqcell_child_arrange_easytile( body );
		//liqcell_child_arrange_makegrid_fly(body,3,3);
		
		liqcell_propseti(self,"arrangecomplete",0);
		
		//liqcell_setpos(body,0,40);
		
		if(!searchterm || !*searchterm)
		{
			// bit of magic here..
			liqcell_setvisible(self,0);
		}
        
        

        
		
		return 1;
		
	}
    
    
    
    // allow repainting
    
    
    
	static int widget_paint(liqcell *self, liqcellpainteventargs *args,liqcell *widget)
	{
		liqcell *search = liqcell_child_lookup(widget,"search");
        if(!search)return 0;
		if(liqcell_getvisible(search))
		{
			if( liqcell_gety(search) > ( liqcell_geth(widget) - liqcell_geth(search) )  )
			{
				// move it a bit more onscreen
				int dif = liqcell_gety(search) - ( liqcell_geth(widget) - liqcell_geth(search) );
				//if(dif>5)dif=5;
				liqcell_setpos( search, liqcell_getx(search), liqcell_gety(search) - dif );
				liqcell_setdirty(widget,1);
			}
		}
		return 0;
	}

	static int widget_resize(liqcell *self, liqcelleventargs *args, liqcell *widget)
	{
		liqcell *search = liqcell_child_lookup(widget,"search");
        if(!search)return 0;
		liqcell_setrect(search, widget->w*0.2 ,widget->h-40,   widget->w*0.6, 60);
		//liqcell_setrect(search, self->w*0.2 ,0,   self->w*0.4, 40);
        return 0;
    }
    
    
    
    
    
    
    
    
    
    
		liqcell *search = liqcell_quickcreatevis("search","textbox",self->w*0.2 ,self->h-40,   self->w*0.6, 60);
		liqcell_setfont(   search,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (32), 0) );
		liqcell_setcaption(search, "" );
		liqcell_propsets(  search, "textcolor",   "rgb(255,255,255)" );
		liqcell_propsets(  search, "backcolor",   "xrgb(0,40,0)" );
		liqcell_handleradd_withcontext( search,    "click",           search_click,  self );
		liqcell_handleradd_withcontext( search,    "captionchange",   search_change, self );
		liqcell_setvisible(search,0);		// watch this!
		liqcell_child_append( self, search );
		
		liqcell_handleradd_withcontext(body,    "keypress",   playground_keypress,  self);
		liqcell_handleradd_withcontext(body,    "keyrelease", playground_keyrelease,self);
		liqcell_handleradd_withcontext(self,    "keypress",   playground_keypress,  self);
		liqcell_handleradd_withcontext(self,    "keyrelease", playground_keyrelease,self);

		liqcell_handleradd_withcontext(self,    "resize",     playground_resize,    self);
		liqcell_handleradd_withcontext(self,    "refresh",    playground_refresh,   self);
		liqcell_handleradd_withcontext(self,    "paint",      playground_paint,  	self);
        
        
        
        
        
        
        
        
        
    