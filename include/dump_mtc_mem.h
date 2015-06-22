#ifndef DUMP_MTC_MEM_H
#define DUMP_MTC_MEM_H
 
#ifndef lint
static char DUMP_MTC_MEM_ID[] = "$Id: dump_mtc_mem.h,v 1.1.1.1 1999/01/07 18:43:26 qsno Exp $";
#endif /* lint */

#include "include/sno_sys.h"

#define N_TRIGS   1024       /* Max number of trigger words to read at once */
// below is *just* LSBs of this 53 bit counter below, beware!!!!
#define UNPK_MTC_C10(a)  (*a)
#define UNPK_MTC_C50(a)  (((*(a+1) >> 21) & 0x7ff) | ((*(a+2) & 0x1fffff) << 11))
#define UNPK_MTC_GT_ID(a) ((*(a+3) & 0xffffff))
#define UNPK_MTC_TRWORD(a) (((*(a+3) >> 24) & 0xff) | ((*(a+4) & 0x7ffff)) << 8)
#define UNPK_MTC_ANALME(a) (((*(a+4) >> 19) & 0x3ff) | ((*(a+5) & 0x1ffff)) << 13)
#define UNPK_MTC_ERR(a)   ((*(a+5) >> 17) & 0xfff)
 


int32 dump_mem(int32 n, u_int32 *trig_buf);
/* fdump_mem(n, trig_buf)
 * dump unpacked memory contents to a file. User defines output format.
 *
 * M. Neubauer 11 Sept 1997
 */

 
#endif /* DUMP_MTC_MEM_H */

