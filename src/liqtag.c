


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "liqapp.h"
#include "liqtag.h"


liqtagcloud *system_tagcloud = NULL;





//#########################################################################
//#########################################################################
//######################################################################### liqtagleaf
//#########################################################################
//#########################################################################



liqtagleaf *liqtagleaf_new()
{
	liqtagleaf *self = (liqtagleaf *)calloc(sizeof(liqtagleaf),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqtagleaf new failed" ); return NULL; }
	// NULL everything
	memset((char *)self,0,sizeof(liqtagleaf));
	self->usagecount=1;
	return self;
}

liqtagleaf * liqtagleaf_hold(liqtagleaf *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}

void liqtagleaf_release(liqtagleaf *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqtagleaf_free(self);
}

void liqtagleaf_free(liqtagleaf *self)
{
	free(self);
}



//#########################################################################
//#########################################################################
//######################################################################### liqtagnode
//#########################################################################
//#########################################################################



liqtagnode *liqtagnode_new()
{
	liqtagnode *self = (liqtagnode *)calloc(sizeof(liqtagnode),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqtagnode new failed" ); return NULL; }
	// NULL everything
	memset((char *)self,0,sizeof(liqtagnode));
	self->usagecount=1;
	return self;
}

liqtagnode * liqtagnode_hold(liqtagnode *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}

void liqtagnode_release(liqtagnode *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqtagnode_free(self);
}

void liqtagnode_free(liqtagnode *self)
{
	free(self);
}


//#########################################################################
//#########################################################################

int liqtagnode_clear(liqtagnode *self)										// Clear the node
{
	//while(self->leaffirst)
	{
		// ...
	}
	return 0;
}

liqtagleaf *liqtagnode_findleaf(liqtagnode *self,char *itemkey)						// check if itemkey is used in this node
{
	liqtagleaf *leaf = self->leaffirst;
	while(leaf)
	{
		if(stristr(itemkey,leaf->key))
		{
			return leaf;
		}
		leaf = leaf->linknext;
	}
	return NULL;
}


liqtagleaf *liqtagnode_findorcreateleaf(liqtagnode *self,char *leafkey, char *leafdata)						// insert items into the node
{
	liqtagleaf *leaf = liqtagnode_findleaf(self,leafkey);
	if(!leaf)
	{
		leaf = liqtagleaf_new();
		leaf->key = strdup( leafkey );
		leaf->filename = strdup(leafdata);
		leaf->linknext = self->leaffirst;
		self->leaffirst = leaf;
		if(!self->leaflast) self->leaflast = leaf;
		if(leaf->linknext)
		{
			leaf->linknext->linkprev = leaf;
		}
		self->leafcount++;
	}
	return leaf;
}


//#########################################################################
//#########################################################################
//######################################################################### liqtagcloud
//#########################################################################
//#########################################################################



liqtagcloud *liqtagcloud_new()
{
	liqtagcloud *self = (liqtagcloud *)calloc(sizeof(liqtagcloud),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqtagcloud new failed" ); return NULL; }
	// NULL everything
	memset((char *)self,0,sizeof(liqtagcloud));
	self->usagecount=1;
	return self;
}

liqtagcloud * liqtagcloud_hold(liqtagcloud *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}

void liqtagcloud_release(liqtagcloud *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqtagcloud_free(self);
}

void liqtagcloud_free(liqtagcloud *self)
{
	free(self);
}

//#########################################################################
//#########################################################################

liqtagnode *liqtagcloud_findnode(liqtagcloud *self,char *nodekey)							// check if itemkey is used in this cloud
{
	liqtagnode *node = self->nodefirst;
	while(node)
	{
		if(strcasecmp(nodekey,node->key)==0)
		{
			// returned a value
			return node;
		}
		node = node->linknext;
	}
	return NULL;
}


int liqtagcloud_containsleaf(liqtagcloud *self,char *leafkey)							// check if itemkey is used in this cloud
{
	//
	liqtagnode *node = self->nodefirst;
	while(node)
	{
		if(liqtagnode_findleaf(node,leafkey))
		{
			// returned a value
			return 1;
		}
		node = node->linknext;
	}
	return 0;
}
liqtagnode *liqtagcloud_findorcreatenode(liqtagcloud *self,char *tagname)						// insert items into the node
{
	liqtagnode *node = liqtagcloud_findnode(self,tagname);
	if(!node)
	{
		node = liqtagnode_new();
		node->key = strdup(tagname);
		node->linknext = self->nodefirst;
		self->nodefirst = node;
		if(!self->nodelast) self->nodelast = node;
		if(node->linknext)
		{
			node->linknext->linkprev = node;
		}
		self->nodecount++;
	}
	return node;
}










//#########################################################################
//#########################################################################



static int tagnode_save(liqtagnode *self)
{
	// only persist nodes at a time.

	char *tagtitle = self->key;

	char filedate[256] = "";
	char filename[FILENAME_MAX] = "";
	liqapp_formatnow(filedate,sizeof(filedate),"yyyymmdd_hhmmss");
	snprintf(filename,sizeof(filename), "%s/tags/liq.%s.%s.tag.%s",    app.userdatapath,    filedate,    app.username,  tagtitle  );
		



	liqapp_log("tagnode_save, saving to '%s'",filename);

	FILE *fd;
	//int   ri;
	fd = fopen(filename, "w");
	if(fd==NULL){ liqapp_log("tagnode_save, cannot open '%s' for writing",filename); return -1; }
	// actual file data

	liqapp_log("tagnode_save, writing head");

	fprintf(fd,									"tag:'%s'\n",
																						tagtitle
																						);
	liqapp_log("tagnode_save, writing leaves");
	liqtagleaf *leaf = self->leaffirst;
	while(leaf)
	{
		
		fprintf(fd,								"\tfile:'%s', '%s'\n",
																						leaf->key, 
																						leaf->filename
																						);
		leaf = leaf->linknext;
	}
	
	liqapp_log("tagnode_save, closing");			
	fclose(fd);
	liqapp_log("tagnode_save, finished");
	return 0;
}





//#########################################################################
//#########################################################################



int liqtagnode_fileload_memstream(liqtagnode *self,char *filename,char *srcdata, int srcsize)
{



//	liqapp_log("liqtagnode_fileload '%s'",filename);
	char *indat;
	//int err=0;
//	liqapp_log("liqtagnode_fileload 2 '%s'",filename);
	if(self->filename) { free(self->filename); self->filename=NULL; }


	int linenum=1;
	FILE *fn=NULL;
	if(!srcdata)
	{
		self->filename = strdup(filename);
	
		fn=fopen(filename,"r");
		if(!fn)
		{
			liqapp_log("liqtagnode_fileload could not open '%s'",filename);
			return -1;
		}
		
	}

	
	int srcpos=0;
	char lineraw[512];
	int linemax=511;
	//char *line=NULL;
	while(  (fn && !feof(fn)) || ( (!fn) && (srcpos<srcsize) && (srcdata[srcpos]) ) )
	{
		char * rc;
		
		if(fn)
		{
		
			rc=fgets(lineraw,linemax, (FILE*) fn);
			if(!rc)break;
		}
		else
		{
			char *ss = &srcdata[srcpos];
			char *pp = strchr(ss,'\n');
			char *tt = pp;
			if(!tt)tt=&srcdata[srcsize-1];
			int cnt=(tt-ss);
			if(cnt>512) cnt=512;
			
			if(tt)
			{
				srcpos += cnt;
				strncpy(lineraw,ss,cnt);
				lineraw[cnt]=0;
				lineraw[sizeof(lineraw)-1]=0;
				//if(pp)srcpos++;
				while( srcdata[srcpos]==10 || srcdata[srcpos]==13 )srcpos++;
			}
			//liqapp_log("mem read: '%s', cnt=%i,sp=%i",lineraw,cnt,srcpos);
		}


		if(linenum==1)
		{
			
						
			if(strncmp(lineraw,"tag:",4) != 0)
			{
				// invalid header
				if(fn)fclose(fn);
				{ return liqapp_warnandcontinue(-1,"liqtagnode_fileload invalid file header"); }						
		
			}				
						
		}
		
		indat=lineraw;
		// proof of concept
		// load in the points first
		// should be MUCH faster
		int indentlevel=0;
		while(*indat==9)
		{
			indentlevel++;
			indat++;
		}
		int isdone=0;		// use this to save some time loading (skips the other scanf's after matching one)

		
		{
			char fk[64] = "";
			char fn[1024]="";

			int res = sscanf(indat,"file:'%64s '%1024s",fk,fn);
			if(res==2)
			{
				fk[ strlen(fk)-2 ] = 0;
				fn[ strlen(fn)-1 ] = 0;
				//fk = liqapp_filename_walkoverpath(fn);
//				liqapp_log("%4i ++file ++ %i '%s' == [[%s]] [[%s]] ",linenum,res,indat,fk,fn);
				// search and see if wwe already know of this node.
				// use fk for that search pattern
				// create a node if not.

				liqtagnode_findorcreateleaf(self, fk, fn);


				
				isdone=1;

			}
			else
			{
//				liqapp_log("%4i --file -- %i '%s' == [[%s]] [[%s]]",linenum,res,indat,fk,fn);
			}		
		
		}
		if(!isdone)
		{
			char tk[64]="";
			int res = sscanf(indat,"tag:'%64s",tk);
			if(res==1)
			{
				tk[ strlen(tk)-1 ] = 0;
				const char *tagname = tk;
//				liqapp_log("%4i ++tag  ++ %i '%s' == [[%s]]",linenum,res,indat,tagname);
				// compare tk against our tagkey
				// they should match.
				isdone=1;
			}
		}
		linenum++;
	}
	if(fn)fclose(fn);
	return 0;
}

int liqtagnode_fileload(liqtagnode *self,char *filename)
{
	return liqtagnode_fileload_memstream(self,filename,NULL,0);
}







//#########################################################################
//#########################################################################













	static int liqtagcloud_scan(liqtagcloud *self,char *path)
	{
		char *widgetpath = path;
		DIR           *	dir_p;
		struct dirent *	dir_entry_p;
		char 			fn[FILENAME_MAX+1];
		char          * ft;
		dir_p = opendir( widgetpath );			
		if(!dir_p)
		{
			liqapp_log("liqtagcloud_scan opendir failed: '%s'",widgetpath);
			return -1;			// heh thanks kot :)
		}				
		while( NULL != (dir_entry_p = readdir(dir_p)))
		{
			if( dir_entry_p->d_name[0]=='.' )
				continue;
		
			ft=dir_entry_p->d_name;
		
			snprintf(fn , FILENAME_MAX , "%s/%s", widgetpath , ft);
		
			struct stat     statbuf;
			if(stat(fn, &statbuf) == -1)
			{
				liqapp_log("liqtagcloud_scan stat failed: '%s'",fn);
				return -1;
			}
			// got the information we need
			if ( S_ISREG(statbuf.st_mode) )
			{
				const char *ext=liqapp_filename_walktoextension(ft);
				if(!ext || !*ext)
				{
					// nothing to see here..
				}
				else
				if(	stristr(ft,"liq.") && stristr(ft,".tag.") )
				{
//					liqapp_log("liqtagcloud_scan matched tag file: '%s'",fn);
				
					liqtagnode *node = liqtagcloud_findorcreatenode(self, (stristr(ft,".tag.")+5) );

//					liqapp_log("liqtagcloud_scan loading tag file: '%s'",fn);

					liqtagnode_fileload( node, fn );
					
				}
			}
		}
		closedir(dir_p);
		return 0;
	}



int liqtagcloud_load(liqtagcloud *self)				// load up the cloud
{
	// scan tags folder
	// load each node
	// merge contents from named
	char filepathbase[FILENAME_MAX] = "";
	snprintf(filepathbase,sizeof(filepathbase), "%s/tags",    app.userdatapath  );

	liqtagcloud_scan(self,filepathbase);
	return 0;
}


int liqtagcloud_systemstart()				// load up the cloud
{
	liqapp_log("liqtagcloud_ayatem starting :)");
	if(system_tagcloud) return -1;
	system_tagcloud = liqtagcloud_new();
	liqtagcloud_load(system_tagcloud);
	liqapp_log("liqtagcloud_ayatem ready.");
	return 0;


	// try things like:

	// to check whether a file is tagged (for none insertion test..)
	// liqtagcloud_containsleaf( system_tagcloud, [filename] );

	
}


