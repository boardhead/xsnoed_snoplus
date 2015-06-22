#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifndef NO_FORK
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include "PPrintWindow.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "ImageData.h"
#include "PUtils.h"

// definitions for capturing and printing images
// (these can be redefined for different system types if necessary)
#ifndef NO_FORK
const short	kNumCaptureArgs = 2;
// Note: This array must have space for 2 additional entries
static char	*sCaptureCommand[kNumCaptureArgs+2]	= { "import", "-silent", NULL, NULL };
#endif

// static string definitions
static char *	sPrintTitle[]	= { "Print Postscript Image (Vector)",
									"Print Window (Raster)" };

static char *	sPrintLabel[]	= { "Print Image to:", "Print Window to:" };

static char *	sPrintMessage[][2] = { { "Printer Command:",
								 		 "Filename (.ps or .eps):" },
								 	   { "Printer Command:",
								 		 "Filename (.gif, .jpg, .ps, .eps, etc...):" } };
								 
PPrintWindow::PPrintWindow(ImageData *data, EPrintType printType)
			: PWindow(data)
{
	int			n;
	XmString	str;
	Arg			wargs[16];
	Widget		w, but;
	
	mWarnDialog = NULL;
#ifdef NO_FORK
	// VAX doesn't support Print Window or printing to device
	mPrintType = kPrintImage;
	mToFile = 1;
#else
	mPrintType = printType;		// kPrintImage or kPrintWindow
	mToFile = data->print_to;
#endif

  	n = 0;
  	mSaveCol = data->image_col;
  	mSaveLabel = data->show_label;
  	
	XtSetArg(wargs[n], XmNtitle, sPrintTitle[printType]); ++n;
	XtSetArg(wargs[n], XmNx, 350); ++n;
	XtSetArg(wargs[n], XmNy, 350); ++n;
	XtSetArg(wargs[n], XmNminWidth, 320); ++n;
	XtSetArg(wargs[n], XmNminHeight, 180); ++n;
	SetShell(CreateShell("printPop",data->toplevel,wargs,n));
	w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0);
	SetMainPane(w);

	n = 0;
	XtSetArg(wargs[n], XmNx, 20); ++n;
	XtSetArg(wargs[n], XmNy, 140); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNrecomputeSize, FALSE); ++n;
	print_button = XtCreateManagedWidget(mToFile ? "Save" : "Print",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(print_button, XmNactivateCallback, (XtCallbackProc)PrintProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 220); ++n;
	XtSetArg(wargs[n], XmNy, 140); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	cancel_button = XtCreateManagedWidget("Cancel",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(cancel_button, XmNactivateCallback, (XtCallbackProc)CancelProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 20); ++n;
	XtSetArg(wargs[n], XmNy, 14); ++n;
	target_label = XtCreateManagedWidget(sPrintLabel[printType], xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 154); ++n;
	XtSetArg(wargs[n], XmNy, 10 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, mToFile == 0); ++n;
	but = XtCreateManagedWidget("Printer",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)ToPrinterProc, this);
	target_radio[0] = but;

	n = 0;
	XtSetArg(wargs[n], XmNx, 244); ++n;
	XtSetArg(wargs[n], XmNy, 10 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, mToFile == 1); ++n;
	but = XtCreateManagedWidget("File",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)ToFileProc, this);
	target_radio[1] = but;

	n = 0;
	XtSetArg(wargs[n], XmNx, 20); ++n;
	XtSetArg(wargs[n], XmNy, 42); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
	str = XmStringCreateLtoR(sPrintMessage[printType][mToFile],XmFONTLIST_DEFAULT_TAG);
	XtSetArg(wargs[n], XmNlabelString, str); ++n;
	cmd_label = XtCreateManagedWidget("StringLabel", xmLabelWidgetClass, w, wargs, n);
	XmStringFree(str);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 20); ++n;
	XtSetArg(wargs[n], XmNy, 63); ++n;
	XtSetArg(wargs[n], XmNwidth, 280); ++n;
	print_text = XtCreateManagedWidget("printtext", xmTextWidgetClass, w, wargs, n);
	XtAddCallback(print_text,XmNactivateCallback,(XtCallbackProc)PrintProc,this);
	setTextString(print_text, data->print_string[mToFile]);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 20); ++n;
	XtSetArg(wargs[n], XmNy, 105); ++n;
	XtSetArg(wargs[n], XmNset, data->print_label != 0); ++n;
	label_toggle = XtCreateManagedWidget("Label",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(label_toggle, XmNvalueChangedCallback, (XtCallbackProc)PrintLabelProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 95); ++n;
	XtSetArg(wargs[n], XmNy, 105); ++n;
	XtSetArg(wargs[n], XmNset, (data->print_col&0x01) != 0); ++n;
	col_toggle = XtCreateManagedWidget("White Bkg",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(col_toggle, XmNvalueChangedCallback, (XtCallbackProc)ColoursProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 203); ++n;
	XtSetArg(wargs[n], XmNy, 105); ++n;
	XtSetArg(wargs[n], XmNset, (data->print_col&0x02) != 0); ++n;
	grey_toggle = XtCreateManagedWidget("Greyscale",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(grey_toggle, XmNvalueChangedCallback, (XtCallbackProc)GreyProc, this);

    SetColours(data->print_col);
    ShowLabels(data->print_label);

	data->mSpeaker->AddListener(this);
}

PPrintWindow::~PPrintWindow()
{
	SaveSettings();		// save our settings
	
	// restore original color/label settings
	SetColours(mSaveCol);
	ShowLabels(mSaveLabel);
}

void PPrintWindow::Listen(int message, void *dataPt)
{
	ImageData	*data;
	
	switch (message) {
		case kMessageColoursChanged:
			// the outside colours changed
			mSaveCol = GetData()->image_col;
			setToggle(col_toggle, (mSaveCol & 0x01) != 0);
			setToggle(grey_toggle, (mSaveCol & 0x02) != 0);
			break;
		case kMessageShowLabelChanged:
			// the label has been changed externally
			mSaveLabel = GetData()->show_label;
			setToggle(label_toggle, mSaveLabel);
			break;
		case kMessageWillWriteSettings:
			SaveSettings();		// make sure the settings are updated
			// temporarily restore original color/label settings
			data = GetData();
			data->image_col = mSaveCol;
			data->show_label = mSaveLabel;
			break;
		case kMessageWriteSettingsDone:
			// return the color/label settings to their proper values
			data = GetData();
			data->image_col = data->print_col;
			data->show_label = data->print_label;
			break;
	}
}

void PPrintWindow::SaveSettings()
{
	ImageData *data = GetData();
	
	// save current print settings
	data->print_col = data->image_col;
	data->print_label = data->show_label;

	// save our print string
	char *print_string = data->print_string[data->print_to];
	char *str = XmTextGetString(print_text);
	strncpy(print_string, str, FILELEN-1);
	XtFree(str);
}

void PPrintWindow::SetPrintType(EPrintType printType)
{
	if (mPrintType != printType) {
		mPrintType = printType;
		SetTitle(sPrintTitle[printType]);
		setLabelString(cmd_label, sPrintMessage[printType][mData->print_to]);
		setLabelString(target_label, sPrintLabel[printType]);
	}
	Raise();
}

void PPrintWindow::SetColours(int colourSet)
{
	Ignore();	// ignore messages generated by ourself
	PResourceManager::SetColours(colourSet);
	Ignore(0);
}

void PPrintWindow::ShowLabels(int on)
{
	ImageData *data = GetData();
	Ignore();	// ignore messages generated by ourself
	setLabel(data, on);
	Ignore(0);
}

void PPrintWindow::SetTarget(int to_file)
{
	ImageData *data = GetData();
	if (mToFile != to_file) {
		// turn off other radio button
		setToggle(target_radio[data->print_to], 0);
		setLabelString(cmd_label, sPrintMessage[mPrintType][to_file]);
		// save our current settings
		SaveSettings();
		// change print string
		setTextString(print_text, data->print_string[to_file]);
		// change print button label
		if (to_file)
		    setLabelString(print_button, "Save");
		else
		    setLabelString(print_button, "Print");
		// finally, update the print target variable in our ImageData
		mToFile = data->print_to = to_file;
	} else {
		// must turn radio button back on
		setToggle(target_radio[mToFile], 1);
	}
}

// NotifyRaised - callback made when another window is raised
void PPrintWindow::NotifyRaised(PImageWindow *theWindow)
{
	// continue with printing
	ContinuePrinting(theWindow);
}

// PromptToClick - prompt the user to click on a window for printing
void PPrintWindow::PromptToClick()
{
	char		buff[256];
	static char	*type_str[] = { "image", "window" };
	static char *to_str[] = { "print", "save" };

	// hide all other widgets
	XtUnmapWidget(print_button);
	XtUnmapWidget(cancel_button);
	XtUnmapWidget(print_text);
	XtUnmapWidget(col_toggle);
	XtUnmapWidget(grey_toggle);
	XtUnmapWidget(label_toggle);
	XtUnmapWidget(target_label);
	XtUnmapWidget(target_radio[0]);
	XtUnmapWidget(target_radio[1]);
	
	// change the command label to a prompt
	sprintf(buff,"     Now click on the %s to %s.", type_str[mPrintType], to_str[mToFile]);
	if (mPrintType == kPrintWindow) {
		char *pt = strchr(buff,'\0');
		sprintf(pt,"\n\n     Or click and drag to define an area\n"
				   "     of the screen to %s.", to_str[mToFile]);
	}
	setLabelString(cmd_label, buff);

	// write similar message to console output
	Printf("Click on the %s to %s...\n", type_str[mPrintType], to_str[mToFile]);

	// must update this window now for "Print Window"
	// because we won't handle any more events until
	// our exec returns after printing is done.
	if (mPrintType == kPrintWindow) {
		XmUpdateDisplay(GetMainPane());
	}
}

void PPrintWindow::DoPrint()
{
	char		*home = getenv("HOME");
	ImageData	*data = GetData();
	static char	*delim = " \t\n\r";
	
	// save current settings
	SaveSettings();
	
	// make a working copy of the text
	char *print_string = data->print_string[mToFile];
	strcpy(mPrintName, print_string);
	
	if (home) {
		strcpy(mTempFilename,home);
		strcat(mTempFilename,"/");
	} else {
		mTempFilename[0] = '\0';
	}
	strcat(mTempFilename,"xsnoed_tmp.ps");
	
	// create argument list for printer
	mPrintFlags = 0;
	mArgc = 0;
	mArgs[mArgc] = strtok(mPrintName, delim);
	while (mArgs[mArgc]) {
		if (mPrintType == kPrintImage) {	// check for "Print Image" options
			if (!strcmp(mArgs[mArgc], "-landscape")) {
				mPrintFlags |= kPrintLandscape;	// set landscape mode
				--mArgc;	// eat this argument
			}
		}
		// leave room for one additional argument (filename) plus NULL termination
		if (++mArgc >= kMaxPrintArgs-2) break;
		// get the next argument
		mArgs[mArgc] = strtok(NULL, delim);
	}

	// make sure we have at least one argument
	if (!mArgs[0] || !*mArgs[0]) {
		Printf("Print syntax error\x07\n");
		return;
	}
		
	if (!mToFile) {
		// add temporary filename to list of arguments for print command
		mArgs[mArgc++] = mTempFilename;
		// argument list must be NULL terminated
		mArgs[mArgc] = NULL;	
	} else {
		// argument list must be NULL terminated
		mArgs[mArgc] = NULL;	
		// check to see if output file exists
		FILE *fp = fopen(mArgs[mArgc-1],"r");
		if (fp) {
			fclose(fp);
			// open warning dialog
			if (mWarnDialog) {
				XtDestroyWidget(mWarnDialog);
				mWarnDialog = NULL;
			}
			XmString	str;
			Arg			wargs[10];
			int			n;
			str = XmStringCreateLocalized("File exists.  Overwrite it?  ");
			n = 0;
			XtSetArg(wargs[n], XmNtitle, "Warning"); ++n;
			XtSetArg(wargs[n], XmNmessageString, str); ++n;
			XtSetArg(wargs[n], XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON); ++n;
			mWarnDialog = XmCreateWarningDialog(GetShell(), "xsnoedWarn",wargs,n);
			XmStringFree(str);	// must free the string
			XtUnmanageChild(XmMessageBoxGetChild(mWarnDialog,XmDIALOG_HELP_BUTTON));
			XtAddCallback(mWarnDialog,XmNcancelCallback,(XtCallbackProc)WarnCancelProc,this);
			XtAddCallback(mWarnDialog,XmNokCallback,(XtCallbackProc)WarnOKProc,this);
			XtAddCallback(mWarnDialog,XtNdestroyCallback,(XtCallbackProc)WarnDestroyProc,this);
			XtManageChild(mWarnDialog);
			return;
		}
	}
	
	ContinuePrinting(NULL);
}

/* continue printing of specified window (or select window if aWindow==NULL) */
void PPrintWindow::ContinuePrinting(PImageWindow *aWindow)
{
	int			delete_this = 0;
	pid_t		pid;

#ifndef NO_FORK
	if (mPrintType == kPrintWindow) {
		pid = fork();
	} else
#endif
	if (aWindow) {
		// print the image to file
		if (!aWindow->GetImage()->Print(mArgs[mArgc-1], mPrintFlags)) {
			Printf("Error writing image to %s\n",mArgs[mArgc-1]);
			return;
		}

		// set bogus pid to bypass fork logic
		pid = 1;
	} else {
		PromptToClick();
		// wait for a window to be raised
		ArmNotifyRaised();
		return;
	}
	if (pid == -1) {
		Printf("Fork 1 error while capturing image\x07\n");
	} else if (pid) {
	
		if (mPrintType == kPrintWindow) {
			PromptToClick();			
			waitpid(pid, NULL, 0);		// wait for capture program to finish
		}
		
		if (mToFile) {
			Printf("Done printing %s to file.\n", mPrintType ? "window" : "image");
			delete_this = 1;	// delete the print window
		}
#ifndef NO_FORK
		else {
			pid = fork();
			if (pid == -1) {
				Printf("Fork 2 error while capturing image\x07\n");
				delete_this = 1;	// delete the print window
			} else if (pid) {
				waitpid(pid, NULL, 0);	// wait for print program to finish
				Printf("Done writing image to %s.\n",mArgs[0]);
				unlink(mTempFilename);		// erase the temporary file
				delete_this = 1;	// delete the print window
			} else {
				// print the temporary image file
				execvp(mArgs[0], mArgs);
				Printf("%s error executing %s\x07\n",strerror(errno),mArgs[0]);
				// this form of exit does not call cleanup routines
				// - we don't want it to -- otherwise settings will be saved, etc.
				_exit(1);
			}
		}
#endif // NO_FORK
		
		
	}
#ifndef NO_FORK
	else {
		if (mToFile) {
			int i;
			// make room for additional capture arguments
			for (i=kMaxPrintArgs-1; i>=kNumCaptureArgs; --i) {
				mArgs[i] = mArgs[i-kNumCaptureArgs];
			}
			while (i >= 0) {
				mArgs[i] = sCaptureCommand[i];
				--i;
			}
			execvp(mArgs[0], mArgs);	// execute capture program with specified options
		} else {
			// add temporary file to list of capture arguments
			// (note, capture argument list is already NULL terminated)
			sCaptureCommand[kNumCaptureArgs] = mTempFilename;
			// execute capture program
			execvp(sCaptureCommand[0], sCaptureCommand);
		}
		Printf("Error executing %s\x07\n",sCaptureCommand[0]);
		_exit(1);
	}
#endif // NO_FORK

	// delete this print window when we are all done
	if (delete_this) {
		delete this;
	}
}

void PPrintWindow::CancelProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	// delete the print dialog
	delete printWin;
}

void PPrintWindow::ToPrinterProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
#ifdef NO_FORK
	Printf("Sorry, Print to device not supported by this version of XSNOED\x07\n");
	setToggle(printWin->target_radio[0], 0);	// turn "Printer" radio back off
#else
	printWin->SetTarget(0);
#endif
}

void PPrintWindow::ToFileProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	printWin->SetTarget(1);
}

void PPrintWindow::ColoursProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	ImageData *data = printWin->GetData();
	printWin->SetColours(data->image_col ^ kWhiteBkg);
}

void PPrintWindow::GreyProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	ImageData *data = printWin->GetData();
	printWin->SetColours(data->image_col ^ kGreyscale);
}

void PPrintWindow::PrintLabelProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	ImageData *data = printWin->GetData();
	printWin->ShowLabels(data->show_label ^ 0x01);
}

// print image to file or printer
void PPrintWindow::PrintProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	printWin->DoPrint();
}

void PPrintWindow::WarnCancelProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	// delete the warning dialog
	XtDestroyWidget(printWin->mWarnDialog);
	printWin->mWarnDialog = NULL;
}

void PPrintWindow::WarnOKProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	XtDestroyWidget(printWin->mWarnDialog);
	printWin->mWarnDialog = NULL;
	// continue with printing
	printWin->ContinuePrinting(NULL);
}

// the warning dialog was destroyed
void PPrintWindow::WarnDestroyProc(Widget w, PPrintWindow *printWin, caddr_t call_data)
{
	// must verify that it is the current dialog being destroyed
	// (could a delayed callback from one we destroyed ourself already)
	if (printWin->mWarnDialog == w) {
		printWin->mWarnDialog = NULL;
	}
}

