/**
 *
 * $Id: DebugUtil.h,v 1.1 2000/02/03 15:05:07 phil Exp $
 * 
 * Copyright (C) 1995 Free Software Foundation, Inc.
 *
 * This file is part of the GNU LessTif Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 **/

#ifndef XM_DEBUGUTIL_H
#define XM_DEBUGUTIL_H

#include <LTconfig.h>
#include <Xm/Xm.h>

#ifdef STDC_HEADERS
#include <stdarg.h>
#define Va_start(a,b) va_start(a,b)
#else /* ! STDC_HEADERS */
#include <varargs.h>
#define Va_start(a,b) va_start(a)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Print a widget tree
 */
void XdbPrintTree(Widget w);
void XdbPrintCompleteTree(Widget w);

/*
 * Print an Arg list
 */
void XdbPrintArgList(char *fn, Widget w, ArgList al, int n, Boolean Get);

/*
 * Convert types into string format
 */
char *XdbFocusDetail2String(int type);
char *XdbFocusMode2String(int type);
char *XdbEventType2String(int type);
char *XdbGeoAction2String(int action);
char *XdbGeometryResult2String(XtGeometryResult r);
char *XdbWidgetGeometry2String(XtWidgetGeometry *g);

char *XdbAttachment2String(int i);
char *XdbMenuFocusOp2String(int f);
char *XdbMenuEnum2String(int f);
char *XdbBoolean2String(int b);
char *XdbXmString2String(XmString xms);
char *XdbPacking2String(unsigned char p);
char *XdbRcType2String(unsigned char t);
char *XdbAlignment2String(int n);
char *XdbMenuType2String(int n);
char *XdbNavigability2String(unsigned char n);
char *XdbHighlightMode2String(int mode);
char *XdbSelectionPolicy2String(int n);
char *XdbReason2String(int reason);
#ifdef XM_P_H
char *XdbFocusChange2String(XmFocusChange c);
#endif
char *XdbNavigationType2String(XmNavigationType nt);
char *XdbEditMode2String(int n);
char *XdbSBDisplayPolicy2String(int n);
char *XdbSBPlacement2String(int n);
char *XdbListSizePolicy2String(int n);
char *XdbResizePolicy2String(int n);

/*
 * Debug printing functions
 */
void XdbDebug(char *fn, Widget w, char *fmt, ...);
void XdbDebug2(char *fn, Widget w, Widget c, char *fmt, ...);
void XdbDebug0(char *fn, Widget w, char *fmt, ...);
void XdbPrintString(char *s);

#ifdef	LESSTIF_PRODUCTION
#define	XdbInDebug(x, y)	False
#define DEBUGOUT(x)

# ifdef	USE_DMALLOC
# undef	USE_DMALLOC
# endif
#else
Boolean XdbInDebug(char *fn, Widget w);
#define DEBUGOUT(x)	x
#endif

/*
 * Some stuff to produce sensible tracing with dmalloc.
 * Find dmalloc at http://www.letters.com/dmalloc/
 *	or	ftp://ftp.letters.com/src/dmalloc/
 *	or	ftp://gatekeeper.dec.com/pub/misc/dmalloc/
 */
#ifdef	USE_DMALLOC

#include <dmalloc.h>

#ifdef	XtMalloc
#undef	XtMalloc
#endif
#define	XtMalloc(x)	XdbMalloc(__FILE__, __LINE__, x)
#ifdef	XtCalloc
#undef	XtCalloc
#endif
#define	XtCalloc(x,y)	XdbCalloc(__FILE__, __LINE__, x, y)
#ifdef	XtRealloc
#undef	XtRealloc
#endif
#define	XtRealloc(x,y)	XdbRealloc(__FILE__, __LINE__, x, y)
#ifdef	XtFree
#undef	XtFree
#endif
#define	XtFree(x)	XdbFree(__FILE__, __LINE__, x)

#endif	/* USE_DMALLOC */

extern XtPointer	XdbMalloc(char *f, int l, int size);
extern XtPointer	XdbCalloc(char *f, int l, int count, int size);
extern XtPointer	XdbRealloc(char *f, int l, XtPointer p, int size);
extern void		XdbFree(char *f, int l, XtPointer p);

#ifdef __cplusplus
}
#endif


#endif /* XM_DEBUGUTIL_H */
