

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "liqbase.h"

#include "liqcell.h"
#include "liqcell_prop.h"

#include "filebuf.h"


// walk a string and produce a set of spans matching the input





//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################





#include "liqcell.h"
static liqcell *stack=NULL;
static int expr();
static int stmt();
typedef unsigned int spanpoint;
	
static char *infirst;
static char *indat;
static int   inlen;
static int   inlinenum;


//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################




int liqcell_parse_liqbrain(liqcell *self,char *inputdata)
{
	// this should simply be an EXPANSION class
	// it should simply follow a rule tree
	// a rule tree should simply be a cell tree of rules
	// the hard coded example below is foolishly bad
	// and lacks much finess and completion.
	
	stack=NULL;
	//infirst = "begin library   fn test var a=1; return; end    var user_age:number = 10 + user_height + 96 * 20 ;  bob=20 ;   user_name=\"gary\" ;      fred=50 ;   end ";
	infirst = inputdata;
	indat=infirst;
	inlinenum=1;
	
	//liqcell *self=liqcell_quickcreatenameclass("liqcell_parse_liqbrain","parse");
	
	while(stmt())
	{
		while(stack)
		{
			liqcell *t=stack;
			stack=t->linknext;
			if(stack) stack->linkprev=NULL;
			t->linknext=NULL;
			//liqapp_log("inserting");
			liqcell_child_insert(self,t);
			//liqcell_print2(t);
			//liqcell_release(t);
		}
	}

	return 0;	
}

int liqcell_parse_liqbrain_filename(liqcell *self,char *filename)
{
	// this should simply be an EXPANSION class
	// it should simply follow a rule tree
	// a rule tree should simply be a cell tree of rules
	// the hard coded example below is foolishly bad
	// and lacks much finess and completion.
	
	struct filebuf filebuf;
	int err=filebuf_open(&filebuf,filename);
	if(err)
	{
    	{ return liqapp_warnandcontinue(-1,"doc_initfromfilename couldnt open file"); }				
	}	
	int res=liqcell_parse_liqbrain(self,filebuf.filedata);
	filebuf_close(&filebuf);



	//int res=liqcell_parse_liqbrain(self,"begin library   fn test var a=1; return; end    var user_age:number = 10 + user_height + 96 * 20 ;  bob=20 ;   user_name=\"gary\" ; fred=50 ;   end  ");

	return res;

}


//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################




//################################################ we have to see things

// see will attempt to match a micro constant with the data
// it is space dodging

static int see(char *pattern)
{
char *start=indat;
char *pattorig=pattern;
	if(!pattern || !*pattern) return 0;
	if(!indat   || !*indat  ) return 0;
	//app_log("see '%s'",pattern);
	while(*pattern && *indat)
	{
		switch(*pattern)
		{		
			case ' ':
				// need to have forced and optional spaces.
				if(pattern[1]==' ')
				{
					// required == "  " (double space)
					if(*indat==' ' || *indat=='\t' || *indat==10 || *indat==13)
					{
						if(*indat==10)inlinenum++;
						// phew! we got one, 
					}
					else
					{
						// no match, rollback and exit
						indat=start;
						return 0;
					}
				}
				else
				{
					// optional == " " (single space)
				}
				// skip whatever else we find
				while(*indat==' ' || *indat=='\t' || *indat==10 || *indat==13)
				{
					if(*indat==10)inlinenum++;
					*indat++;
				}
				pattern++;
				break;
			
			default:
				if(toupper(*pattern)==toupper(*indat))
				{
					// phew!
					*pattern++;
					*indat++;
				}
				else
				{
					// no match, rollback and exit
					indat=start;
					return 0;
				}
				break;
		}
	}
	//app_log("saw '%s'",pattorig);
	return indat-start;
}



//###########################################################################
//###########################################################################
//########################################################################### primatives
//###########################################################################
//###########################################################################

static int seecstring()
{
char *start=indat;
	if(!indat   || !*indat  ) return 0;
	see(" ");
int cnt=0;

	if(see(" \""))
	{
		
		while(*indat && *indat!='\"')
		{
			if(*indat=='\\' && indat[1])
			{
				// skip the \ specials
				indat++;
			}
			if(*indat==10 || *indat==13)
			{
				// invalid, we hit a CR or LF before terminator
				break;
			}
			else
			{
				cnt++;
				indat++;
			}
		}
		if(*indat=='\"')
		{
			// w000t
			indat++;
			return 1;
		}
		
		// missing terminator
		indat=start;
		return 0;
		
	}
	// no match, rollback and exit
	indat=start;
	return 0;
}

static int seecchar()
{
char *start=indat;
	if(!indat   || !*indat  ) return 0;
	see(" ");
int cnt=0;

	if(see(" \'"))
	{
		
		if(*indat && *indat!='\'')
		{
			if(*indat=='\\' && indat[1])
			{
				// skip the \ specials
				indat++;
			}
			if(*indat==10 || *indat==13)
			{
				// invalid, we hit a CR or LF before terminator
				//break;
			}
			else
			{
				cnt++;
				indat++;
			}
		}
		if(*indat=='\'')
		{
			// w000t
			indat++;
			return 1;
		}
		
		// missing terminator
		indat=start;
		return 0;
		
	}
	// no match, rollback and exit
	indat=start;
	return 0;
}


static int seenumber()
{
char *start=indat;
	if(!indat   || !*indat  ) return 0;
	see(" ");
int cnt=0;
	while(*indat>='0' && *indat<='9')
	{
		//
		//app_log("digit");
		cnt++;
		indat++;
	}
	if(*indat=='.')
	{
		indat++;
		while(*indat>='0' && *indat<='9')
		{
			//
			//app_log("digit");
			cnt++;
			indat++;
		}			
	}
	if(cnt)
	{
		//app_log("num");
		return 1;
	}
	// no match, rollback and exit
	indat=start;
	return 0;
}

static int seecommentline()
{
char *start=indat;
	if(!indat   || !*indat  ) return 0;
	see(" ");
	if(see("//"))
	{
		// on a winner
		while(*indat  && !(*indat==10 || *indat==13) )
		{
			//cnt++;
			indat++;
		}
		while(*indat==10 || *indat==13)
		{
			if(*indat==10)inlinenum++;
			indat++;
		}
		
		//app_log("commentline");
		return 1;
	}		

	// no match, rollback and exit
	indat=start;
	return 0;
}


static int seeidentifier()
{
char *start=indat;
	if(!indat   || !*indat  ) return 0;
	see(" ");
	
char *startnospace=indat;
int cnt=0;
	if((*indat>='a' && *indat<='z') || (*indat>='A' && *indat<='Z') || (*indat=='_'))
	{
		indat++;
		cnt++;
		while((*indat>='a' && *indat<='z') || (*indat>='A' && *indat<='Z') || (*indat=='_') || (*indat>='0' && *indat<='9'))
		{
			//
			//app_log("letter");
			indat++;
			cnt++;
		}
	}
/*
	if(*indat=='.')
	{
		// this really turns it into reference location
		indat++;
		if((*indat>='a' && *indat<='z') || (*indat>='A' && *indat<='Z') || (*indat=='_'))
		{
			indat++;
			cnt++;
			while((*indat>='a' && *indat<='z') || (*indat>='A' && *indat<='Z') || (*indat=='_') || (*indat>='0' && *indat<='9'))
			{
				//
				//app_log("letter");
				indat++;
				cnt++;
			}
		}		
	}
	
 */
	if(cnt)
	{
		
		if(cnt==3 && strncmp(startnospace,"end",3)==0)
		{
			// sorry, "end" is a keyword, i gather i need more later..
			// todo: fix keyword handling, "end" is not the only one, it needs to check a dictionary
			indat=start;
			return 0;
		}
		//liqapp_log("identifier '%s'",startnospace);
		return 1;
	}
	// no match, rollback and exit
	indat=start;
	return 0;
}




//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################



static spanpoint upto(char *breadcrumb)
{
	//app_log("upto   %3i,%s",(indat-infirst),breadcrumb);
	return (spanpoint)(indat-infirst);
}

static int shift(int what)
{
	//liqapp_log("shift   %3i",(indat-infirst));
	if(what)
	{
		// push the current cell onto the stack
		return 1;
	}
	else
	{
		return 0;
	}
}
static int reduce(spanpoint start,char *identifier)
{
	
liqcell *t = NULL;
	if(identifier)
	{
		// we want to also store the complete original text in the data property :)
		
		
		t = liqcell_quickcreatevis(identifier,"span",start,0,((indat-infirst)-start),1);
		
		while(stack && stack->x >= start)
		{
			liqcell *c = stack;			
			stack = c->linknext;
			c->linknext=NULL;
			if(stack)
			{
				stack->linkprev=NULL;
			}
			// expand t and position c
			//c->x=t->w;
			//liqcell_update_boundfrompos(c);
			
			//t->w+=c->w;
			//liqcell_update_boundfrompos(t);
			// now insert
			liqcell_child_insert(t,c);
		}
		// now put t into the stack
		t->linknext = stack;
		if(stack)
		{
			stack->linkprev=t;
		}
		stack = t;
		
		//liqapp_log("reduce %3i,%3i,%s",start,upto("reduce"),identifier);
	}
	else
	{
		// do not reduce the stack, the top item should already be correct
		//liqapp_log("reduce %3i,%3i,%s",start,upto("reduce"),"NULL");
	}
	
	

	return 1;
}
static int warn(spanpoint start,char *identifier,char *reason)
{
	// should pop the stack
	//liqapp_log("fail  msg    %3i :: %s",(indat-infirst),reason);
	char buf[64]; int bl=(indat-infirst); {if(bl>63)bl=63;} snprintf(buf,bl,"%s",indat);buf[bl]=0;
	char *clr=buf; while(*clr){ if(*clr==10 || *clr==13){*clr=' ';}  clr++; }
	
	//liqapp_log("warn  '%s' : '%s'",reason,buf);

	// first, fold the contents up into a bundle	
	return reduce(start,identifier);
	
	// then wrap all this up :)
	return reduce(start,"warn");
}
static int fail(spanpoint start,char *reason)
{
	// should pop the stack
	//liqapp_log("fail  msg    %3i :: %s",(indat-infirst),reason);

	//liqapp_log("fail  '%s'",reason);

	// first, fold the contents up into a bundle
	reduce(start,reason);
	
	// then wrap all this up in an error indicator :)
	reduce(start,"fail");
	
	return 0;

	//return 0;
}
static int revert(spanpoint start)
{
	// should pop the stack
	//liqapp_log("revert   %3i",(indat-infirst));
	indat=infirst+start;
	return 0;
}



//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################



static int exprconst()
{
	spanpoint start=upto("const");

	if(shift(seenumber()))
	{
		return reduce(start,"number");
	}
	
	if(shift(seecstring()))
	{
		while(shift(seecstring()))
		{
			// get all of them in one go..
		}
		return reduce(start,"string");
	}
	
	if(shift(seecchar()))
	{
		return reduce(start,"char");
	}	

	return revert(start);
}






static int exprref()
{
	spanpoint start=upto("ref");
	
	
	if(shift(seeidentifier()))
	{
		// some.thing
		while(shift(see(" .")))
		{
			if(shift(seeidentifier()))
			{
				//..
			}
			else
			{
				return fail(start,"missing ident");			
			}
		}
		return reduce(start,"ref");
	}

	return revert(start);
}

static int exprargs()
{
	spanpoint start=upto("args");
	if(shift(see(" (")))
	{
		if(shift(expr()))
		{
			while(shift(see(" ,")))
			{
				if(shift(expr()))
				{
					//..
				}
				else
				{
					return fail(start,"missing args expr");			
				}
			}
			if(shift(see(" )")))
			{
				return reduce(start,"args");
			}
			return fail(start,"missing args close");
		}
		return fail(start,"missing args expr");
	}
	return revert(start);
}



static int exprvar()
{
	spanpoint start=upto("var");
	
	
	if(shift(seeidentifier()))
	{
		// some.thing
		while(shift(see(" .")))
		{
			if(shift(seeidentifier()))
			{
				//..
			}
			else
			{
				return fail(start,"missing ident");			
			}
		}
		if(shift(exprargs()))
		{
			// fn(ARGS)
			return reduce(start,"function");
		}
		return reduce(start,"ident");
	}
	
	
	if(shift(exprconst()))
	{
		return reduce(start,NULL);
	}
	



	return revert(start);
}



static int exprbra()
{
	spanpoint start=upto("bra");

	if(shift(see(" (")))
	{
		if(shift(expr()))
		{
			if(shift(see(" )")))
			{
				return reduce(start,"bracket");
			}
			//
			return fail(start,"missing bracket close");
		}
		return fail(start,"missing bracket expr");
	}
	if(shift(exprvar()))
	{
		return reduce(start,NULL);
	}
	return revert(start);
}

static int exprneg()
{
	spanpoint start=upto("neg");

	if(shift(see(" -")))
	{
		if(shift(exprneg()))
		{
			//
			return reduce(start,"neg");
		}
		return fail(start,"missing negative");
	}
	if(shift(exprbra()))
	{
		return reduce(start,NULL);
	}
	return revert(start);
}

static int exprmul()
{
	spanpoint start=upto("mul");
	if(shift(exprneg()))
	{
		if(shift(see(" *")))
		{
			if(shift(exprmul()))
			{
				//
				return reduce(start,"mul");
			}
			return fail(start,"missing multiplier");
		}
		if(shift(see(" /")))
		{
			if(shift(exprmul()))
			{
				//
				return reduce(start,"div");
			}
			return fail(start,"missing divisor");
		}
		return reduce(start,NULL);
	}
	return revert(start);
}




static int expradd()
{
	spanpoint start=upto("add");
	if(shift(exprmul()))
	{
		if(shift(see(" +")))
		{
			if(shift(expradd()))
			{
				//
				return reduce(start,"plus");
			}
			return fail(start,"missing sum");
		}
		if(shift(see(" -")))
		{
			if(shift(exprmul()))
			{
				//
				return reduce(start,"minus");
			}
			return fail(start,"missing sub");
		}
		return reduce(start,NULL);
	}
	return revert(start);
}


static int expr()
{
	spanpoint start=upto("expr");
	if(shift(exprref()))		// ident
	{
		if(shift(see(" =")))
		{
			if(shift(expr()))		// expr
			{
				return reduce(start,"assign");
			}
			return fail(start,"missing assign");
		}
		// revert because we want to retry this reference in the proper expression syntax
		revert(start);
	}
	if(shift(expradd()))
	{
		return reduce(start,NULL);
	}
	return revert(start);	
}


static int decl()
{
	spanpoint start=upto("decl");

	if(shift(exprref()))		// ident
	{
		if(shift(see(" :")))
		{
			if(shift(exprref()))		// class
			{
				if(shift(see(" =")))
				{
					if(shift(expr()))		// init
					{
						return reduce(start,"decl_var_class_init");					
					}
					return fail(start,"missing decl_var_class_init");
				}
				return reduce(start,"decl_var_class");					
			}
			return fail(start,"missing decl_var_class");
		}
		if(shift(see(" =")))
		{
			if(shift(expr()))		// init
			{
				return reduce(start,"decl_var_init");					
			}
			return fail(start,"missing decl_var_init");
		}
		return reduce(start,"decl_var");
	}
	return revert(start);
}


static int comment()
{
	spanpoint start=upto("commend");
	if(seecommentline())
	{
		while(seecommentline())
		{
			
			// ..
		}
		return reduce(start,"comment");				
	}
	return revert(start);
}


static int stmt()
{
	spanpoint start=upto("stmt");
	if(shift(see(" var ")))
	{
		if(shift(decl()))
		{
			if(shift(see(" ;")))
			{
				//return reduce(start,"var");
				// now entirely optional :)
			}
			return reduce(start,"var");
			//return warn(start,"var","missing return ;");
		}
		return fail(start,"missing decl");
	}
	
	if(shift(see(" fn ")))
	{
		if(shift(decl()))
		{
			while(shift(stmt()))
			{
				// ..
			}
			if(shift(see(" end ")))
			{
				return reduce(start,"sub");
			}
			return fail(start,"missing sub end");
		}
		return fail(start,"missing sub decl");
	}
		

	if(shift(see(" begin ")))
	{
		if(shift(decl()))
		{
			while(shift(stmt()))
			{
				// ..
			}
			if(shift(see(" end ")))
			{
				return reduce(start,"group");
			}
			return warn(start,"group","missing group end");
		}
		return fail(start,"missing group decl");
	}
	
	if(shift(see(" return ")))
	{
		if(shift(expr()))
		{
			if(shift(see(" ;")))
			{
				//return reduce(start,"return");
				// now entirely optional :)
			}
			return reduce(start,"return");
			//return warn(start,"return","missing return terminator ;");
		}
		if((see(" ;")))
		{
			//return reduce(start,"return");
			// now entirely optional :)
		}
		return reduce(start,"return");
		//return warn(start,"return","missing return ;");
	}


	if(shift(comment()))
	{
		return reduce(start,"comment");				
	}
	
	if(shift(decl()))
	{
		if((see(" ;")))
		{
			// now entirely optional :)
		}
			return reduce(start,NULL);				
		//return warn(start,"decl","missing decl terminator ;");
	}
	return revert(start);
}



































/*







static int html()
{
	spanpoint start=upto("html");
	if(shift(see(" <")))
	{
		// baseline core subset of identifiable things required for parsing
		// < a     href= '' >
		// < img   src = '' >
		// < br    /        >
		// < html  >
		// < head  >
		// < body  >
		// < / body  >
		// < / head  >
		// < / html  >

		// < script  >
		// < / script  >
		
		if(shift(see("/")))
		{
			// closing tag
			if( shift(exprref()) )
			{
				// "</[ref]>"
				if(shift(see(" > ")))
				{
					return reduce(start,"tag.closing");
				}
				// bad, something extra ontop of a closing tag...
			}
			
		}
	
		if( shift(exprref()) )
		{
			
			while(shift(exprconst()))
			{
				// multiple constants are applicable
				// ..
			}
			
			if(shift(see(" > ")))
			{
				// now we know we have a tag, we must loop and continue all children until we encounter a closing tag of this type
				// we are not trying to be smart and we are not trying to guess structure yet
				return reduce(start,"tag.opening");
			}
			// bad, something extra ontop of a closing tag...
		}
	
		

		return fail(start,"missing sub decl");
	}
	return revert(start);
}





int seehtml()
{
	//..
	
}




int liqcell_parse_html(liqcell *self,char *inputdata)
{
	// this should simply be an EXPANSION class
	// it should simply follow a rule tree
	// a rule tree should simply be a cell tree of rules
	// the hard coded example below is foolishly bad
	// and lacks much finess and completion.
	
	stack=NULL;
	//infirst = "begin library   fn test var a=1; return; end    var user_age:number = 10 + user_height + 96 * 20 ;  bob=20 ;   user_name=\"gary\" ;      fred=50 ;   end ";
	infirst = inputdata;
	indat=infirst;
	inlinenum=1;
	
	//liqcell *self=liqcell_quickcreatenameclass("liqcell_parse_html","parse");
	
	while(stmt())
	{
		while(stack)
		{
			liqcell *t=stack;
			stack=t->linknext;
			if(stack) stack->linkprev=NULL;
			t->linknext=NULL;
			//liqapp_log("inserting");
			liqcell_child_insert(self,t);
			//liqcell_print2(t);
			//liqcell_release(t);
		}
	}

	return 0;	
}

int liqcell_parse_html_filename(liqcell *self,char *filename)
{
	// this should simply be an EXPANSION class
	// it should simply follow a rule tree
	// a rule tree should simply be a cell tree of rules
	// the hard coded example below is foolishly bad
	// and lacks much finess and completion.
	
	struct filebuf filebuf;
	int err=filebuf_open(&filebuf,filename);
	if(err)
	{
    	{ return liqapp_warnandcontinue(-1,"liqcell_parse_html_filename couldnt open file"); }				
	}	
	int res=liqcell_parse_html(self,filebuf.filedata);
	filebuf_close(&filebuf);



	//int res=liqcell_parse_liqbrain(self,"begin library   fn test var a=1; return; end    var user_age:number = 10 + user_height + 96 * 20 ;  bob=20 ;   user_name=\"gary\" ; fred=50 ;   end  ");

	return res;

}




*/

















