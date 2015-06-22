#include <Xm/Xm.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "PMenu.h"
#include "CUtils.h"
#include "PUtils.h"

#define VERIFY_ACCELERATORS		// un-comment this to automatically test for
								// duplcate accelerators/mnemonics

MenuList	  *	PMenu::sCurMenuItem 	= NULL;	// last selected menu item
int				PMenu::sWasAccelerator	= 0;

//-------------------------------------------------------------------------------------------------
// MenuList function definitions
//
MenuList::MenuList(PMenu *anOwner, MenuStruct *ms, PMenuHandler *aHandler)
		: owner(anOwner), id(ms->id), menu_handler(aHandler), flags(ms->flags),
		  button(NULL), sub_menu(NULL), next(NULL)
{
}
MenuList::~MenuList()
{
}

//-------------------------------------------------------------------------------------------------
// PMenu function definitions
//

// PMenu constructor
PMenu::PMenu(Widget menu, MenuStruct *menuDef, int nitems, PMenuHandler *handler)
{
	mWidget = menu;
	mHandler = handler;
	mMenuList = CreateMenu(NULL,menu,menuDef,nitems,handler);
}

// PMenu destructor
PMenu::~PMenu()
{
	DestroyMenu(mMenuList,0);
}

// AddMenu - Add new menu as the last entry in the menu bar
void PMenu::AddMenu(MenuStruct *menuDef, int nitems, PMenuHandler *handler)
{
	// look for last item in current menu list
	MenuList **	menuListPt = &mMenuList;
	while (*menuListPt) {
		menuListPt = &((*menuListPt)->next);
	}
	// use existing handler if 'handler' is NULL
	if (!handler) handler = mHandler;
	// add new menus to end of current list
	*menuListPt = CreateMenu(NULL,mWidget,menuDef,nitems,handler);
}

// CreateMenu [private] - Create a new Motif menu from MenuStruct definition
MenuList *PMenu::CreateMenu(char *title, Widget menu, MenuStruct *menuDef, int nitems,
							PMenuHandler *handler, int index)
{
	int			i, n;
	Arg			wargs[10];
	WidgetList	buttons;
	MenuList  *	menuList = NULL;
	MenuList **	menuListPt = &menuList;

	if (!nitems) return(NULL);
	
#ifdef VERIFY_ACCELERATORS
	VerifyAccelerators(menuDef, nitems);
#endif
	
	// create temporary array for menu item widgets
	buttons = (WidgetList)XtMalloc(nitems*sizeof(Widget));

	if (title) {
		XtCreateManagedWidget(title,xmLabelWidgetClass, menu, NULL, 0);
		XtCreateManagedWidget("separator",xmSeparatorWidgetClass, menu,NULL,0);
	}
	for (i=0; i<nitems; ++i) {
	
		// create and initialize new MenuList item
		MenuList *item = new MenuList(this, menuDef+i, handler);
		if (!item) break;
		
		// copy over the accelerator
		item->accelerator = menuDef[i].accelerator;
		// translate special accelerator keys
		switch (item->accelerator) {
			case '<':
				item->accelerator = ',';
				break;
			case '>':
				item->accelerator = '.';
				break;
		}
		
		// insert item into linked list
		*menuListPt = item;
		menuListPt = &item->next;
		
		// initialize argument list for button
		n = 0;
		if (index != XmLAST_POSITION) {
			// put button at specified position in menu
			XtSetArg(wargs[n], XmNpositionIndex, index); ++n;
			++index;
		}
		
		if (menuDef[i].name == NULL) {
			// name is NULL -- add a menu separator
			buttons[i] = XtCreateManagedWidget("separator",xmSeparatorWidgetClass,menu,wargs,n);
		} else {
			// disable menu item if specified
			if (item->flags & (MENU_DISABLED | MENU_PROTECTED)) {
				XtSetArg(wargs[n],XmNsensitive,FALSE);  ++n;
			}
			// set mnemonic if specified
			if (menuDef[i].mnemonic) {
				XtSetArg(wargs[n], XmNmnemonic, menuDef[i].mnemonic); ++n;
			}
			if (menuDef[i].sub_menu) {
				// item has a sub-menu -- add a cascade button to the menu
				Widget sub_menu;
				sub_menu = XmCreatePulldownMenu(menu,"SubMenu",NULL,0);
				XtSetArg(wargs[n],XmNsubMenuId,sub_menu); ++n;
				buttons[i] = XtCreateWidget(menuDef[i].name, xmCascadeButtonWidgetClass,menu,wargs,n);
				// add cascading callback
				XtAddCallback(buttons[i],XmNcascadingCallback, (XtCallbackProc)CascadeProc, item);
				item->sub_menu = CreateMenu(NULL,sub_menu,menuDef[i].sub_menu,
											menuDef[i].n_sub_items,handler);
			} else if (item->id) {
				// item has a command ID -- add a button to the menu
				if (item->flags & (MENU_TOGGLE|MENU_RADIO)) {
					// add a toggle button
					if (item->flags & MENU_RADIO) {
						XtSetArg(wargs[n],XmNindicatorType,XmONE_OF_MANY);  ++n;
					}
					// set toggle on if specified
					if (item->flags & MENU_TOGGLE_ON) {
						XtSetArg(wargs[n],XmNset,TRUE);  ++n;
					}
					buttons[i] = XtCreateWidget(menuDef[i].name, xmToggleButtonWidgetClass,menu,wargs,n);
					XtAddCallback(buttons[i],XmNvalueChangedCallback, (XtCallbackProc)MenuProc, item);
				} else {
					// add a push button
					buttons[i] = XtCreateWidget(menuDef[i].name, xmPushButtonWidgetClass,menu,wargs,n);
					XtAddCallback(buttons[i],XmNactivateCallback, (XtCallbackProc)MenuProc, item);
				}
				// add accelerator if specified
				if (menuDef[i].accelerator) {
					char buff[64];
					sprintf(buff,"Alt+%c",toupper(menuDef[i].accelerator));
					XmString str = XmStringCreateLtoR(buff,"SMALL");
					n = 0;
/* handle the accelerators manually until I figure out
** how to get them working in all windows
					sprintf(buff,"Alt<Key>%c",tolower(menuDef[i].accelerator));
					XtSetArg(wargs[n], XmNaccelerator, buff); ++n;
*/
					XtSetArg(wargs[n], XmNacceleratorText, str); ++n;
					XtSetValues(buttons[i], wargs, n);
					XmStringFree(str);
					// Patch -- zero accelerator so it can't be used again
					menuDef[i].accelerator = 0;
				}
			} else {
				// add a dead label to the menu
				buttons[i] = XtCreateWidget(menuDef[i].name, xmLabelWidgetClass,menu,wargs,n);
			}
		}
		item->button = buttons[i];
	}
	XtManageChildren(buttons,nitems);
	XtFree((char *)buttons);	// done with button WidgetList
	
	return(menuList);
}

// AddMenuItem - add item to end of the main menu or a submenu (if subMenu is not NULL)
// - uses existing handler or subMenu handler if 'handler' is NULL
void PMenu::AddMenuItem(MenuStruct *newItem, MenuList *subMenu, PMenuHandler *handler, int index)
{
	if (subMenu && subMenu->sub_menu) {
		// add new item to the specified sub-menu
		Widget menuWidget = NULL;
		Arg wargs[1];
		XtSetArg(wargs[0], XmNsubMenuId, &menuWidget);
		XtGetValues(subMenu->button, wargs, 1);
		if (menuWidget) {
			// use sub-menu handler if 'handler' is NULL
			if (!handler) handler = subMenu->menu_handler;
			subMenu->sub_menu = AddMenuItem(menuWidget, subMenu->sub_menu, newItem, handler, index);
		}
	} else {
		// use existing handler if 'handler' is NULL
		if (!handler) handler = mHandler;
		// add new item to the main menu bar
		mMenuList = AddMenuItem(mWidget, mMenuList, newItem, handler, index);
	}
}

// AddMenuItem [private] - add one item to the end of a menu
// menu - menu widget
// ms - original menu struct (can be NULL)
// numItems - original number of items in menu
// newItem - new item to add to the menu
// handler - menu handler routine
// index - position of new item in menu (0 = first, 'numItems' or greater = last)
MenuList *PMenu::AddMenuItem(Widget menu, MenuList *ms, MenuStruct *newItem, PMenuHandler *handler, int index)
{
	// create a new menu list with the single item
	MenuList *theMenu = CreateMenu(NULL, menu, newItem, 1, handler, index);
	
	if (ms) {
		// insert item into existing menu
		if (index) {
			// find specified location in linked list
			MenuList *afterItem = ms;
			for (int i=1; i!=index; ++i) {
				if (!afterItem->next) break;	// at end of list
				afterItem = afterItem->next;
			}
			// insert item into list
			theMenu->next = afterItem->next;
			afterItem->next = theMenu;
			// return pointer to first item in menu list
			theMenu = ms;
		} else {
			// new item is the first entry in the list
			theMenu->next = ms;
		}
	}
	
	return(theMenu);	// return a pointer to the new menu list
}

// RemoveMenuItem - remove specified item from given menu
// - ms must not be NULL
void PMenu::RemoveMenuItem(MenuList *ms, int index)
{
	MenuList **itemPt;
	
	// get pointer to first item in menu
	if (ms) {
		itemPt = &ms->sub_menu;		// remove from sub menu
	} else {
		itemPt = &mMenuList;		// remove from main menu
	}
	MenuList *item = *itemPt;
	
	if (!item) return;	// no items in this menu
	
	// find the specified item
	for (int i=0; i!=index; ++i) {
		if (!item->next) break;	// stop if no more items
		// step to next item in list
		itemPt = &item->next;
		item = *itemPt;
	}
	// remove item from the linked list
	*itemPt = item->next;
	
	// destroy submenu if it exists
	if (item->sub_menu) {
		DestroyMenu(item->sub_menu, 1);
	}
	// destroy the button widget
	if (item->button) {
		XtDestroyWidget(item->button);
	}
	// delete the menu list entry
	delete item;
}

// DestroyMenu [private, static] - recursively free memory allocated to menu structures
void PMenu::DestroyMenu(MenuList *ms, int destroy_widgets)
{
	// free all sub-menus first
	while (ms) {
		// destroy the submenu if necessary
		if (ms->sub_menu) {
			DestroyMenu(ms->sub_menu, destroy_widgets);
		}
		// destroy the button widget if necessary
		if (ms->button && destroy_widgets) {
			XtDestroyWidget(ms->button);
		}
		MenuList *oldItem = ms;	// save pointer to this entry
		ms = ms->next;			// step to next entry in list
		delete oldItem;			// delete the old menu list entry
	}
}


// GetToggle [static] - return non-zero if a menu toggle is selected
int PMenu::GetToggle(MenuList *ms)
{
/*
	int	is_on = 0;
	Arg wargs[1];
	
	XtSetArg(wargs[0], XmNset, &is_on);
	XtGetValues(ms->button, wargs, 1);
	return(is_on);
*/
	return((ms->flags & MENU_TOGGLE_ON) != 0);
}

// SetToggle - set the value of a menu toggle by item ID
void PMenu::SetToggle(int id, int on)
{
	MenuList *ms = FindMenuItem(id, mMenuList);
	if (ms) SetToggle(ms, on);
}

// SetToggle [static] - set the value of a menu toggle in a specific menu struct
void PMenu::SetToggle(MenuList *ms, int on)
{
	int new_flags;
	
	if (on) {
		new_flags = ms->flags | MENU_TOGGLE_ON;
	} else {
		new_flags = ms->flags & ~MENU_TOGGLE_ON;
	}
	if (ms->flags != new_flags) {
		Arg wargs[1];
		XtSetArg(wargs[0], XmNset, on ? TRUE : FALSE);
		XtSetValues(ms->button,wargs,1);
		ms->flags = new_flags;
	}
}

// SetEnabled [static] - utility to set enabled flags
void PMenu::SetEnabled(int *flagPt, int on)
{
	if (on) {
		*flagPt &= ~MENU_DISABLED;	// reset disabled flag
	} else {
		*flagPt |= MENU_DISABLED;		// set disabled flag
	}
}

// EnableItem - enable/disable a menu item by ID
void PMenu::EnableItem(int id, int on)
{
	MenuList *ms = FindMenuItem(id, mMenuList);
	if (ms) EnableItem(ms, on);
}

// EnableItem [static] - enable/disable a menu item
void PMenu::EnableItem(MenuList *ms, int on)
{
	int wasSensitive = IsSensitive(ms);
	
	if (on) ms->flags &= ~MENU_DISABLED;
	else	ms->flags |= MENU_DISABLED;

	int isSensitive = IsSensitive(ms);
	
	// change button sensitivity if necessary
	if (isSensitive != wasSensitive) {
		Arg warg;
		XtSetArg(warg, XmNsensitive, isSensitive);
		XtSetValues(ms->button, &warg, 1);
	}
}

// ProtectItem - protect/unprotect a menu item by ID
void PMenu::ProtectItem(int id, int on)
{
	MenuList *ms = FindMenuItem(id, mMenuList);
	if (ms) ProtectItem(ms, on);
}

// ProtectItem [static] - protect/unprotect a menu item
void PMenu::ProtectItem(MenuList *ms, int on)
{
	int wasSensitive = IsSensitive(ms);
	
	if (on) ms->flags |= MENU_PROTECTED;
	else	ms->flags &= ~MENU_PROTECTED;

	int isSensitive = IsSensitive(ms);
	
	// change button sensitivity if necessary
	if (isSensitive != wasSensitive) {
		Arg warg;
		XtSetArg(warg, XmNsensitive, isSensitive);
		XtSetValues(ms->button, &warg, 1);
	}
}

// IsSensitive [static] - return TRUE if menu item is sensitive (enabled and unprotected)
int PMenu::IsSensitive(MenuList *ms)
{
	return((ms->flags & (MENU_DISABLED | MENU_PROTECTED)) == 0);
}

// UpdateTogglePair [static] - Update Menu radio toggle pair
// - on call, valPt points to the command ID of the previously selected radio item
// - on return, *valPt is updated with the ID of the newly selected radio item
// - radio button widget states and MenuList flags are all set appropriately
int PMenu::UpdateTogglePair(int *valpt)
{
	MenuList *ms = sCurMenuItem;

	if (ms) {
	
		if (*valpt == ms->id) {
			if (ms->flags & MENU_RADIO) {
				// we just selected the same entry again
				// X will have turned off the toggle so turn it back on
				SetToggle(ms, TRUE);
			} else {
				Printf("Menu radio error -- ID = %d\n", ms->id);
			}
			return(0);
		} else if (ms->owner) {
			MenuList *oldItem = FindMenuItem(*valpt, ms->owner->GetMenuList());
			if (oldItem && (oldItem->flags & MENU_RADIO)) {
				// X turned on the new toggle so turn off the old one
				SetToggle(oldItem, FALSE);
			} else {
				Printf("Menu radio error -- ID = %d\n", ms->id);
			}
			*valpt = ms->id;
			return(1);
		}
	}
	return(0);
}

// SetLabel - set the label of a menu item by ID
void PMenu::SetLabel(int id, char *str)
{
	MenuList *ms = FindMenuItem(id, mMenuList);
	if (ms) SetLabel(ms, str);
}

// SetLabel [static] - set the label of a menu entry
void PMenu::SetLabel(MenuList *ms, char *str)
{
	// set the button widget label string
	setLabelString(ms->button, str);
}

// GetLabel - return label string of menu item specified by ID
// Important: caller must free the returned string with XtFree()
char *PMenu::GetLabel(int id)
{
	MenuList *ms = FindMenuItem(id, mMenuList);
	if (ms) {
		return(GetLabel(ms));
	} else {
		return(NULL);
	}
}

// GetLabel [static] - return label string of specified menu item
// - Important: caller must free the returned string with XtFree()
// - gets label from current menu item if ms is NULL
char *PMenu::GetLabel(MenuList *ms)
{
	if (!ms) ms = sCurMenuItem;
	if (ms && ms->button) {
		return(getLabelString(ms->button));
	} else {
		return(NULL);
	}
}


// FindMenuItem - return pointer to MenuList item associated with specified menu command ID
MenuList *PMenu::FindMenuItem(int id)
{
	return FindMenuItem(id,mMenuList);
}

// FindMenuItem [private, static] - find item by ID in specified menu
// - also searches sub-menus
MenuList *PMenu::FindMenuItem(int id, MenuList *menu)
{
	MenuList	*ms;
	
	if (id) {	// ignore zero id's (used by menu separators)
		while (menu) {
			if (menu->id == id) return(menu);
			if (menu->sub_menu) {
				ms = FindMenuItem(id, menu->sub_menu);
				if (ms) return(ms);
			}
			menu = menu->next;
		}
	}
	return((MenuList *)0);
}

// DoAccelerator - do the action of a menu accelerator
int PMenu::DoAccelerator(KeySym ks)
{
	return(DoAccelerator(ks, mMenuList));
}

// DoAccelerator [private, static] - do accelerator action in specified menu
// - returns non-zero if accelerator found
int PMenu::DoAccelerator(KeySym ks, MenuList *menu)
{
	while (menu) {
		if (menu->accelerator == ks) {
			// only select item if it is sensitive
			if (IsSensitive(menu)) {
				sWasAccelerator = 1;
				SelectItem(menu);	// found a match! -- select the menu item
				sWasAccelerator = 0;
			}
			return(1);
		}
		if (menu->sub_menu) {
			// do accelerators in sub-menus too
			if (DoAccelerator(ks, menu->sub_menu)) {
				return(1);
			}
		}
		menu = menu->next;
	}
	return(0);	// accelerator not found
}

// VerifyAccelerators [private, static] - check for duplicate accelerators/mnemonics
// - does not recurse into sub-menus
void PMenu::VerifyAccelerators(MenuStruct *menuDef, int nitems)
{
	char		ch;
	char		*mne[256];
	static char	*acc[256];

	memset(mne, 0, 256 * sizeof(char *));

	for (int i=0; i<nitems; ++i) {
		if (menuDef[i].accelerator) {
			ch = tolower(menuDef[i].accelerator);
			if (acc[(int)ch]) {
				Printf("Item '%s' has duplicate accelerator '%c' - Originally defined by '%s'\n",
						menuDef[i].name, ch, acc[(int)ch]);
			} else {
				acc[(int)ch] = menuDef[i].name;
			}
		}
		if (menuDef[i].mnemonic) {
			ch = tolower(menuDef[i].mnemonic);
			if (mne[(int)ch]) {
				Printf("Item '%s' has duplicate mnemonic '%c' - Originally defined by '%s'\n",
						menuDef[i].name, ch, mne[(int)ch]);
			} else {
				mne[(int)ch] = menuDef[i].name;
			}
		}
	}
}

// SelectItem - Select an item via programmers interface
// - returns 0 if item selected OK
int PMenu::SelectItem(int id)
{
	MenuList *ms = FindMenuItem(id);
	
	if (!ms) return(-1);
	
	SelectItem(ms);
	
	return(0);
}

// SelectItem [static] - select specified menu item
void PMenu::SelectItem(MenuList *ms)
{
	if (ms->flags & (MENU_TOGGLE | MENU_RADIO)) {
		// set the widget state, but don't change our MenuList flags
		// (they will be set by MenuProc)
		Arg wargs[1];
		XtSetArg(wargs[0], XmNset, !GetToggle(ms));
		XtSetValues(ms->button,wargs,1);
	}
	
	// do the callback
	MenuProc((Widget)0, ms, (caddr_t)0);
}

// MenuProc [private, static] - menu callback procedure
void PMenu::MenuProc(Widget w, MenuList *ms, caddr_t call_data)
{
	sCurMenuItem = ms;	// set current menu struct
	// if the user selected a toggle or radio button, Motif
	// automatically changes the state of the button so we
	// must change or flags to reflect the new button state
	if (ms->flags & (MENU_TOGGLE | MENU_RADIO)) {
		ms->flags ^= MENU_TOGGLE_ON;
	}
	if (ms->menu_handler) {
		ms->menu_handler->DoMenuCommand(ms->id);
	}
}

// CascadeProc [private, static] - cascading callback procedure
// (called prior to opening of a cascade menu)
void PMenu::CascadeProc(Widget w, MenuList *ms, caddr_t call_data)
{
	// check the state of all items in the sub-menu
	for (ms=ms->sub_menu; ms!=NULL; ms=ms->next) {
		if (!ms->id || !ms->menu_handler) continue;
		// call the handler to check the state of this menu item
		int newFlags = ms->menu_handler->CheckMenuCommand(ms->id, ms->flags);
		if (newFlags != ms->flags) {
			// the state has changed -- update the item as necessary
			int diff = (newFlags ^ ms->flags);
			if (diff & MENU_TOGGLE_ON) {
				// the item toggle has changed state
				SetToggle(ms, newFlags & MENU_TOGGLE_ON);
			}
			if (diff & MENU_DISABLED) {
				// the item has been enabled/disabled
				EnableItem(ms, !(newFlags & MENU_DISABLED));
			}
		}
	}
}
