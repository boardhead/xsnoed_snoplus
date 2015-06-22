#ifndef __PWindow_h__
#define __PWindow_h__

#include <Xm/Xm.h>

#ifdef LESSTIF
#define RADIO_OFFSET	0
#else
#define RADIO_OFFSET	2
#endif


class PMenu;
class PMenuHandler;
struct MenuStruct;
struct ImageData;
struct SWindowData;
struct SWindowGeometry;


class PWindow {
public:
	PWindow(ImageData *data,Widget shell=0,Widget mainPane=0);
	virtual ~PWindow();
	
	virtual void	Update();
	virtual void	UpdateSelf()			{ }

	virtual void	Show();
	virtual char *	Class()					{ return sWindowClass;	}
	
	virtual void	SetShell(Widget w);
	
	void			Raise();
	void			SetTitle(char *str);
	char *			GetTitle();
	void			GetFullName(char *buff);
	void			Resize(int width, int height);
	void			ResizeToFit(Widget w);
	void			SaveWindowData();
	void			SetMainWindow();
	void			SetDirty(int flag=0x01);
	int				IsDirty()				{ return mDirty;		}
	int				IsVisible()				{ return mVisible;		}
	int				WasResized()			{ return mWasResized;	}
	Widget			GetShell()				{ return mShell; 		}
	void			SetMainPane(Widget w)	{ mMainPane = w; 		}
	Widget			GetMainPane()			{ return mMainPane; 	}
	ImageData	  *	GetData()				{ return mData;			}
	
	void			CreateMenu(Widget menu, MenuStruct *menuList, int nItems, PMenuHandler *handler);
	PMenu		  *	GetMenu()				{ return mMenu;			}
	int				SelectMenuItem(int id);
	void			SetMenuItemText(int id, char *str);
	void			GetWindowGeometry(SWindowGeometry *geo);
	void			CheckWindowOffset(int border_width);
	
	static void		HandleUpdates();
	static Widget	CreateShell(char *name,Widget parent,Arg *wargs=NULL,int n=0);
	
	// public variables
	PWindow		  *	mNextMainWindow;
	
	static PWindow*	sMainWindow;
	static char   *	sWindowClass;

protected:
	ImageData	  *	mData;
	PMenu		  *	mMenu;

	static int		sWindowDirty;
	
private:
	static void		DestroyWindProc(Widget w, PWindow *aWind, caddr_t call_data);
	
	Widget			mShell;			// window shell widget
	Widget			mMainPane;		// main form or rowcolumn widget
	Widget			mResizeWidget;	// widget to resize to when shown
	int				mWasResized;	// non-zero if window was resized
	int				mDirty;			// flag indicating the window needs redrawing
	int				mVisible;		// flag indicates window is visible
	
	static int		sOffsetDone;	// true if window position resource offset was set
};

#endif // __Window_h__
