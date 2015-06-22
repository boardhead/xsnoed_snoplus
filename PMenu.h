#ifndef __PMenu_h__
#define __PMenu_h__

#include <Xm/Xm.h>
#include <X11/keysym.h>

// menu item flags
#define MENU_TOGGLE		0x0100		// the item is a toggle button
#define MENU_RADIO		0x0200		// the item is a radio button
#define MENU_TOGGLE_ON	0x0400		// the toggle or radio button is turned on
#define MENU_DISABLED	0x0800		// the item is disabled
#define MENU_PROTECTED	0x1000		// the item is protected (permanently disabled)

class PMenuHandler {
public:
    virtual ~PMenuHandler() { }
	// execute a menu command
	virtual void	DoMenuCommand(int anID) 				{ }
	// check/change the state of a menu item before the menu is selected
	virtual int		CheckMenuCommand(int anID, int flags)	{ return flags; }
};

// menu item structure -- used to define menu items when creating new menus
struct MenuStruct {
	char		  *	name;			// menu item label (NULL for separator item)
	char			accelerator;	// accelerator character
	KeySym			mnemonic;		// mnemonic keysym
	int				id;				// menu command ID number (0 for no command)
	MenuStruct	  *	sub_menu;		// pointer to first sub-menu item (NULL for no sub-menu)
	int				n_sub_items;	// number of items in sub-menu (0 for no sub-menu)
	int				flags;			// menu item flag bits
};

class PMenu;

// menu list element -- used to store data for active menu items
struct MenuList {
	MenuList(PMenu *anOwner, MenuStruct *ms, PMenuHandler *aHandler);
	~MenuList();
	
	PMenu		  *	owner;			// pointer to PMenu object that created this item
	int				id;				// menu command ID number (0 for no command)
	KeySym			accelerator;	// menu accelerator
	PMenuHandler  *	menu_handler;	// pointer to object that handles menu commands
	int				flags;			// menu item flag bits
	Widget			button;			// button widget (never NULL, even for separator items)
	MenuList	  *	sub_menu;		// first item in sub-menu (NULL if no sub-menu)
	MenuList	  *	next;			// pointer to next item in menu (NULL if last item)
};


// class definition
class PMenu {
public:
	PMenu(Widget menu, MenuStruct *menuDef, int nitems, PMenuHandler *handler);
	~PMenu();
	
	Widget				GetWidget()			{ return mWidget;		}
	
	MenuList		  *	FindMenuItem(int id);
	
	void				AddMenu(MenuStruct *menuDef, int nitems, PMenuHandler *handler);
	void				AddMenuItem(MenuStruct *newItem, MenuList *subMenu=NULL,
									PMenuHandler *handler=NULL, int index=XmLAST_POSITION);
	void				RemoveMenuItem(MenuList *subMenu=NULL, int index=XmLAST_POSITION);
	
	int					SelectItem(int id);
	void				SetToggle(int id, int on=TRUE);
	void				EnableItem(int id, int on=TRUE);
	void				ProtectItem(int id, int on=TRUE);
	void				SetLabel(int id, char *str);
	char *				GetLabel(int id);
	int					DoAccelerator(KeySym ks);
	
	MenuList		  *	GetMenuList()		{ return mMenuList; }
	
	static void			SelectItem(MenuList *ms);
	static int			GetToggle(MenuList *ms);
	static void			SetToggle(MenuList *ms, int on=TRUE);
	static void			EnableItem(MenuList *ms, int on=TRUE);
	static void			ProtectItem(MenuList *ms, int on=TRUE);
	static void			SetLabel(MenuList *ms, char *str);
	static char		  *	GetLabel(MenuList *ms);
	static void			SetEnabled(int *flagPt, int on=TRUE);
	static int			UpdateTogglePair(int *valpt);
	static int			IsSensitive(MenuList *ms);
	static MenuList   *	GetCurMenuItem()	{ return sCurMenuItem;		}
	static int			WasAccelerator()	{ return sWasAccelerator;	}

private:
	MenuList 		  *	CreateMenu(char *title,Widget menu,MenuStruct *menuDef,int nitems,
								   PMenuHandler *handler, int index=XmLAST_POSITION);
	MenuList		  *	AddMenuItem(Widget menu, MenuList *ms, MenuStruct *newItem,
									PMenuHandler *handler, int index=XmLAST_POSITION);
	
	static MenuList   *	FindMenuItem(int id, MenuList *menu);
	static int			DoAccelerator(KeySym ks, MenuList *menu);
	static void			DestroyMenu(MenuList *menu, int destroy_widgets);
	static void			VerifyAccelerators(MenuStruct *menuDef, int nitems);

	static void			MenuProc(Widget w, MenuList *ms, caddr_t call_data);
	static void			CascadeProc(Widget w, MenuList *ms, caddr_t call_data);
	
	Widget				mWidget;
	PMenuHandler	  *	mHandler;
	MenuList		  *	mMenuList;
	
	static MenuList	  *	sCurMenuItem;	// current menu item (only valid in menu handler)
	static int			sWasAccelerator;
};

#endif // __PMenu_h__
