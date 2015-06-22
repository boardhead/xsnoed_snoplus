#ifndef __Utils_h__
#define __Utils_h__

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef VAX
#define Printf vaxPrintf	// avoid Printf/printf name conflict
#endif

void	setProgname(char *aFilename, char *aPathname);
double	double_time(void);
void	usleep_(unsigned long usec);
int		bc2int(char *s, char first);
char *	int2bc(int m, char first);
void	quit(char *msg);
int		Printf(char *,...);
void	SetPrintfOutput(char *buff, int size);

#ifdef  __cplusplus
}
#endif

#endif /* __Utils_h__ */
