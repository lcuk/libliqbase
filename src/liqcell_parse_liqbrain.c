

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "liqbase.h"

#include "liqcell.h"
#include "liqcell_prop.h"

#include "filebuf.h"

#ifdef __cplusplus
extern "C" {
#endif

// walk a string and produce a set of spans matching the input


int liqcell_parse_html(liqcell *self, const char *inputdata);


//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################





#include "liqcell.h"
static liqcell *stack=NULL;
static int expr();
static int stmt();
static int html();
static int somehtml();
typedef unsigned int spanpoint;
	
static const char *infirst;
static const char *indat;
//static int   inlen;
static int   inlinenum;


//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################




int liqcell_parse_liqbrain(liqcell *self, const char *inputdata)
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
	

 
	
	
	stack=NULL;
	//infirst = "begin library   fn test var a=1; return; end    var user_age:number = 10 + user_height + 96 * 20 ;  bob=20 ;   user_name=\"gary\" ;      fred=50 ;   end ";
	infirst = inputdata;
	indat=infirst;
	inlinenum=1;
	//liqcell *self=liqcell_quickcreatenameclass("liqcell_parse_liqbrain","parse");
	while(somehtml())
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
// example
//{
//  liqcell *parse1=liqcell_quickcreatenameclass("parse1","parse");
//  int res=liqcell_parse_liqbrain(self,"begin libraryx   fn test var a=1; return; end  user_height=160;  var user_age:number = 10 + user_height + 96 * 20 ;  bob=20 ;   user_name=\"gary\" ; fred=50 ;   end  ");
//  liqcell_parse_liqbrain_filename(parse1,"/usr/share/liqbase/media/parse.liqbrain.example.lol");
//  liqcell_print2(parse1);
//}


int outmore(int outused, char *outbuf, int amount)
{
	if(outused+amount<sizeof(outbuf))
	{
		outused+=amount;
		return 1;
	}
	return 0;
}


/**
 * Print a tree of liqcells starting with the provided liqcell
 * @param self The provided liqcell
 */
static void liqcell_parse_liqbrain_print2(liqcell *self,int s, const char *inbuffer)
{
	//static int recdep=0;
	//liqcell_print(self,"self",recdep*4);
	//if(recdep>=2)return;
	//recdep++;
	
	s=0;
	int off = liqcell_getx(self);
	int wid = liqcell_getw(self);
	
	int insiz=strlen(inbuffer);
	
	char outbuf[512]={0};
	int outused=0;
		
	if(s+off>0)
	{
		if(outmore(outused, outbuf, s+off))
		{
			//memset( &outbuf[0], ' ', s+off);
			
			memcpy( &outbuf[0], &inbuffer[0],   s+off);
		}
	}
	//if(wid)
	{
		
		
		if(outmore(outused, outbuf, 9))
		{

			if(strcasecmp(self->name,"fail")==0 )
			{
				memcpy( &outbuf[s+off], &(  "\e[37;41m"  ),   9);
			}
			else
			{
				memcpy( &outbuf[s+off], &(  "\e[37;44m"  ),   9);
				
			}
		}
		
		
		if(wid)
		{
			if(outmore(outused, outbuf, wid))
			{
				//memset( &outbuf[s+off+8], '*', wid);
				memcpy( &outbuf[s+off+8], &inbuffer[s+off],   wid);
			}
		}
		
		if(outmore(outused, outbuf, 5))
		{
			memcpy( &outbuf[s+off+8+wid], &(  "\e[0m"  ),   5);
		}
	}	
	//if(outmore(1))
	//{
	//	//memset( &outbuf[s+off+wid+1], ']', 1);
	//}
	// i should attempt to draw the remaining section
	
	// finalize the string
	if( insiz > s+off+wid )
	{
		strncpy(&outbuf[s+off+7+wid+5],&inbuffer[s+off+wid],sizeof(outbuf) - outused);
	}
	
	
	// outbuf is null terminated
	
	liqcell *xx=self->linkchild;
	while(xx)
	{
		
		liqcell_parse_liqbrain_print2(xx,s+off,inbuffer);
		xx=xx->linknext;
	}	
	liqapp_log("# %s %s:%s",outbuf,(self->name?self->name:""),(self->classname?self->classname:"") );
	
	
	
	

	//recdep--;
}


int liqcell_parse_liqbrain_test()
{
	// run tests
	{
		liqcell *parse1;
		//char *buffer="begin libraryx   fn test var a=1; return; end  user_height=160;  var user_age:number = 10 + user_height + 96 * 20 ;  bob=20 ;   user_name=\"gary\" ; fred=50 ;   end  ";
		
		// shorter version
		//char *buffer="begin lx; fn tst var a=1; return; end uh=160; var ua:num = 10 + uh + 96 * 20 ; bob=20; un=\"gary\"; fred=50; end";
		
		
		
		int res;
		const char *buffer;
		
		
		parse1=liqcell_quickcreatenameclass("parse1","parse");
		buffer="begin lx; fn tst() var a=1; return 9; end uh=160; var ua:num = 10 + uh + 96 * 20 ; bob=20; un=\"gary\"; fred=50; end";
		res=liqcell_parse_liqbrain(parse1,buffer);
		//liqcell_print2(parse1);
		liqcell_parse_liqbrain_print2(parse1,0,buffer);
		liqcell_release(parse1);
		
		parse1=liqcell_quickcreatenameclass("parse2","parse");
		buffer="begin tt; fn hello() return 22; end <html><head><title>hello world</title></head><body width=40>anyone home?</body></html>";
		res=liqcell_parse_liqbrain(parse1,buffer);
		//liqcell_print2(parse1);
		liqcell_parse_liqbrain_print2(parse1,0,buffer);		
		//liqcell_parse_liqbrain_filename(parse1,"/usr/share/liqbase/media/parse.liqbrain.example.lol");
		
		
		parse1=liqcell_quickcreatenameclass("parse2","parse");
		buffer="<table><tr><TD class='bifhitem' width=100px>2010-05-05T09:37:08</TD></tr></table>";
		res=liqcell_parse_liqbrain(parse1,buffer);
		//liqcell_print2(parse1);
		liqcell_parse_liqbrain_print2(parse1,0,buffer);		
		//liqcell_parse_liqbrain_filename(parse1,"/usr/share/liqbase/media/parse.liqbrain.example.lol");
		
				
		
		
		
		
		
		
		liqcell_release(parse1);
	}
	exit(0);
}

//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################
//###########################################################################




//################################################ we have to see things

// see will attempt to match a micro constant with the data
// it is space dodging

static int see(const char *pattern)
{
const char *start=indat;
//char *pattorig=pattern;
	if(!pattern || !*pattern) return 0;
	if(!indat   || !*indat  ) return 0;
	//liqapp_log("see '%s'",pattern);
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
					indat++;
				}
				pattern++;
				break;
			
			default:
				if(toupper(*pattern)==toupper(*indat))
				{
					// phew!
					pattern++;
					indat++;
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
	//liqapp_log("saw '%s'",pattorig);
	return indat-start;
}



//###########################################################################
//###########################################################################
//########################################################################### primatives
//###########################################################################
//###########################################################################
static int seeqstring()
{
const char *start=indat;
	if(!indat   || !*indat  ) return 0;
	//see(" ");
int cnt=0;

	if(see(" \'"))
	{
		
		while(*indat && *indat!='\'')
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


static int seecstring()
{
const char *start=indat;
	if(!indat   || !*indat  ) return 0;
	//see(" ");
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
const char *start=indat;
	if(!indat   || !*indat  ) return 0;
	//see(" ");
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
const char *start=indat;
	if(!indat   || !*indat  ) return 0;
	see(" ");
int cnt=0;
	while(*indat>='0' && *indat<='9')
	{
		//
		//liqapp_log("digit");
		cnt++;
		indat++;
	}
	if(*indat=='.')
	{
		indat++;
		while(*indat>='0' && *indat<='9')
		{
			//
			//liqapp_log("digit");
			cnt++;
			indat++;
		}			
	}

	if(cnt)
	{
		if(*indat=='p' && indat[1]=='x')
		{
			indat+=2;
			cnt+=2;
		}
		else
		{
			if(*indat=='%')
			{
				indat++;
			}
		}
		

		//liqapp_log("num");
		return 1;
	}
			
	// no match, rollback and exit
	indat=start;
	return 0;
}

static int seecommentline()
{
const char *start=indat;
	if(!indat   || !*indat  ) return 0;
	//see(" ");
	if(see(" //"))
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
		
		//liqapp_log("commentline");
		return 1;
	}		

	// no match, rollback and exit
	indat=start;
	return 0;
}

static int seesinglecharacter()
{
	if(!indat   || !*indat  ) return 0;
	// return and advance a character
	indat++;
	return 1;
}
	
	
static int seeidentifier()
{
const char *start=indat;
	if(!indat   || !*indat  ) return 0;
	see(" ");
	
const char *startnospace=indat;
int cnt=0;
	if((*indat>='a' && *indat<='z') || (*indat>='A' && *indat<='Z') || (*indat=='_'))
	{
		indat++;
		cnt++;
		while((*indat>='a' && *indat<='z') || (*indat>='A' && *indat<='Z') || (*indat=='_') || (*indat>='0' && *indat<='9'))
		{
			//
			//liqapp_log("letter");
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
				//liqapp_log("letter");
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



static spanpoint upto(const char *breadcrumb)
{
	//liqapp_log("upto   %3i,%s",(indat-infirst),breadcrumb);
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
static int reduce(spanpoint start, const char *identifier)
{
	
liqcell *t = NULL;


				

	if(identifier)
	{
		// we want to also store the complete original text in the data property :)
		
		// remove trailing whitespace from the buffer
		const char *infin = indat;
		if(infin && infin > (char *)start)
		{
			while(infin > (char *)start && ( infin[-1]==' ' || infin[-1]=='\t' || infin[-1]==10 || infin[-1]==13 ) )
			{
				infin--;
			}
		}

		t = liqcell_quickcreatevis(identifier,"span",start,0,((infin-infirst)-start),1);
		
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
static int warn(spanpoint start, const char *identifier, const char *reason)
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
static int fail(spanpoint start, const char *reason)
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
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("const");

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
	
	if(shift(seeqstring()))
	{
		while(shift(seeqstring()))
		{
			// get all of them in one go..
		}
		return reduce(start,"string");
	}	
	
	
	
	if(shift(seecchar()))
	{
		return reduce(start,"char");
	}	

	return revert(realstart);
}






static int exprref()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("ref");
	
	
	if(shift(seeidentifier()))
	{
		// some.thing
		while(shift(see(".")))
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

	return revert(realstart);
}

static int exprargs()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("args");
	if(shift(see("(")))
	{
		if(shift(expr()))
		{
			see(" ");
			while(shift(see(",")))
			{
				if(shift(expr()))
				{
					//..
				}
				else
				{
					return fail(start,"missing args expr");			
				}
				see(" ");
			}
			if(shift(see(")")))
			{
				return reduce(start,"args");
			}
			return fail(start,"missing args close");
		}
		return fail(start,"missing args expr");
	}
	return revert(realstart);
}



static int exprvar()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("var");
	
	
	if(shift(seeidentifier()))
	{
		// some.thing
		//see(" ");
		while(shift(see(".")))
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
		see(" ");
		if(shift(exprargs()))
		{
			// fn(ARGS)
			return reduce(start,"function");
		}
		return reduce(start,"ident");
	}
	
	see(" ");
	if(shift(exprconst()))
	{
		return reduce(start,NULL);
	}
	



	return revert(realstart);
}



static int exprbra()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("bra");

	if(shift(see("(")))
	{
		if(shift(expr()))
		{
			see(" ");
			if(shift(see(")")))
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
	return revert(realstart);
}

static int exprneg()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("neg");

	if(shift(see("-")))
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
	return revert(realstart);
}

static int exprmul()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("mul");
	if(shift(exprneg()))
	{
		see(" ");
		if(shift(see("*")))
		{
			if(shift(exprmul()))
			{
				//
				return reduce(start,"mul");
			}
			return fail(start,"missing multiplier");
		}
		if(shift(see("/")))
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
	return revert(realstart);
}




static int expradd()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("add");
	if(shift(exprmul()))
	{
		if(shift(see("+")))
		{
			if(shift(expradd()))
			{
				//
				return reduce(start,"plus");
			}
			return fail(start,"missing sum");
		}
		if(shift(see("-")))
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
	return revert(realstart);
}


static int expr()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("expr");
	if(shift(exprref()))		// ident
	{
		if(shift(see("=")))
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
	return revert(realstart);	
}


static int decl()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("decl");

	if(shift(exprref()))		// ident
	{
		see(" ");
		if(shift(see(":")))
		{
			if(shift(exprref()))		// class
			{
				see(" ");
				if(shift(see("=")))
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
		if(shift(see("=")))
		{
			if(shift(expr()))		// init
			{
				return reduce(start,"decl_var_init");					
			}
			return fail(start,"missing decl_var_init");
		}
		return reduce(start,"decl_var");
	}
	return revert(realstart);
}


static int comment()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("comment");
	if(seecommentline())
	{
		while(seecommentline())
		{
			
			// ..
		}
		return reduce(start,"comment");				
	}
	return revert(realstart);
}


static int stmt()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("stmt");
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
		if(shift(exprref()))
		{
			
			if(shift(see(" (")))
			{
				if(shift(decl()))
				{
					// ..
					while( shift(see(" ,")))
					{
						if(shift(decl()))
						{
							// ..
						}
						else
						{
							// fail
							return fail(start,"missing sub argument");
						}
					}

				}
				
				if( shift(see(")")))
				{
					// ok
				}
				else
				{
					// fail
					return fail(start,"missing sub closing bracket");					
				}
			}
			else
			{
				// fail
				return fail(start,"missing sub ()");
			}
			
			// one of only places to require a ; to mark pre-declaration only
			if(shift(see(" ;")))
			{
				return reduce(start,"sub.declare");
			}
			
			// main definition
			
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
		return fail(start,"missing sub name");
	}
		

	if(shift(see(" begin ")))
	{
		if(shift(decl()))
		{

			if(shift(see(" ;")))
			{
				//return reduce(start,"var");
				// now entirely optional :)
			}
			
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
			return reduce(start,"return.value");
			//return warn(start,"return","missing return terminator ;");
		}
		if((see(" ;")))
		{
			//return reduce(start,"return");
			// now entirely optional :)
		}
		return reduce(start,"return.empty");
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
	return revert(realstart);
}





































//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################



struct htmlbufnode
{
	int index;
	char *identifier;
	liqcell *hit_exprref;
	int foundclosingtag;
};

static struct htmlbufnode htmlbuf[256];
static int                htmlbuf_used=0;
static struct htmlbufnode * htmlbuf_alloc(liqcell *hit_exprref);
static int htmlbuf_match_samename(struct htmlbufnode *a, struct htmlbufnode *b);
static int htmlbuf_release(struct htmlbufnode *a);
static int htmlbuf_isclosing();

static struct htmlbufnode * htmlbuf_alloc(liqcell *hit_exprref)
{
	if(htmlbuf_used<sizeof(htmlbuf))
	{

		int off = liqcell_getx(hit_exprref);
		int wid = liqcell_getw(hit_exprref);
		char *ident = strndup(&infirst[off],wid);
		htmlbuf[htmlbuf_used].index=htmlbuf_used;
		htmlbuf[htmlbuf_used].identifier = ident;
		htmlbuf[htmlbuf_used].hit_exprref = hit_exprref;
		htmlbuf[htmlbuf_used].foundclosingtag = 0;
		
		
		//liqapp_log("htmlbuf_alloc %d,%d,%d '%s'",htmlbuf_used, off,wid,ident );
		htmlbuf_used++;
		return &htmlbuf[htmlbuf_used-1];
	}
	return NULL;
}
static int htmlbuf_match_samename(struct htmlbufnode *a, struct htmlbufnode *b)
{
	if(  a && b && a->identifier && b->identifier && (strcmp(a->identifier,b->identifier)==0) )
	{
		//liqapp_log("htmlbuf_match_samename %d,%d'%s'=='%s'",a->index,b->index, a->identifier,b->identifier );
		return 1;
	}
	return 0;
}
static int htmlbuf_release(struct htmlbufnode *a)
{
	//liqapp_log("htmlbuf_release %d '%s'",a->index, a->identifier, a->index==(htmlbuf_used-1) ? "YES" : "NO" );
		
		
	if(  a && a->identifier )
	{
		//liqapp_log("htmlbuf_release %d '%s'",a->index, a->identifier );
		free(a->identifier);
		a->identifier=NULL;
		a->index=-a->index;
		//if(a->index==(htmlbuf_used-1))
		
		htmlbuf_used--;
		

		
		return 1;
	}
	htmlbuf_used--;
	
	return 0;
}

static int htmlbuf_isclosing()
{
	if(htmlbuf_used==0)
	{
		return 0;
	}
	
					int htmlbuf_idx=0;
					for(htmlbuf_idx=0;htmlbuf_idx<htmlbuf_used-1;htmlbuf_idx++)
					{
						//liqapp_log("htmlbuf_isclosing %d '%s' ? %d",htmlbuf_idx, htmlbuf[htmlbuf_idx].identifier, htmlbuf[htmlbuf_idx].foundclosingtag );
						if( htmlbuf[htmlbuf_idx].foundclosingtag )
						{
							// quick exit, a parent branch closed leaving us open
							// we must vacate this loop now
							return 1;
						}
					}
					return 0;
}




//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################

static int htmltagarg()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("htmltagarg");

	if( shift(exprref()) )
	{
		if(shift(see(" =")))
		{
			if(shift(exprconst()))
			{
				return reduce(start,"htmltagarg");
			}
			return fail(start,"missing html tag arg value");
		}
		return fail(start,"missing html tag arg expression");
	}

	return revert(realstart);
}


static int html()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("html");
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
		
		if(shift(see(" / ")))
		{
			// closing tag
			if( shift(exprref()) )
			{
				liqcell *tagreference = stack;
				
				
				// "</[ref]>"
				
				if(shift(see(" > ")))
				{
					// now, we should check the htmlbuf

					// start perversly enough by allocating a new item on the stack
					// this actually allocates us an identifier to compare against
					
					struct htmlbufnode * htmlbufnodex = htmlbuf_alloc( tagreference );

					int htmlbuf_idx=0;
					for(htmlbuf_idx=0;htmlbuf_idx<htmlbufnodex->index;htmlbuf_idx++)
					{
						if( htmlbuf_match_samename(htmlbufnodex, &htmlbuf[htmlbuf_idx] ) )
						{
							htmlbuf[htmlbuf_idx].foundclosingtag = 1; // :D
						}
					}
					// we are done with our temporary item now, it should actually be top of the stack
					
					htmlbuf_release(htmlbufnodex);
					return reduce(start,"tag.closing");
				}
				// bad, something extra ontop of a closing tag...
				return fail(start,"missing html close tag closing '>'");
			}
			
			return fail(start,"missing html close tag ref");
			
		}
	
		if( shift(exprref()) )
		{
			liqcell *tagreference = stack;
			
			
			while(shift(htmltagarg()))
			{
				// multiple arguments are applicable
				// ..
			}
			if(shift(see(" / ")))
			{
				// we have been given a tag which is marked as self closing
				// there is to be no more content within

				if(shift(see(" > ")))
				{
					// now we know we have a tag, we must loop and continue all children until we encounter a closing tag of this type
					// we are not trying to be smart and we are not trying to guess structure yet
					return reduce(start,"tag.opening");
				}
				// bad, something extra ontop of a closing tag...				
				return fail(start,"missing html tag auto close completion '>'");			
			}
			
			if(shift(see(" > ")))
			{
				// now we know we have a tag, we must loop and continue all children until we encounter a closing tag of this type
				// we are not trying to be smart and we are not trying to guess structure yet


				struct htmlbufnode * htmlbufnodex = htmlbuf_alloc( tagreference );


				while(shift(somehtml()))
				{
					// multiple children are applicable
					// however, we might have been closed due to a matching tag.
					// this would be indicated by the
					// however, we should stop if our buffer has been marked as completed



					if( htmlbuf_isclosing() )
					{
						// quick exit, a parent branch closed leaving us open
						// we must vacate this loop now
						break;
					}

					if(htmlbufnodex->foundclosingtag)
					{
						// quick exit because this closing tag was identified
						break;
					}
				}
				
				// release the buffer node now
				
				htmlbuf_release(htmlbufnodex);
				
				return reduce(start,"tag.opening");
			}
			// bad, something extra ontop of a closing tag...
			
			return fail(start,"missing html tag completion: '>'");
		}
	
		

		return fail(start,"missing html tag ref");
	}
	return revert(realstart);
}





static int somehtmlfrag()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("somehtmlfrag");

	if(shift(see(" ")))
	{	
	}
	if( *indat && *indat!='<' )
	{
		while(*indat && *indat!='<')
		{
			if(shift(seesinglecharacter()))
			{
				//
			}
			
		}
		return reduce(start,"somehtmlfrag");
	}
	return revert(realstart);
}



static int somehtml()
{
	spanpoint realstart=upto(""); see(" "); spanpoint start=upto("somehtml");

	if(shift(see(" ")))
	{	
	}
	
	if( shift(html())  || shift(somehtmlfrag()) || shift(seesinglecharacter()) )
	{
		/*
		do
		{
			if( htmlbuf_isclosing() )
			{
				// quick exit, a parent branch closed leaving us open
				// we must vacate this loop now
				break;
			}
		}		
		while( shift(html())  || shift(somehtmlfrag()) || shift(seesinglecharacter()) );
		*/
		return reduce(start,"somehtml");
	}
	//while( shift(html()) || shift(exprconst()) || shift(seeidentifier()) || shift(seesinglecharacter()))
	//..
	return revert(realstart);
}




int liqcell_parse_html(liqcell *self, const char *inputdata)
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


#ifdef __cplusplus
}
#endif

