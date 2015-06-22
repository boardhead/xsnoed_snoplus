#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#ifdef USE_FTIME
#include <sys/timeb.h>
#elif defined(USE_GETTIMEOFDAY)
#include <sys/time.h>
#include <unistd.h>
#endif
#include "CUtils.h"

char *progname = "xsnoed";
char *progpath = "xsnoed";

#ifdef VAX
unsigned long LIB$WAIT( float* );
#endif

/*---------------------------------------------------------------------------
*/
void setProgname(char *aFilename, char *aPathname)
{
	progname = aFilename;
	progpath = aPathname;
}

/*
 * get current accurate time in seconds using system clock - PH 02/01/98
 */
double double_time(void)
{
#ifdef USE_FTIME
	/* support for systems without clock_gettime() - PH 08/12/98 */
	struct timeb tb;
	ftime(&tb);
	return(tb.time + 1e-3 * tb.millitm);
#elif defined(USE_GETTIMEOFDAY)
	/* add support for systems without clock_gettime() or ftime() - PH 01/20/99 */
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	return(tv.tv_sec + 1e-6 * tv.tv_usec);
#else
	struct timespec ts;
	/* (rumoured to be accurate to about 25ms) */
	clock_gettime(CLOCK_REALTIME, &ts);
	return(ts.tv_sec + 1e-9 * ts.tv_nsec);
#endif
}


/* portable sleep routine */
void usleep_(unsigned long usec)
{
#ifdef USE_NANOSLEEP
	struct timespec ts;
	ts.tv_sec = usec / 1000000UL;
	ts.tv_nsec = (usec - ts.tv_sec * 1000000UL) * 1000;
	nanosleep(&ts,NULL);
#elif VAX
	float secs;
	secs = usec * 0.000001;
	LIB$WAIT(&secs);
#else
	usleep(usec);
#endif
}


/* convert a 4-character barcode to an integer */
int bc2int(char* bc, char first)
{
	int	n;
	if( *bc != first ) return( -1);
	for( n=1;  n<4; ++n ) if( bc[n]<'A' || bc[n]>'Z' ) return( -1);
	n = ( ( bc[1]-'A' ) *26 + bc[2]-'A' ) * 26 + bc[3]-'A';
	return( n<0 ? -1 : n );
}

/* convert an integer to a 4 character barcode */
char *int2bc(int barcode, char first)
{
	static char bc[5];
	if( barcode<0 ) {
		strcpy( bc, "    ");
	} else {
		bc[4] = 0;
		bc[3] = barcode%26 + 'A';
		bc[2] = (barcode/26)%26 + 'A';
		bc[1] = (barcode/676)%26 + 'A';
		bc[0] = first;
	}
	return( bc );
}

/* Printf buffer variables */
static char *sPrintfBuffer = NULL;
static int	 sPrintfBufferMax = 0;
static int	 sPrintfBufferLen = 0;

void SetPrintfOutput(char *buff, int size)
{
	sPrintfBuffer = buff;
	sPrintfBufferMax = size;
	sPrintfBufferLen = 0;
	if (buff && size) *buff = '\0';	/* start with null string in buffer */
}

#ifdef VAX
// necessary to avoid conflict with printf on VAX
int vaxPrintf(char *fmt,...)
#else
int Printf(char *fmt,...)
#endif
{
	int		len = 0;
	va_list	varArgList;
	
	va_start(varArgList,fmt);
	if (sPrintfBuffer) {
		/* check to see if buffer is getting too full */
		if (sPrintfBufferLen + 80 > sPrintfBufferMax) {
			printf("%s: Printf buffer too full!\n", progname);
		} else {
			/* add output to buffer */
			char *pt = strchr(sPrintfBuffer, '\0');
			len = sprintf(pt,"%s: ",progname);
			len += vsprintf(pt+len,fmt,varArgList);
			sPrintfBufferLen = pt - sPrintfBuffer + len;
		}
	} else {
		len = printf("%s: ", progname);
		len += vprintf(fmt,varArgList);
	}
	va_end(varArgList);
	return(len);
}

void quit(char *msg)
{
	printf("%s: %s\n",progname,msg);
	exit(0);
}

