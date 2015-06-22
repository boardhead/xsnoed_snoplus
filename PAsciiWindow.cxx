//
// File:		PAsciiWindow.cxx
//
// Description:	Allows user to record ASCII-based information about events to file.
//
// Created:		02/08/00 - Phil Harvey
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include "PAsciiWindow.h"
#include "PZdabWriter.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "ImageData.h"
#include "xsnoed.h"
#include "openfile.h"

#define ASCII_MENU_FILENAME		"ascii_window_menu.dat"

#define WINDOW_WIDTH			396
#define WINDOW_HEIGHT			172
#define MIN_WINDOW_WIDTH		320


PAsciiWindow::PAsciiWindow(ImageData *data)
			: PWindow(data)
{
	int			n;
	Arg			wargs[16];
	Widget		w, but, but2;
	
	// try to open file for extra event classifications
    FILE *fp = openFile(ASCII_MENU_FILENAME,"r",data->file_path);
    
    if (fp) mExtraHeight = 37;
    else mExtraHeight = 0;
    
	mAutoNext = data->ascii_auto;
	mNumComments = 0;
	mCurComment = -1;
	mCurMenuItem = 0;
	mWriter = NULL;
	
  	n = 0;
	XtSetArg(wargs[n], XmNtitle, "ASCII Output"); ++n;
	XtSetArg(wargs[n], XmNx, 400); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNminWidth, MIN_WINDOW_WIDTH); ++n;
	XtSetArg(wargs[n], XmNminHeight, WINDOW_HEIGHT + mExtraHeight); ++n;
	SetShell(CreateShell("asciiPop",data->toplevel,wargs,n));
	w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0);
	SetMainPane(w);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 128 + mExtraHeight); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	but = XtCreateManagedWidget("Write",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)WriteProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 110); ++n;
	XtSetArg(wargs[n], XmNy, 128 + mExtraHeight); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	XtSetArg(wargs[n], XmNset, mAutoNext); ++n;
	but = XtCreateManagedWidget("Auto-Next",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but, XmNvalueChangedCallback, (XtCallbackProc)AutoProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNy, 128 + mExtraHeight); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	but2 = XtCreateManagedWidget("Done",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but2, XmNactivateCallback, (XtCallbackProc)DoneProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 18); ++n;
	XtCreateManagedWidget("Filename:", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNy, 12); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNleftOffset, 100); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 3); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	filename_text = XtCreateManagedWidget("filetext", xmTextWidgetClass, w, wargs, n);
	XtAddCallback(filename_text,XmNactivateCallback,(XtCallbackProc)WriteProc,this);
	setTextString(filename_text, data->ascii_file);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 55); ++n;
	XtCreateManagedWidget("Label:", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNy, 49); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNleftOffset, 100); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 3); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	format_text = XtCreateManagedWidget("formattext", xmTextWidgetClass, w, wargs, n);
	XtAddCallback(format_text,XmNactivateCallback,(XtCallbackProc)WriteProc,this);
	setTextString(format_text, data->ascii_label);
    
    if (fp) {
    	const int	kBuffSize = 1024;
    	char		buff[kBuffSize];
    	char	  *	delim = "\n\r";
    	char	  *	pt;
    	
    	int fileSize = fread(buff,1,kBuffSize-1,fp);
    	
    	buff[fileSize] = '\0';	// null-terminate the string
     	
     	// get label for the menu
    	pt = strtok(buff, delim);
   	
    	if (pt) {
    		
			XmString menu_label = XmStringCreateLocalized(pt);
			
			// create the menu definition
			const int kMaxMenuItems = 100;
			MenuStruct menu_struct[kMaxMenuItems];
			int numItems = 0;
			pt = "<none>";
			for (;;) {
				MenuStruct *ms = menu_struct + numItems;
				ms->name 		= pt;
				ms->accelerator = 0;
				ms->mnemonic 	= 0;
				ms->id 			= numItems + 1;
				ms->sub_menu 	= NULL;
				ms->n_sub_items	= 0;
				ms->flags		= 0;
				if (++numItems >= kMaxMenuItems) break;
				if (numItems > 1) {
					pt = strtok(NULL, delim);
					if (!pt) break;
				} else {
					pt = NULL;	// add a separator after the first entry
				}
			}
				
			// create classification option menu
			Widget sub_menu = XmCreatePulldownMenu(w, "optionMenu", NULL, 0);
			CreateMenu(sub_menu, menu_struct, numItems, this);
			
			// create option menu widget
			n = 0;
			XtSetArg(wargs[n], XmNx, 16); ++n;
			XtSetArg(wargs[n], XmNy, 86); ++n;
			XtSetArg(wargs[n], XmNmarginHeight, 0); ++n;
			XtSetArg(wargs[n], XmNmarginWidth, 0); ++n;
			XtSetArg(wargs[n], XmNsubMenuId, sub_menu); ++n;
			XtSetArg(wargs[n], XmNlabelString, menu_label); ++n;
			Widget option_menu = XmCreateOptionMenu(w, "ascii_opt", wargs, n);
			XtManageChild(option_menu);

			// free the string used to create the menu label
			XmStringFree(menu_label);
		
/*			// set current menu item
			MenuList *cur_item = mMenu->FindMenuItem(mCurMenuItem);
			if (cur_item) {
				n = 0;
				XtSetArg(wargs[n], XmNmenuHistory, cur_item->button); ++n;
				XtSetValues(option_menu, wargs, n);
			}*/
		}
		fclose(fp);
	}

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 92 + mExtraHeight); ++n;
	XtCreateManagedWidget("Comment:", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNy, 86 + mExtraHeight); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNleftOffset, 100); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 3); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	comment_text = XtCreateManagedWidget("commenttext", xmTextWidgetClass, w, wargs, n);
	XtAddCallback(comment_text,XmNactivateCallback,(XtCallbackProc)WriteProc,this);
	
	static int sInitializedTranslations = 0;
	static XtTranslations translate_key_up;
	static XtTranslations translate_key_down;
	// get translations only once to avoid memory leaks
	if (!sInitializedTranslations) {
		sInitializedTranslations = 1;
		translate_key_up = XtParseTranslationTable("<Key>osfUp: XSnoedTranslate(+)");
		translate_key_down = XtParseTranslationTable("<Key>osfDown: XSnoedTranslate(-)");
	}
	// override up/down translations for comment_text widget
	XtOverrideTranslations(comment_text, translate_key_up);
	XtOverrideTranslations(comment_text, translate_key_down);
	
	// listen for messages from xsnoed speaker
	data->mSpeaker->AddListener(this);
	
	// listen for messages from global resource speaker
	PResourceManager::sSpeaker->AddListener(this);
}


PAsciiWindow::~PAsciiWindow()
{
	SaveSettings();

	// free comment history buffer
	for (int i=0; i<mNumComments; ++i) {
		XtFree(mComments[i]);
	}
	// free zdab writer
	if (mWriter) {
		delete mWriter;
		mWriter = NULL;
	}
}

void PAsciiWindow::DoMenuCommand(int anID)
{
	mCurMenuItem = anID;
}

// SaveSettings - save settings to ImageData structure
void PAsciiWindow::SaveSettings()
{
	ImageData *data = GetData();
	
	// save defaults
	if (data->ascii_label_allocated) {
		XtFree(data->ascii_label);	// free old label if necessary
	}
	data->ascii_label_allocated = 1;	// set flag indicating we need to free this label
	data->ascii_label = XmTextGetString(format_text);
	if (data->ascii_file_allocated) {
		XtFree(data->ascii_file);
	}
	data->ascii_file_allocated = 1;
	data->ascii_file = XmTextGetString(filename_text);
	data->ascii_auto = mAutoNext;
}

void PAsciiWindow::Listen(int message, void *dataPt)
{
	switch (message) {
	
		case kMessageWillWriteSettings:
			SaveSettings();		// save our settings now so they get written to file
			break;
			
		case kMessageTranslationCallback: {
			TranslationData *transData = (TranslationData *)dataPt;
			// must test to see if our widget generated the callback
			if (transData->widget == comment_text) {
				if (transData->num_params == 1) {
					int dir;
					if (transData->params[0][0] == '+') {
						dir = 1;
					} else {
						dir = -1;
					}
					SwitchComment(dir);
				} else {
					Printf("PAsciiWindow -- wrong number of translation parameters!\n");
				}
			}
		}	break;
	}
}

// Show - show the window (at the proper size!)
void PAsciiWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		Resize(WINDOW_WIDTH, WINDOW_HEIGHT+mExtraHeight);
	}
}

// WriteComment - write comment to file
void PAsciiWindow::WriteComment()
{
	char label[kLabelSize];
	const int kBuffSize = 256;
	char buff[kBuffSize];
	
	int	write_zdab = 0;
	
	FILE *fp = NULL;
	char *filename_buff = XmTextGetString(filename_text);
	
	// get rid of whitespace in filename
	char *delim = " \t\n\r";
	char *filename = strtok(filename_buff, delim);
	static char empty_str[] = "";
	
	if (filename) {
		// write zdab file too if filename starts with a '+'
		if (*filename == '+') {
			write_zdab = 1;
			++filename;
			if (!filename[0]) {
				// look for another token
				filename = strtok(NULL, delim);
				if (!filename) {
					filename = empty_str;
					write_zdab = 0;
				}
			}
		}
		if (filename[0]) {
			fp = fopen(filename,"a");
			if (!fp) {
				Printf("Error opening file %s -- Write failed!\x07\n", filename);
				XtFree(filename_buff);
				return;
			}
		}
	} else {
		filename = empty_str;
	}
	
	// write event to zdab file if requested
	if (write_zdab && strlen(filename) < kBuffSize-5) {
		// construct .zdab filename with same prefix
		strcpy(buff, filename);
		// look for extension in current filename
		char *ext = strrchr(buff, '.');
		if (!ext) {
			ext = strchr(buff, '\0');
		} else {
			// make sure the extension isn't in a directory name
			char *pt = strrchr(buff, '/');
			if (pt && pt>ext) {
				ext = strchr(buff, '\0');
			}
		}
		strcpy(ext, ".zdab");
		if (mWriter && strcmp(mWriter->GetFilename(), buff)) {
			// file name has changed, delete the old writer
			delete mWriter;
			mWriter = NULL;
		}
		if (!mWriter) {
			mWriter = new PZdabWriter(buff);
		}
		if (mWriter && mWriter->IsOpen()) {
			ImageData *data = GetData();
			PmtEventRecord *per = xsnoed_get_event(data);
			if (per) {
				if (!mWriter->Write(per)) {
					if (data->hex_id) {
						Printf("Saved event 0x%lx to output zdab file %s\n", data->event_id, buff);
					} else {
						Printf("Saved event %ld to output zdab file %s\n", data->event_id, buff);
					}
				}
				free(per);
			}
		} else {
			Printf("Error creating output zdab file %s\n", buff);
			delete mWriter;
			mWriter = NULL;
		}
	}
	
	char *format  = XmTextGetString(format_text);
	char *comment = XmTextGetString(comment_text);

	XSnoedWindow::BuildLabelString(GetData(), NULL, format, label);
	
	buff[0] = '\0';
	
	if (mCurMenuItem > 1) {
		char *str = mMenu->GetLabel(mCurMenuItem);
		if (str) {
			sprintf(buff," (%.*s)", kBuffSize-5, str);
			XtFree(str);
		}
	}
	Printf("ASCII Output (%s) %s%s %s\n", filename, label, buff, comment);
	if (fp) {
		fprintf(fp, "%s%s %s\n", label, buff, comment);
		fclose(fp);
	}

	// save comment if it isn't empty
	if (comment[0]) {
		// look through history for an identical comment
		// - remove this entry if it exists
		int first_empty=0;
		for (;;) {
			if (first_empty >= mNumComments) {
				// we didn't find a matching comment
				if (first_empty < kCommentHistoryNum) {
					// we will add the new comment
					++mNumComments;
				} else {
					// free the last comment in the array
					--first_empty;
					XtFree(mComments[first_empty]);
				}
				break;
			}
			if (!strcmp(comment, mComments[first_empty])) {
				// found an identical comment -- erase it
				XtFree(mComments[first_empty]);
				break;
			}
			++first_empty;
		}
		// shift up other comments in array
		if (first_empty) {
			memmove(mComments+1, mComments, first_empty * sizeof(char *));
		}
		// save this comment
		mComments[0] = comment;
	} else {
		// free the comment string since we didn't save it
		XtFree(comment);
	}
	mCurComment = -1;	// reset current comment number
	
	// must free filename and format strings (but not comment string!)
	XtFree(filename_buff);
	XtFree(format);
	
	// reset the string
	setTextString(comment_text, "");
	
	if (mAutoNext) {
		xsnoed_next(GetData(),1);
	}
}

// SwitchComment - show the prev/next comment from the comment history
void PAsciiWindow::SwitchComment(int dir)
{
	int		newComment = mCurComment + dir;
	
	if (newComment>=-1 && newComment<=mNumComments) {
		mCurComment = newComment;
		if (newComment==-1 || newComment==mNumComments) {
			setTextString(comment_text, "");
		} else {
			setTextString(comment_text, mComments[mCurComment]);
		}
	}
}

void PAsciiWindow::WriteProc(Widget w, PAsciiWindow *asciiWin, caddr_t call_data)
{
	asciiWin->WriteComment();
}

void PAsciiWindow::AutoProc(Widget w, PAsciiWindow *asciiWin, caddr_t call_data)
{
	// toggle auto-next feature
	asciiWin->mAutoNext = !asciiWin->mAutoNext;
}

void PAsciiWindow::DoneProc(Widget w, PAsciiWindow *asciiWin, caddr_t call_data)
{
	// delete the window
	delete asciiWin;
}




