/*
############################################################################
#                                                                          #
# Copyright (c) 1993-1994 CASPUR Consortium                                # 
#                         c/o Universita' "La Sapienza", Rome, Italy       #
# All rights reserved.                                                     #
#                                                                          #
# Permission is hereby granted, without written agreement and without      #
# license or royalty fees, to use, copy, modify, and distribute this       #
# software and its documentation for any purpose, provided that the        #
# above copyright notice and the following two paragraphs and the team     #
# reference appear in all copies of this software.                         #
#                                                                          #
# IN NO EVENT SHALL THE CASPUR CONSORTIUM BE LIABLE TO ANY PARTY FOR       #
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING  #
# OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE       #
# CASPUR CONSORTIUM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    #
#                                                                          #
# THE CASPUR CONSORTIUM SPECIFICALLY DISCLAIMS ANY WARRANTIES,             #
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY #
# AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER   #
# IS ON AN "AS IS" BASIS, AND THE CASPUR CONSORTIUM HAS NO OBLIGATION TO   #
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.   #
#                                                                          #
#       +----------------------------------------------------------+       #
#       |   The ControlHost Team: Ruten Gurin, Andrei Maslennikov  |       #
#       |   Contact e-mail      : ControlHost@caspur.it            |       #
#       +----------------------------------------------------------+       #
#                                                                          #
############################################################################
*/

/*
 $Id: dispatch.h,v 1.1.1.1 1999/01/07 18:43:26 qsno Exp $
*/

/* High level dispatcher communications interface */

#define TAGSIZE 8

#if defined(VM) || defined(MVS)
#include "ascebcd.h"

#define init_disp_link(a,b) idisplink(a,b)
#define init_2disp_link(a,b,c) i2displink(a,b,c)
#define get_data(a,b) getdata(a,b) /* to avoid overlaps with get_data_addr */
#define send_me_always() smealways()
#define send_me_next() smenext()
#define put_fulldata(a,b,c) putfdata(a,b,c)
#define put_fullstring(a,b) putfstring(a,b)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int init_disp_link(const char *host, const char *subscr);
int resubscribe(const char *subscr);
int set_skip_mode(const char *mode);
int wait_head(char *tag, int *size);
int check_head(char *tag, int *size);
int get_data(void *buf, int lim);
int get_string(char *buf, int bufsize);
int put_data(const char *tag, const void *buf, int size, int *pos);
int put_string(const char *tag, const char *buf, int *pos);
int put_fulldata(const char *tag, const void *buf, int size);
int put_fullstring(const char *tag, const char *buf);
int put_zfullstring(const char *tag, const char *buf); /* string with final zero */
int my_id(const char *id);
int unique_id(const char *id);
int connected(void);
void drop_connection(void);
int whereis(const char *host, const char *id, char *reply, int maxreplen);
void *get_data_addr(void);
void unlock_data(void);

#if defined(VM) || defined(MVS)
int send_me_always();
int send_me_next();
#else
int send_me_always(void);
int send_me_next(void);
#endif

#ifndef OS9
int init_2disp_link(const char *host, const char *subscrdata, const char *subscrcmd);
#endif

#ifdef __cplusplus
  }
#endif
