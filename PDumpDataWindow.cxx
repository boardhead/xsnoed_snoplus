//
// File:		PDumpDataWindow.cxx
//
// Description:	Allows user to record ASCII-based information about events to file.
//
// Created:		04/21/00 Mark Howe. mostly copied from PAsciiWindow.cxx
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
#include "PDumpDataWindow.h"
#include "PResourceManager.h"
#include "PEventHistogram.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "ImageData.h"
#include "xsnoed.h"
#include "openfile.h"

#define WINDOW_WIDTH			320
#define WINDOW_HEIGHT			165
#define MIN_WINDOW_WIDTH		320


PDumpDataWindow::PDumpDataWindow(ImageData *data)
			: PWindow(data)
{
	int		n;
	Arg		wargs[16];
	Widget		w, but, but2;
	    
	mNumComments = 0;
	mCurComment	 = -1;
	mOverwrite	 = 0;
	
  	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Dump Displayed Data"); ++n;
	XtSetArg(wargs[n], XmNx, 200); ++n;
	XtSetArg(wargs[n], XmNy, 100); ++n;
	XtSetArg(wargs[n], XmNminWidth, MIN_WINDOW_WIDTH); ++n;
	XtSetArg(wargs[n], XmNminHeight, WINDOW_HEIGHT); ++n;
	SetShell(CreateShell("dumpPop",data->toplevel,wargs,n));
	w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0);
	SetMainPane(w);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 121); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	but = XtCreateManagedWidget("Write",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)WriteProc, this);
    	    	
	n = 0;
	XtSetArg(wargs[n], XmNy, 121); ++n;
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
	setTextString(filename_text, data->dump_file);
    	    
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 55); ++n;
	XtCreateManagedWidget("Comment:", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNy, 49); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNleftOffset, 100); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 3); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	comment_text = XtCreateManagedWidget("commenttext", xmTextWidgetClass, w, wargs, n);
	XtAddCallback(comment_text,XmNactivateCallback,(XtCallbackProc)WriteProc,this);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 49); ++n;
	XtSetArg(wargs[n], XmNy, 85); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, 1); ++n;
	append_radio = XtCreateManagedWidget("Append",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(append_radio, XmNvalueChangedCallback, (XtCallbackProc)AppendProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 160); ++n;
	XtSetArg(wargs[n], XmNy, 85); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	overwrite_radio = XtCreateManagedWidget("Overwrite",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(overwrite_radio, XmNvalueChangedCallback, (XtCallbackProc)OverwriteProc, this);
    	
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

PDumpDataWindow::~PDumpDataWindow()
{
	SaveSettings();

	// free comment history buffer
	for (int i=0; i<mNumComments; ++i) {
		XtFree(mComments[i]);
	}
}

// SaveSettings - save settings to ImageData structure
void PDumpDataWindow::SaveSettings()
{
	ImageData *data = GetData();
	
	if (data->dump_file_allocated) {
		XtFree(data->dump_file);
	}
	data->dump_file_allocated = 1;
	data->dump_file = XmTextGetString(filename_text);
}

void PDumpDataWindow::Listen(int message, void *dataPt)
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
					Printf("PDumpDataWindow -- wrong number of translation parameters!\n");
				}
			}
		}	break;
	}
}

// Show - show the window (at the proper size!)
void PDumpDataWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		Resize(WINDOW_WIDTH, WINDOW_HEIGHT);
	}
}

// DumpDisplayData - dump displayed data to file
void PDumpDataWindow::DumpDisplayData()
{
	FILE *fp = NULL;
	ImageData *data = GetData();
	char *filename_buff = XmTextGetString(filename_text);

	// get rid of whitespace in filename
	char *delim = " \t\n\r";
	char *filename = strtok(filename_buff, delim);
	char *comment = XmTextGetString(comment_text);

	if (filename && filename[0]) {
		if (mOverwrite) {
			fp = fopen(filename,"w");
		} else {
			fp = fopen(filename,"a");
		}
		if (!fp) {
			Printf("Error opening file %s -- Write failed!\x07\n", filename);
			XtFree(filename_buff);
			return;
		}
	} else {
		fp = stdout;	// write to stdout
	}
	int numHits = data->hits.num_nodes;
	char buff[128];
	PEventHistogram::GetHistogramLabel(data, buff);

	fprintf(fp,"# -----------------------------------\n");
	fprintf(fp,"# Comment: %s\n",comment);	
	fprintf(fp,"# Event GTID: %ld\n",(long)data->event_id);	
	fprintf(fp,"# Hits displayed: %d\n",numHits);
	fprintf(fp,"# Data: %s\n", buff);
	
	if (numHits) {
		HitInfo *hi = data->hits.hit_info;
		for (int i=0; i<numHits; ++i, ++hi) {
			fprintf(fp,"%2d %2d %2d %g\n", hi->crate, hi->card, hi->channel, getHitVal(data, hi));
		}
	}
	if (filename && filename[0]) {
		fclose(fp);
		char *pt;
		if (mOverwrite) pt = "overwrite";
		else pt = "append";
		Printf("Dumped GTID %ld %s to %s (%s)\n", (long)data->event_id, buff, filename, pt);
	}

	// save comment if it isn't empty
	if (comment[0]) {
		// look through history for an identical comment
		// - remove this entry if it exists
		int first_empty=0;
		for (;;) {
			if (first_empty >= mNumComments) {
				// we didn't find a matching comment
				if (first_empty < kCommentHistoryNum2) {
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
	
	// reset the string
	setTextString(comment_text, "");
}

// SwitchComment - show the prev/next comment from the comment history
void PDumpDataWindow::SwitchComment(int dir)
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

void PDumpDataWindow::WriteProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data)
{
	dumpWin->DumpDisplayData();
}

void PDumpDataWindow::AppendProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data)
{
	// append to file
	dumpWin->mOverwrite = 0;
	setToggle(dumpWin->overwrite_radio, 0);
}

void PDumpDataWindow::OverwriteProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data)
{
	// overwrite file
	dumpWin->mOverwrite = 1;
	setToggle(dumpWin->append_radio, 0);
}

void PDumpDataWindow::DoneProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data)
{
	// delete the window
	delete dumpWin;
}




