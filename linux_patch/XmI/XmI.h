/**
 *
 * $Id: XmI.h,v 1.1 2000/02/03 15:05:07 phil Exp $
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

#ifndef _XM_I_H
#define _XM_I_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * LessTif-specific functions/variables.  Use at the cost of incompatibility
 * with Motif 1.2
 * YOU SHOULD NOT CALL CALL THESE FUNCTIONS IF YOU DON'T KNOW WHAT YOU'RE
 * DOING!!
 * Correction: Some of these functions are totally undocumented Motif 1.2
 * calls.  WE don't know if we got them right, so you'd better not bank
 * on them.
 */

#include <Xm/XmP.h>
#include <Xm/ScreenP.h>
#include <Xm/ManagerP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/GadgetP.h>
#include <Xm/DrawP.h>
#include <Xm/RowColumnP.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <XmI/MacrosI.h>

/*
 * extra resources
 */
#ifndef XmNdefaultVirtualBindings
#define XmNdefaultVirtualBindings	"defaultVirtualBindings"
#endif

#define _XA_MOTIF_DEFAULT_BINDINGS	"_MOTIF_DEFAULT_BINDINGS"

/*
 * STRING AND FONTLIST INTERNALS
 */
struct _XmFontListRec {
    char *tag;
    XmFontType type;
    XtPointer font;
};

struct _XmFontListContextRec {
    XmFontList fontlist;
    int current_entry;
};

struct __XmStringExtRec {
    unsigned char tag;
    unsigned char len;
    unsigned char data[1];
};

struct __XmStringComponentRec {
    XmStringComponentType type;
    int length;
    char *data;
    short font;
};
typedef struct __XmStringComponentRec _XmStringComponentRec, *_XmStringComponent;

struct __XmStringRec {
    struct __XmStringComponentRec **components;
    int number_of_components;
};

struct __XmStringContextRec {
    struct __XmStringRec *string;
    int current_component;
};

/*
 * NOTE: The first two fields in this structure MUST match those
 * in struct __XmStringContextRec!!!
 */
struct _XmtStringContextRec {
    struct __XmStringRec *string;
    int current_component;
    char *text;
    short textlen;
    char *charset;
    XmStringDirection direction;
    Boolean separator;
};

/* ADDED FOR EXTERNAL FORM */
#define XmSTRING_COMPONENT_XMSTRING     (XmSTRING_COMPONENT_LOCALE_TEXT + 1)
#define XmSTRING_TAG                    0xDFU
#define XmSTRING_LENGTH                 0x80U

/*
 * XmIm stuff
 */
typedef unsigned char XmIMInputPolicy;

#define	IP_INHERIT_POLICY	0
#define	IP_PER_WIDGET		1
#define	IP_PER_SHELL		2

/*
 * GENERIC PROTOTYPES
 */
XmScreenInfo *_XmGetScreenInfo(Widget w);

String _XmMakeDialogName(String name);
void _XmError(Widget w, char *message, ...);

/*
 * Dimension variables (below) are typed as int because Dimension is
 * unsigned. However, we need to check whether they become negative, hence
 * the theoretically incorrect type.
 * We don't do GetValues on them so we should be ok.
 */
typedef struct
{
    /* NOTE: The next four lines ABSOLUTELY MUST MATCH the first
     * three lines in XmMWValues!!!!!!! */
    Boolean   ShowVSB, ShowHSB, HasHSB, HasVSB;
    Position  HsbX, HsbY, VsbX, VsbY, ClipX, ClipY, WorkX, WorkY;
    int       HsbW, HsbH, VsbW, VsbH, ClipW, ClipH, WorkW, WorkH;
    int       SwY, SwW, SwH;
}
XmSWValues;

typedef struct
{
    /* NOTE: The next four lines ABSOLUTELY MUST MATCH the first
     * three lines in XmSWValues!!!!! */
    Boolean   ShowVSB, ShowHSB, HasHSB, HasVSB;
    Position  HsbX, HsbY, VsbX, VsbY, ClipX, ClipY, WorkX, WorkY;
    int       HsbW, HsbH, VsbW, VsbH, ClipW, ClipH, WorkW, WorkH;
    int       SwY, MwW, MwH;
    int       mbw, mbh, cww, cwh, mww, mwh, www, wwh;
    int       s1w, s1h, s2w, s2h, s3w, s3h;
    Position  mbx, mby, cwx, cwy, mwx, mwy, wwx, wwy;
    Position  s1x, s1y, s2x, s2y, s3x, s3y;
}
XmMWValues;

/* default procs */
void _XmCascadePixmapDefault(Widget w, int offset, XrmValue *val);

/* Find the VendorShell Extension Object */
extern Widget _LtFindVendorExt(Widget);

/*
 * fontlist prototypes
 */
XmFontList _XmFontListCreateDefault(Display *);
XmFontListEntry _XmFontListEntryFromTag(XmFontList fontlist, char *tag);

/* for vendor */
extern void _XmInitProtocols(Widget w);
extern void _XmDestroyProtocols(Widget w);
#ifdef LESSTIF_EDITRES
extern void _XmNSEEditResCheckMessages(Widget w, XtPointer data,
				       XEvent *event, Boolean *cont);
#endif

/* Things for Label/LabelG */

extern void _XmLabelGetPixmapSize(Widget w, Pixmap Pix, Dimension *width, Dimension *height);

/* For buttons */
#define ACTIVATE_DELAY	100

/* GeomUtils : A few of these I'm not sure of*/
extern XtGeometryResult _XmGMReplyToQueryGeometry(Widget w,
						XtWidgetGeometry *request,
						XtWidgetGeometry *reply);
extern XtGeometryResult _XmGMHandleQueryGeometry(Widget w,
						 XtWidgetGeometry *proposed,
						 XtWidgetGeometry *answer,
				    		 Dimension margin_width,
						 Dimension margin_height,
						 unsigned char resize_policy);
extern void _XmGMEnforceMargin(Widget w,
			       Dimension margin_width,
			       Dimension margin_height,
			       Boolean useSetValues);
extern void _XmGMCalcSize(Widget w,
			  Dimension margin_w, Dimension margin_h,
			  Dimension *retw, Dimension *reth);
extern void _XmGMDoLayout(Widget w,
			  Dimension margin_w, Dimension margin_h,
			  unsigned char resize_policy, short adjust);
extern XtGeometryResult _XmGMHandleGeometryManager(Widget w, Widget instigator,
						   XtWidgetGeometry *desired,
						   XtWidgetGeometry *allowed,
						   Dimension margin_width,
						   Dimension margin_height,
						   unsigned char resize_policy,
						   Boolean allow_overlap);
extern Boolean _XmGMOverlap(Widget w, Widget instigator,
			    Position x, Position y,
			    Dimension width, Dimension height);

/* for DialogS.c */
extern void _XmBbMap(Widget w);
extern void _XmBbUnmap(Widget w);

/* for ImageCache */
extern void _XmSetupImageCache(void);

/* from MenuUtil.c */
extern void _XmFakeExpose(Widget menu_shell);
extern void _XmSetInPMMode(Widget w, Boolean flag);
extern Boolean _XmGetInPMMode(Widget w);

/* from RowColumn.c */
#if 0
extern void _XmFixOptionMenu(Widget, Boolean);
#endif
extern void _XmRCSetKidGeo(XmRCKidGeometry kg, Widget instigator);

/* used as the operation parameter for _XmMenuFocus. */
enum {
  XmMENU_FOCUS_SAVE=0,
  XmMENU_FOCUS_RESTORE,
  XmMENU_FOCUS_SET
};

/* from MessageB.c */
extern void _XmMessageBoxInstallImages(Widget w);

/* from misc (and for primitives and gadgets) */
extern void _XmInstallStippleImages(Widget w);

#define XmEVEN_STIPPLE_IMAGE	"xm_even_stipple"
#define XmODD_STIPPLE_IMAGE	"xm_odd_stipple"

/* from ResInd */
extern void _XmExportXmString(Widget w, int offset, XtArgVal *value);
extern void _XmExportString(Widget w, int offset, XtArgVal *value);

/* from Manager.c */
#ifndef MCEPTR
#define MCEPTR(cl) \
    ((XmManagerClassExt *)(&(((XmManagerWidgetClass)(cl))->manager_class.extension)))
#endif
#ifndef _XmGetManagerClassExtPtr
#define _XmGetManagerClassExtPtr(cl, o) \
    ((*MCEPTR(cl) && (((*MCEPTR(cl))->record_type) == (o))) \
        ? MCEPTR(cl) \
        : ((XmManagerClassExt *)_XmGetClassExtensionPtr(((XmGenericClassExt *)MCEPTR(cl)), (o))))
#endif

extern void _XmManagerInstallAccelerator(Widget m, Widget w, String s);
extern void _XmManagerInstallMnemonic(Widget m, Widget w, KeySym mn);
extern void _XmManagerUninstallAccelerator(Widget m, Widget w);
extern void _XmManagerUninstallMnemonic(Widget m, Widget w);

/* for ScrolledW */
/* T. Straumann: */
extern void _XmScrolledWPreferredSize(Widget w, Widget instigator,
				      XtWidgetGeometry *instigator_geom, XmSWValues * vals);
extern void _XmScrolledWLayout(Widget w, Widget instigator,
				      XtWidgetGeometry *instigator_geom, XmSWValues * vals);
extern void _XmScrolledWConfigureChildren(Widget w, Widget instigator,
				      XtWidgetGeometry *instigator_geom, XmSWValues * vals);

/* from SelectionBox.c */
extern Boolean _XmSelectionBoxMatch(XmSelectionBoxWidget w);


/* for TearOff */
extern void _XmPushButtonSetTranslation(Widget, int);

/* TearOff */
extern void _XmTearOffInitiate(Widget w, XEvent *event);

/* Text */
void _XmChangeVSB(XmTextWidget w, XmTextPosition pos);
void _XmRedisplayHBar(XmTextWidget w, int offset);

/* Traversal */
extern void _XmSetFocusResetFlag(Widget w, Boolean value);
extern Boolean _XmGetFocusResetFlag(Widget w);
extern Widget _XmGetActiveTabGroup(Widget widget);
extern Widget _XmFindTopMostShell(Widget widget);

/* VirtKeys */
/*
 * This is for handling of ALT, META, and other modifier keys when parsing
 * the virtual binding(s). In contrast to the closed software foundation,
 * LessTif is able to configure the current setting of modifiers and to
 * adjust itself to the right modifier masks.
 * USING THIS STUFF WILL MAKE YOUR APPLICATION MORE USEFUL BUT WILL BREAK
 * THE COMPATIBILITY. THIS IS PRIMARILY FOR INTERNAL USE.
 */
typedef enum _XmModifierLevels {
    ALTModifier = 0,
    METAModifier,
    SUPERModifier,
    HYPERModifier,
    /* This one must be the last! */ MAX_MODIFIERS
} XmModifierLevels;

typedef Modifiers XmModifierMaskSet[MAX_MODIFIERS];
typedef Modifiers *XmModifierMaskSetReference;

extern XmModifierMaskSetReference _XmGetModifierMappingsForDisplay(Display *Dsp);
extern void _XmInvalidateModifierMappingsForDisplay(Display *Dsp);
extern void _XmRefreshVirtKeys(Widget w);

/* Visual */

extern void _XmAddBackgroundToColorCache(Screen *screen, Colormap color_map,
					 String background_spec,
					 int rgb_fallback, XrmValue *val);
extern void _XmInvalidateColorCache(Boolean default_only);

/* XmString */

extern void _XmStringBaselines(XmFontList fontlist, _XmString string,
			       Position y, Dimension *baselines);

/*
 * Behaviour control for XmFormLayout, XmRowColumnLayout.
 * This is their second parameter.
 */
#define Mode_Normal     0x00    
#define Mode_Test       0x01    
#define Mode_Resize     0x02

/*
 * Extra color tools.
 */
extern Pixel _XmWhitePixelOfObject(Widget);
extern Pixel _XmBlackPixelOfObject(Widget);

/*
 * Thread stuff (lib/Xm/misc.c).
 */
extern void _XmAppLock(XtAppContext appc);
extern void _XmAppUnlock(XtAppContext appc);
extern void _XmProcessLock(void);
extern void _XmProcessUnlock(void);
extern void _XmObjectLock(Widget w);
extern void _XmObjectUnlock(Widget w);

#ifdef __cplusplus
}
#endif



#endif /* _XM_I_H */
