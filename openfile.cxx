/* file utilites for xsnoed - PH 12/11/98 */

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "openfile.h"

static char openFileName[FILENAME_LEN];

extern char *	progpath;

/*
** Try to open file first in the default directory,
** then in the directory specified in the command line (progpath --
** where the executable lives), then the specified search directory,
** and finally in all directories of the path statement.
** - The name of the opened file is available from getOpenFileName()
*/
FILE *openFile(char *name, char *mode, char *searchPath)
{
	int		i, n;
	FILE	*fp;
	char	*pt2;
	char	*path;

	/* first try to open the specified file as named */
	strcpy(openFileName,name);
	fp = openPlainFile(openFileName,mode);
	if (!fp) {
		/* remove any path specification from 'name' */
		pt2 = strrchr(name,'/');
		if (pt2) name = pt2 + 1;
		/* look for file in the same directory as the executable */
		if (!fp && progpath) {
			strcpy(openFileName,progpath);
			pt2 = strrchr(openFileName,'/');
			if (pt2) {
				strcpy(pt2+1,name);
				fp = openPlainFile(openFileName,mode);
			}
		}
		/* look for file in 'searchPath' and 'PATH' */
		for (i=0; i<2 && !fp; ++i) {
			if (i==0) {
				path = searchPath;
			} else {
				path = getenv("PATH");
			}
			if (!path) continue;
			for (;;) {
				pt2 = strchr(path,':');
				if (!pt2) pt2 = strchr(path,0);
				n = pt2 - path;
				if (n) {
					memcpy(openFileName,path,n);
					/* add trailing '/' if necessary */
					if (openFileName[n-1] != '/') {
						openFileName[n++] = '/';
					}
					strcpy(openFileName+n,name);
					fp = openPlainFile(openFileName,mode);
					if (fp) break;
				}
				if (!(*pt2)) break;
				path = pt2 + 1;
			}
		}
	}
	return(fp);
}

/* creates an alternate file by adding altExt to the name
** - overwrites existing files
** - The name of the opened file is available from getOpenFileName()
*/
FILE *createAltFile(char *name, char *mode, char *altExt)
{
	char	buff[512];
	
	strcpy(buff,name);
	strcat(buff,altExt);
	strcpy(openFileName, buff);	/* save new name */
	return(fopen(buff,mode));
}

/* opens an alternate file by adding altExt to the name
** - The name of the opened file is available from getOpenFileName()
*/
FILE *openAltFile(char *name,char *mode,char *searchPath,char *altExt)
{
	char	buff[512];

	strcpy(buff,name);
	strcat(buff,altExt);
	return(openFile(buff,mode,searchPath));
}

/*
** Open a specified file.
** Make sure it is a plain file (not a directory, etc) if opened for reading
*/
FILE *openPlainFile(char *name, char *mode)
{
#ifndef STAT_BUG
	struct stat	file_stat;

	if (mode[0]=='r') {
		if (stat(name,&file_stat)) return((FILE *)0);
		if (!(file_stat.st_mode & S_IFREG)) return((FILE *)0);
	}
#endif /* STAT_BUG */
	return(fopen(name,mode));
}

/* get full pathname of last file opened */
char *getOpenFileName(void)
{
	return(openFileName);
}

/* get name of last file opened with path removed */
char *getOpenFileNameNoPath(void)
{
	char *pt = strrchr(openFileName,'/');
	if (pt) ++pt;
	else pt = openFileName;
	return(pt);
}

/* Force a string int `name of last opened file' */
void putOpenFileName( char *p )
{
	strcpy( openFileName, p );
	return;
}

