
#ifndef SNO_SYS_H
#define SNO_SYS_H

#ifndef MAC

typedef unsigned char  byte;
typedef short          int16;
typedef unsigned short u_int16;
typedef int            int32;
typedef unsigned int   u_int32;

#else /* MAC */

typedef u_char  byte;
typedef short   int16;
typedef u_short u_int16;
typedef long    int32;
typedef u_long  u_int32;

#endif /* MAC */

#ifdef VAX
typedef unsigned long u_long;
#endif

#define BYTE

#endif /* not SNO_SYS_H */
