#include <X11/StringDefs.h>
#include <Xm/RowColumn.h>
#ifdef CHILD_WINDOWS
#include <Xm/MwmUtil.h>		// for MWM_DECOR_ definitions
#endif
#include <string.h>
#include "PWindow.h"
#include "PMenu.h"
#include "PResourceManager.h"
#include "PSpeaker.h"
#include "XSnoedWindow.h"
#include "xsnoed.h"
#ifdef LESSTIF
#include "xsnoedstream.h"
#endif

char	  *	PWindow::sWindowClass 		= "PWindow";
int			PWindow::sWindowDirty		= 0;
PWindow	  *	PWindow::sMainWindow		= NULL;
int			PWindow::sOffsetDone		= 0;

//----------------------------------------------------------------------------------------
// PWindow functions
//
PWindow::PWindow(ImageData *data,Widget shell,Widget mainPane)
	   : mData(data)
{
	mNextMainWindow = NULL;
	mMenu = NULL;
	mResizeWidget = NULL;
	mWasResized = 0;
	mDirty = 0x01;
	mVisible = 0;
	SetShell(shell);		// this will add a destroy callback
	SetMainPane(mainPane);
	if (data) {
		SetDirty();
	}
}

PWindow::~PWindow()
{
	// if our shell is NULL, then we have arrived here after our
	// corresponding X window has been destroyed.  If the shell is
	// not NULL, we must destroy the X window ourselves.
	if (GetShell()) {
		// save the window data
		SaveWindowData();
		// get the shell widget
		Widget w = GetShell();
		// set the shell to NULL (to be safe)
		SetShell(NULL);
		// avoid recursive deletes
		XtRemoveCallback(w,XtNdestroyCallback,(XtCallbackProc)DestroyWindProc,this);
		// destroy the X window
		XtDestroyWidget(w);
	}
	delete mMenu;		// delete our menu (if created)

	// finally, zero pointers to this window
	if (mData) {
		if (mData->mMainWindow == this) {
			mData->mMainWindow = NULL;
		} else {
			for (int i=0; i<NUM_WINDOWS; ++i) {
				if (mData->mWindow[i] == this) {
					mData->mWindow[i] = NULL;
				}
			}
		}
#ifdef LESSTIF
		// manually do our callbacks to patch linux bug where
		// callbacks are lost when a window closes - PH 12/20/99
		Kick(mData);
#endif
	}
}

// GetWindowGeometry - return current geometry of window
void PWindow::GetWindowGeometry(SWindowGeometry *geo)
{
	int		n;
	int		minWidth, minHeight;
	Arg		wargs[10];
	
	n = 0;
	XtSetArg(wargs[n], XmNx, &geo->x); ++n;
	XtSetArg(wargs[n], XmNy, &geo->y); ++n;
	XtSetArg(wargs[n], XmNwidth, &geo->width); ++n;
	XtSetArg(wargs[n], XmNheight, &geo->height); ++n;
	XtSetArg(wargs[n], XmNminWidth, &minWidth); ++n;
	XtSetArg(wargs[n], XmNminHeight, &minHeight); ++n;
	XtGetValues(GetShell(), wargs, n);

	// geometry width/height is relative to minimum width/height
	geo->width -= minWidth;
	geo->height -= minHeight;
}

/* SaveWindowData - Save window position information into resource database */
void PWindow::SaveWindowData()
{
	SWindowGeometry	geo;
	char			buff[256];
	
	if (GetShell()) {	// just to be safe (xsnoman is having problems in this area)

		// update resource database with current window location
		GetWindowGeometry(&geo);
		
		// get the full resource name for this widget
		PResourceManager::GetResourceName(GetShell(), buff);
		
		// save the window geometry to the resource database
		PResourceManager::SetWindowGeometry(buff, &geo);
	}
}

// CheckWindowOffset - check for offset between resource and actual window position
void PWindow::CheckWindowOffset(int border_width)
{
	// compare position in resources to actual position
	// and calculate window offset if not done already
	if (!sOffsetDone) {
		SWindowGeometry geo;
		// look for geometry from saved settings resource
		if (!PResourceManager::GetWindowGeometry(GetShell(), &geo) &&
			// settings not saved -- look for default XSnoed geometry
			!PResourceManager::GetWindowGeometry("XSnoed", &geo))
		{
			// no geometry resource -- use default position/size
			geo.x = geo.y = 30;
			geo.width = geo.height = 300;
		}
		SWindowGeometry actual;
		GetWindowGeometry(&actual);
		int dx = actual.x - border_width - geo.x;
		int dy = actual.y - border_width - geo.y;
		PResourceManager::SetWindowOffset(dx, dy);
		sOffsetDone = 1;
//		printf("pos (%d %d) offset (%d %d) size (%d %d)\n",
//				actual.x, actual.y, dx, dy, geo.width, geo.height);
	}
}

// Show - make window visible (must be called after creating the window)
void PWindow::Show()
{
	if (mShell) {
		// make sure we are updated before we are made visible
		Update();
		// do we have a parent widget?
		if (XtParent(mShell)) {
			// has a parent -- must be a child window, so manage it
			XtManageChild(mShell);
		} else {
			// no parent -- must be a top level shell, so realize it
			XtRealizeWidget(mShell);
		}
		mVisible = 1;	// we are now visible
/*
** see if this window size/position was set from the resources
*/
		SWindowGeometry geo;
		if (PResourceManager::GetWindowGeometry(GetShell(), &geo)) {
			mWasResized = 1;
		}
		// resize to fit specified widget if necessary
		if (mResizeWidget) {
			ResizeToFit(mResizeWidget);
			mResizeWidget = NULL;
		}
	}
}

// CreateShell() - create a new shell owned by the specified parent
// - if the parent is NULL, this creates a top-level shell
Widget PWindow::CreateShell(char *name, Widget parent, Arg *wargs, int n)
{
	Widget w;
	
#ifdef CHILD_WINDOWS
	if (parent) {
		// attempt to patch window decoration bug by adding 2 more arguments
		const int kMaxArgs = 20;
		Arg	targs[kMaxArgs];
		if (n > kMaxArgs-2) n = kMaxArgs-2;
		memcpy(targs, wargs, n * sizeof(Arg));
		XtSetArg(targs[n], XmNmwmDecorations, MWM_DECOR_ALL | MWM_DECOR_MINIMIZE);  ++n;
		XtSetArg(targs[n], XmNoverrideRedirect, FALSE); ++n;
		
		// create the popup shell
		w = XtCreatePopupShell(name, topLevelShellWidgetClass, parent, targs, n);
	} else {
#endif
#ifdef DEMO_VERSION
		// don't allow windows of demo version to be closed if protection is on
		const int kMaxArgs = 20;
		Arg	targs[kMaxArgs];
		if (n > kMaxArgs-2) n = kMaxArgs-2;
		memcpy(targs, wargs, n * sizeof(Arg));
		if (XSnoedWindow::sProtect) {
			XtSetArg(targs[n], XmNdeleteResponse, XmDO_NOTHING); ++n;
		}
		w = XtAppCreateShell(name, "XSnoed", topLevelShellWidgetClass,
							 PResourceManager::sResource.display, targs, n);
#else
		// create a shell widget
		w = XtAppCreateShell(name, "XSnoed", topLevelShellWidgetClass,
							 PResourceManager::sResource.display, wargs, n);
#endif
#ifdef CHILD_WINDOWS
	}
#endif
	return(w);
}

// SetMainWindow - make this a main window
void PWindow::SetMainWindow()
{
	if (!sMainWindow) {
		// this is the base main window
		sMainWindow = this;
	} else {
		// insert into main window linked list
		mNextMainWindow = sMainWindow->mNextMainWindow;
		sMainWindow->mNextMainWindow = this;
	}
}

void PWindow::SetDirty(int flag)
{
	mDirty |= flag;

	if (!sWindowDirty && mData->toplevel) {
	
		// set dirty flag indicating a window needs updating
		sWindowDirty = 1;
		
		// send a ClientMessage to be sure we break out of XtAppNextEvent()
		// so we can update our event windows from the event loop
		XClientMessageEvent clientMsg;
		clientMsg.type = ClientMessage;
		clientMsg.display = mData->display;
		clientMsg.window = XtWindow(mData->toplevel);
		clientMsg.message_type = 0;
		clientMsg.format = 8;
		XSendEvent(mData->display, XtWindow(mData->toplevel), FALSE, 0, (XEvent *)&clientMsg);
	}
}

// HandleUpdates - perform all updates via this mechanism
void PWindow::HandleUpdates()
{
	if (sWindowDirty) {
		sWindowDirty = 0;
		for (PWindow *win=sMainWindow; win; win=win->mNextMainWindow) {
			ImageData *data = win->GetData();
			win->Update();
			for (int i=0; i<NUM_WINDOWS; ++i) {
				if (data->mWindow[i]) {
					data->mWindow[i]->Update();
				}
			}
		}
	}
}

// Update - update window if necessary
void PWindow::Update()
{
	if (mDirty) {
		UpdateSelf();
		mDirty = 0;
	}
}

// SetTitle - set window title
void PWindow::SetTitle(char *str)
{
	Arg		wargs[1];

	XtSetArg(wargs[0], XmNtitle, str);
	XtSetValues(mShell, wargs, 1);		// set window title
}

// GetTitle - get window title
// (the returned string is owned by X, so don't change or delete it)
char *PWindow::GetTitle()
{
	char  *	pt;
	Arg		wargs[1];
	
	XtSetArg(wargs[0], XmNtitle, &pt);
	XtGetValues(mShell, wargs, 1);		// get window title
	return(pt);
}

// Raise - raise window to top of stack so it isn't obscured by other windows
void PWindow::Raise()
{
	XRaiseWindow(mData->display, XtWindow(mShell));
}

// Resize - resize the window
void PWindow::Resize(int width, int height)
{
	// can only resize window if we are visible
	if (mVisible) {
		// only resize window if we are visible
		XResizeWindow(mData->display, XtWindow(mShell), width, height);
		mWasResized = 1;
	}
}

// Resize to fit - resize height of window to fit specified widget
void PWindow::ResizeToFit(Widget w)
{
	int			n;
	Arg			wargs[10];
	Dimension	top, width, height;
	
	if (mVisible) {
		// resize window to fit new height
		n = 0;
		XtSetArg(wargs[n], XmNy, &top);  ++n;
		XtSetArg(wargs[n], XmNheight, &height);  ++n;
		XtGetValues(w, wargs, n);
	
		// leave the width the same
		n = 0;
		XtSetArg(wargs[n], XmNwidth, &width); ++n;
		XtGetValues(GetShell(), wargs, n);
		Resize(width, top + height);
	} else {
		// delay resize until we are visible
		mResizeWidget = w;
	}
}

// Create menu object
// - if 'menu' is NULL, also creates standard menubar widget 
//   (in this case, this routine must be called after SetMainPane())
void PWindow::CreateMenu(Widget menu, MenuStruct *menuList, int nItems, PMenuHandler *handler)
{
	// create menu widget if necessary
	if (!menu) {
		Widget w = GetMainPane();
		if (!w) return;		// can do nothing if main pane not set
		// create menu widget
		Arg wargs[10];
		int n = 0;
		XtSetArg(wargs[n],XmNmarginHeight,		1); ++n;
		XtSetArg(wargs[n],XmNleftAttachment,	XmATTACH_FORM); ++n;
		XtSetArg(wargs[n],XmNtopAttachment,		XmATTACH_FORM); ++n;
		XtSetArg(wargs[n],XmNrightAttachment,	XmATTACH_FORM); ++n;
		menu = XmCreateMenuBar( w, "xsnoedMenu" , wargs, n);
		XtManageChild(menu);
	}
	if (mMenu) {
		// add to existing menus
		mMenu->AddMenu(menuList, nItems, handler);
	} else {
		// create PMenu object
		mMenu = new PMenu(menu, menuList, nItems, handler);
	}
}

void PWindow::SetShell(Widget w)
{
	mShell = w;
	
	if (w) {
		/* delete object when window is destroyed */
		XtAddCallback(w,XmNdestroyCallback,(XtCallbackProc)DestroyWindProc,this);

		// THIS DOESN'T WORK!!!! why?
/** Xt accelerators incompatible with Motif accelerators
		PWindow *mainWindow = GetData()->mMainWindow;
		if (mainWindow!=NULL && mainWindow!=this) {
			// install main window accelerators in all other windows
			XtInstallAllAccelerators(w, mainWindow->GetShell());
		}
*/		
	}
}

// SelectMenuItem - select menu item as if user had selected it himself
// return 0 if item selected OK
int PWindow::SelectMenuItem(int id)
{
	if (!mMenu) return(-2);
		
	return(mMenu->SelectItem(id));
}

// SetMenuItemText - set the text of a menu item
void PWindow::SetMenuItemText(int id, char *str)
{
	if (mMenu) {
		mMenu->SetLabel(id, str);
	}
}

void PWindow::DestroyWindProc(Widget w, PWindow *aWind, caddr_t call_data)
{
	// save the window data first
	aWind->SaveWindowData();
	
	// set the shell to NULL to indicate that the widget has been deleted
	aWind->SetShell(NULL);
	
	// delete the PWindow object
	delete	aWind;
}
