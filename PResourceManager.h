#ifndef __PResourceManager_h__
#define __PResourceManager_h__

#include <stdio.h>
#include <Xm/Xm.h>
#include "PSpeaker.h"
#include "PProjImage.h"
#include "xsnoed.h"
#include "XSnoedResource.h"

const short	kMaxSettingsFilenameLen	= 256;

// data for translation callback
struct TranslationData {
	Widget		widget;		// the widget generating the callback
	XEvent    *	event;		// the event generating the callback
	String	  *	params;		// pointer to array of parameter strings
	Cardinal	num_params;	// number of parameters
};

struct SWindowGeometry {
	Dimension	x, y;
	Dimension	width, height;
};

class PResourceManager {
public:
	static void					InitApp();
	static void					InitResources(Widget toplevel);
	static void					WriteSettings(XSnoedResource *res, int force_save);
	static XrmOptionDescRec   *	GetCommandLineOptions();
	static int					GetNumOptions();
	static void					SetApp(XtAppContext anApp, Display *dpy, GC gc);
	static void					SetColours(int colorSet);
	static void					ListResources(Widget w);
	static Cursor				GetCursor(ECursorType aCursor)	{ return sResource.cursor[aCursor]; }
	static int					GetSettingsFilename(char *outName);
	static void					SetSettingsFilename(char *name);
	static void					SetWindowGeometry(char *name, SWindowGeometry *geo);
	static int					GetWindowGeometry(char *name, SWindowGeometry *geo);
	static int					GetWindowGeometry(Widget w, SWindowGeometry *geo);
	static void					GetResourceName(Widget w, char *buff);
	static XColor			  *	GetColour(int num) { return sColours + num; }
	static void					SetColour(int num, XColor *xcol);
	static void					SetWindowOffset(int dx, int dy);

#ifdef OPTICAL_CAL
	static void					GetOpticalParameters();
#endif

	static XSnoedResource		sResource;
	static PSpeaker			  *	sSpeaker;
	
	// X callbacks
	static void					Str2floatXm(XrmValue *args, Cardinal *nargs, XrmValue *fromVal, XrmValue *toVal);
	static void					TranslationCallback(Widget w, XEvent *ev, String *params,
											 Cardinal *num_params);
	
	static void					WritePaddedLabel(FILE *fp, char *object_name, char *res_name);

private:
	static void					FreeColours();
	static void					CopyColours();
	static float				ReadResourceVersion(char *buff);
	static void					LoadSettings();
	static int					VerifySettingsFile();
	static void					WriteWindowGeometries(FILE *dest_file);
	static void				 	AllocColours(int num, Pixel **col, int first,
											 int nseeds, int overscale, int extras);
	static void					FreeAllocatedColours(Pixel *col, int num);
											 
	static int					sResourceFileSaveConfig;
	static char				  *	sAllocFlags;
	static char					sSettingsFilename[kMaxSettingsFilenameLen];
	static XColor				sColours[2 * NUM_COLOURS];
	static char					sColoursAllocated[2 * NUM_COLOURS];
	static int					sWindowOffsetX;
	static int					sWindowOffsetY;
};

// made this extern "C" instead of a static member to make a dumb compiler happier - PH 05/08/00
extern "C" int WriteGeoProc(XrmDatabase *database,XrmBindingList bindings,XrmQuarkList quarks,
							XrmRepresentation *type,XrmValue *value, XPointer enumData);
							
#endif // __PResourceManager_h__
