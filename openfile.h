#ifndef __openfile__
#define __openfile__

#include <stdio.h>

#define FILENAME_LEN    512

#ifdef  __cplusplus
extern "C" {
#endif

FILE *	openFile(char *name, char *mode, char *searchPath);
FILE *	openAltFile(char *name,char *mode,char *searchPath,char *altExt);
FILE *	createAltFile(char *name, char *mode, char *altExt);
FILE *	openPlainFile(char *name, char *mode);
char *	getOpenFileName(void);
char *	getOpenFileNameNoPath(void);
void    putOpenFileName( char *p );

#ifdef  __cplusplus
}
#endif

#endif
