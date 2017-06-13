#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <X11/IntrinsicP.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "PEventControlWindow.h"
#include "PResourceManager.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "PZdabWriter.h"
#include "SnoStr.h"
#include "menu.h"
#include "xsnoed.h"
#include "xsnoedstream.h"
#ifdef DEMO_VERSION
#include "XSnoedWindow.h"
#endif

#define WIDGET_HEIGHT		30
#define	WINDOW_WIDTH		350
#define WINDOW_HEIGHT		307
#define WINDOW_MIN_HEIGHT	109

static char *nhit_type_str[] = {
    "NHIT", "Norm", "", "Neck", "FECD", "LG", "OWL", "BUTTS",
#ifdef SNOPLUS
    "CAEN",
#else
    "Shaper", "MUX", "Scope", "General"
#endif
};

static char *nhit_op_str[] = { "", "=", ">", ">=", "<", "<=", "<>", "" };

static MenuStruct goto_menu[] = {
	{ "GTID",	0, 0,	kGotoGTID+1, 		NULL, 0, 0 },
	{ "Run",	0, 0,	kGotoRun+1, 		NULL, 0, 0 },
	{ "Time",	0, 0,	kGotoTime+1,		NULL, 0, 0 },
};


//-----------------------------------------------------------------------------
// callbacks
//
static void fwdProc(Widget w, ImageData *data, caddr_t call_data)
{
	xsnoed_next(data,1);
}

static void backProc(Widget w, ImageData *data, caddr_t call_data)
{
	xsnoed_next(data,-1);
}

static void ffwdProc(Widget w, ImageData *data, caddr_t call_data)
{
	xsnoed_next(data,10);
}

static void fbackProc(Widget w, ImageData *data, caddr_t call_data)
{
	xsnoed_next(data,-10);
}

static void allProc(Widget w, ImageData *data, caddr_t call_data)
{
	data->history_all ^= 1;
	if (data->was_history) {
		showHistory(data,0);
		// make sure the label is updated
		PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
		if (pe_win) {
			pe_win->UpdateHistoryLabel(1);
		}
	}
}

static void filterProc(Widget w, ImageData *data, caddr_t call_data)
{
	PEventControlWindow::SetEventFilter(data);
	PEventControlWindow::UpdateTriggerText(data);
}

static void offProc(Widget w, ImageData *data, caddr_t call_data)
{
	PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win->GetTriggerFlag() != TRIGGER_OFF) {
    	setTriggerFlag(data,TRIGGER_OFF);
    }
}

static void singleProc(Widget w, ImageData *data, caddr_t call_data)
{
	PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win->GetTriggerFlag() != TRIGGER_SINGLE) {
    	PEventControlWindow::SetEventFilter(data);
    	setTriggerFlag(data,TRIGGER_SINGLE);
    }
}

static void contProc(Widget w, ImageData *data, caddr_t call_data)
{
	PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win->GetTriggerFlag() != TRIGGER_CONTINUOUS) {
	    PEventControlWindow::SetEventFilter(data);
	    setTriggerFlag(data,TRIGGER_CONTINUOUS);
	}
}

static void scaleProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
	data->time_interval = call_data->value / 10.0;
	
	if (data->trigger_flag == TRIGGER_CONTINUOUS) {
		PEventControlWindow::UpdateTriggerText(data);
	}
}

//-----------------------------------------------------------------------------------
// PEventControlWindow constructor
//
PEventControlWindow::PEventControlWindow(ImageData *data)
				   : PWindow(data)
{
	int		i, n;
	Arg		wargs[10];
	char	buff[256];
	Widget	w, but;
	
	data->mSpeaker->AddListener(this);	// listen for trigger changed messages
	PResourceManager::sSpeaker->AddListener(this);
	
	mDeadButton = NULL;
	
	mTriggerFlag = data->trigger_flag;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Event Control"); ++n;
	XtSetArg(wargs[n], XmNx, 150); ++n;
	XtSetArg(wargs[n], XmNy, 150); ++n;
	XtSetArg(wargs[n], XmNminWidth, WINDOW_WIDTH); ++n;
	XtSetArg(wargs[n], XmNminHeight, WINDOW_MIN_HEIGHT); ++n;
	SetShell(CreateShell("evtPop",data->toplevel,wargs,n));
	SetMainPane(w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 8); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
	trigger_label = XtCreateManagedWidget("trigger_text",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNy, 33); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtCreateManagedWidget("sep1",xmSeparatorWidgetClass,w,wargs,n);
	
    CreateTriggerRadio(TRIGGER_CONTINUOUS);
    CreateTriggerRadio(TRIGGER_SINGLE);
    CreateTriggerRadio(TRIGGER_OFF);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 82); ++n;
	XtCreateManagedWidget("Period (sec):",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 135); ++n;
	XtSetArg(wargs[n], XmNy, 67); ++n;
	XtSetArg(wargs[n], XmNwidth, 198); ++n;
	XtSetArg(wargs[n], XmNminimum, 1); ++n;
	XtSetArg(wargs[n], XmNmaximum, 100); ++n;
	XtSetArg(wargs[n], XmNdecimalPoints, 1); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(wargs[n], XmNvalue, (int)(data->time_interval * 10)); ++n;
	but = XtCreateManagedWidget("Scale",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)scaleProc,data);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 114); ++n;
	XtSetArg(wargs[n], XmNwidth, 115); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
	history_label = XtCreateManagedWidget("History:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 135); ++n;
	XtSetArg(wargs[n], XmNy, 109); ++n;
	XtSetArg(wargs[n], XmNwidth, 34); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	but = XtCreateManagedWidget("-1",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)backProc,data);

	n = 0;
	XtSetArg(wargs[n], XmNx, 171); ++n;
	XtSetArg(wargs[n], XmNy, 109); ++n;
	XtSetArg(wargs[n], XmNwidth, 34); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	but = XtCreateManagedWidget("+1",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)fwdProc,data);

	n = 0;
	XtSetArg(wargs[n], XmNx, 207); ++n;
	XtSetArg(wargs[n], XmNy, 109); ++n;
	XtSetArg(wargs[n], XmNwidth, 38); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	but = XtCreateManagedWidget("-10",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)fbackProc,data);

	n = 0;
	XtSetArg(wargs[n], XmNx, 247); ++n;
	XtSetArg(wargs[n], XmNy, 109); ++n;
	XtSetArg(wargs[n], XmNwidth, 38); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	but = XtCreateManagedWidget("+10",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)ffwdProc,data);

	n = 0;
	XtSetArg(wargs[n], XmNx, 285); ++n;
	XtSetArg(wargs[n], XmNy, 109 + RADIO_OFFSET); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNset, data->history_all==0); ++n;
	but = XtCreateManagedWidget("Cut",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)allProc,data);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 147); ++n;
	XtCreateManagedWidget("NHIT Thresh:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 135); ++n;
	XtSetArg(wargs[n], XmNy, 142); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	thresh_text = XtCreateManagedWidget("Threshold",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(thresh_text,XmNactivateCallback,(XtCallbackProc)filterProc,data);
	SetNhitText();

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 180); ++n;
	XtCreateManagedWidget("Trigger Mask:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 135); ++n;
	XtSetArg(wargs[n], XmNy, 175); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	trigger_text = XtCreateManagedWidget("Triggers",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(trigger_text,XmNactivateCallback,(XtCallbackProc)filterProc,data);
	SetTriggerMaskText();

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 213); ++n;
	XtCreateManagedWidget("PMT/NCD:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 135); ++n;
	XtSetArg(wargs[n], XmNy, 208); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	pmt_ncd_text = XtCreateManagedWidget("PmtNcd",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(pmt_ncd_text,XmNactivateCallback,(XtCallbackProc)filterProc,data);
	SetPmtNcdLogicText();

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 241); ++n;
	XtSetArg(wargs[n], XmNwidth, 60); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	goto_button = XtCreateManagedWidget("Goto:", xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(goto_button,XmNactivateCallback,(XtCallbackProc)GotoProc,this);

	// create goto option menu
	Widget sub_menu = XmCreatePulldownMenu(w, "optionMenu", NULL, 0);
	CreateMenu(sub_menu, goto_menu, XtNumber(goto_menu), this);
	
	// create option menu widget
	n = 0;
	XtSetArg(wargs[n], XmNx, 72); ++n;
	XtSetArg(wargs[n], XmNy, 240); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 0); ++n;
	XtSetArg(wargs[n], XmNmarginWidth, 0); ++n;
	XtSetArg(wargs[n], XmNsubMenuId, sub_menu); ++n;
	Widget option_menu = XmCreateOptionMenu(w, "goto_opt", wargs, n);
	XtManageChild(option_menu);

	// set current 'Goto' menu item
	MenuList *cur_item = mMenu->FindMenuItem(data->goto_run_num+1);
	if (cur_item) {
		n = 0;
		XtSetArg(wargs[n], XmNmenuHistory, cur_item->button); ++n;
		XtSetValues(option_menu, wargs, n);
	}
	
	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 160); ++n;
	XtSetArg(wargs[n], XmNy, 241); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
//	XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
	event_text = XtCreateManagedWidget("EventNum",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(event_text,XmNactivateCallback,(XtCallbackProc)GotoProc,this);
	setTextString(event_text,"0");
	
	for (i=0; i<NUM_WRITERS; ++i) {
		n = 0;
		XtSetArg(wargs[n], XmNx, 16); ++n;
		XtSetArg(wargs[n], XmNy, 274+33*i); ++n;
		XtSetArg(wargs[n], XmNwidth, 108); ++n;
//		XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
		sprintf(buff,"Save%d:",i+1);
		outfile_button[i] = XtCreateManagedWidget(buff,xmPushButtonWidgetClass,w,wargs,n);
		XtAddCallback(outfile_button[i],XmNactivateCallback,(XtCallbackProc)WriteProc,this);
	
		n = 0;
		XtSetArg(wargs[n], XmNleftOffset, 135); ++n;
		XtSetArg(wargs[n], XmNy, 274+33*i); ++n;
		XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
		XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
		XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
		XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
//		XtSetArg(wargs[n], XmNheight, WIDGET_HEIGHT); ++n;
		sprintf(buff,"Outfile%d",i+1);
		outfile_text[i] = XtCreateManagedWidget(buff,xmTextWidgetClass,w,wargs,n);
		XtAddCallback(outfile_text[i],XmNactivateCallback,(XtCallbackProc)WriteProc,this);
		setTextString(outfile_text[i],data->output_file[i]);
	}
	
	static int sInitializedTranslations = 0;
	static XtTranslations translate_tab;
	// get translations only once to avoid memory leaks
	if (!sInitializedTranslations) {
		sInitializedTranslations = 1;
		translate_tab = XtParseTranslationTable("<Key>Tab: XSnoedTranslate()");
	}
	// override Tab translations for threshold, trigger and pmt/ncd widgets
	XtOverrideTranslations(thresh_text,  translate_tab);
	XtOverrideTranslations(trigger_text, translate_tab);
	XtOverrideTranslations(pmt_ncd_text, translate_tab);

	UpdateEventNumber();
    UpdateTriggerText();
}

void PEventControlWindow::CreateTriggerRadio(int num)
{
    int n;
    Arg wargs[24];
    Widget but;
    ImageData *data = GetData();
    
    switch (num) {
        case TRIGGER_CONTINUOUS:
            n = 0;
            XtSetArg(wargs[n], XmNx, 16); ++n;
            XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
            XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
            XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
            XtSetArg(wargs[n], XmNset, mTriggerFlag == TRIGGER_CONTINUOUS); ++n;
            but = XtCreateManagedWidget("Continuous",xmToggleButtonWidgetClass,GetMainPane(),wargs,n);
            XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)contProc, data);
	        break;
	    case TRIGGER_SINGLE:
            n = 0;
            XtSetArg(wargs[n], XmNx, 165); ++n;
            XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
            XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
            XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
            XtSetArg(wargs[n], XmNset, mTriggerFlag == TRIGGER_SINGLE); ++n;
            but = XtCreateManagedWidget("Next",xmToggleButtonWidgetClass,GetMainPane(),wargs,n);
            XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)singleProc, data);
            break;
        case TRIGGER_OFF:
            n = 0;
            XtSetArg(wargs[n], XmNx, 265); ++n;
            XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
            XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
            XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
            XtSetArg(wargs[n], XmNset, mTriggerFlag == TRIGGER_OFF); ++n;
            but = XtCreateManagedWidget("Stop",xmToggleButtonWidgetClass,GetMainPane(),wargs,n);
            XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)offProc, data);
            break;
        default:
            return;
    }
    trigger_radio[num] = but;
}

void PEventControlWindow::Listen(int message, void *dataPt)
{
	switch (message) {
	
		case kMessageTriggerChanged: {
		    int oldTrigger = mTriggerFlag;
			mTriggerFlag = GetData()->trigger_flag;	// save new trigger type
            if (oldTrigger == mTriggerFlag) break;
            
			// unset old radio
//	        XmToggleButtonSetState(trigger_radio[oldTrigger], 0, FALSE);
			
			// (hack! -- destroy the old button since the button dies if you toggle the state)
			if (mDeadButton) {
			    XtDestroyWidget(mDeadButton);
			}
			mDeadButton = trigger_radio[oldTrigger];
			// unmanage the button now, but delete it later so it doesn't confuse
			// X11 by getting destroyed from (possibly) inside its own callback
			XtUnmanageChild(mDeadButton);
			CreateTriggerRadio(oldTrigger);
			
			// set new radio
			XmToggleButtonSetState(trigger_radio[mTriggerFlag], 1, FALSE);

			UpdateTriggerText();	// update the text according to the new trigger setting
		}   break;
			
		case kMessageHistoryEventChanged:
			UpdateHistoryLabel(1);
			break;
			
		case kMessageTimeFormatChanged:
			if (GetData()->goto_run_num == kGotoTime) {
				UpdateEventNumber();
			}
			break;
			
		case kMessageTranslationCallback: {
			TranslationData *transData = (TranslationData *)dataPt;
			// check for correct number of parameters (zero)
			if (transData->num_params == 0) {	
				char *str;
				if (transData->widget == thresh_text) {
					str = XmTextGetString(thresh_text);
					SetNhitLogic(GetData(), str);
					XtFree(str);
					SetNhitText();	// reset text to standard form
				} else if (transData->widget == trigger_text) {
					str = XmTextGetString(trigger_text);
					SetTriggerMaskLogic(GetData(), str);
					XtFree(str);
					SetTriggerMaskText();	// reset text to standard form
				} else if (transData->widget == pmt_ncd_text) {
					str = XmTextGetString(pmt_ncd_text);
					SetPmtNcdLogic(GetData(), str);
					XtFree(str);
					SetPmtNcdLogicText();	// reset text to standard form
				}
			}
		}	break;
	}
}

void PEventControlWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		Resize(WINDOW_WIDTH,WINDOW_HEIGHT);
	}
}

/* Set event number in window */
void PEventControlWindow::UpdateEventNumber(ImageData *data)
{
	PEventControlWindow	*pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
	if (pe_win) {
		pe_win->UpdateEventNumber();
	}
}

void PEventControlWindow::UpdateEventNumber()
{
	char		buff[64];
	ImageData	*data = GetData();
	
	switch (data->goto_run_num) {
		case kGotoGTID:
			sprintf(buff,data->hex_id ? "0x%lx" : "%ld",data->event_id);
			break;
		case kGotoRun:
			sprintf(buff,"%ld",data->run_number);
			break;
		case kGotoTime:
			if (data->event_time != 0.0) {
				/* display time in Sudbury, local or GMT time zone */
				struct tm *tms = getTms(data->event_time, data->time_zone);
				sprintf(buff,"%.2d/%.2d/%d %.2d:%.2d:%.2d",
							 tms->tm_mon+1, tms->tm_mday, tms->tm_year+1900,
							 tms->tm_hour, tms->tm_min, tms->tm_sec);
			} else {
				strcpy(buff,"00/00/0000 00:00:00");
			}
			break;
		default:
			return;
	}
	setTextString(event_text,buff);
}

void PEventControlWindow::UpdateTriggerText(ImageData *data)
{
	PEventControlWindow	*pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
	if (pe_win) {
		pe_win->UpdateTriggerText();
	}
}

void PEventControlWindow::UpdateTriggerText()
{
	char		buff[1024];
	int			len;
	ImageData	*data = GetData();
	
	if (data->trigger_flag == TRIGGER_CONTINUOUS) {
		len = sprintf(buff,"Continuous  %.1fs",data->time_interval);
	} else if (data->trigger_flag == TRIGGER_SINGLE) {
		len = sprintf(buff,"Single");
	} else {
		len = sprintf(buff,"<stopped>");
	}
	if (data->trigger_flag && (data->nhit_logic_num || data->trigger_bitmask ||
	    data->pmt_logic_num || data->ncd_logic_num))
	{
	    if (data->nhit_logic_num) {
	        len = PrintNhitLogic(data, buff);
	    }
		if (data->trigger_bitmask) {
			len += sprintf(buff+len," (");
			len += PrintTriggerMask(data, buff+len);
			len += sprintf(buff+len,")");
		}
		if (data->pmt_logic_num || data->ncd_logic_num) {
			len += sprintf(buff+len," ");
			len += PrintPmtNcdLogic(data, buff+len);
		}
	}
	setLabelString(trigger_label, buff);
	XtResizeWidget(trigger_label, 500, 20, 0);
}


void PEventControlWindow::UpdateHistoryLabel(int isHistory)
{
	char	buff[64];
	
	if (isHistory) {
		int num = -GetData()->history_evt;
		// add "+" sign to positive numbers (but not to zero, so %+d won't do it)
		if (GetData()->history_all) {
			sprintf(buff,"History [%s%d]:",num>0 ? "+" : "", num);
		} else {
			sprintf(buff,"History (%s%d):",num>0 ? "+" : "", num);
		}
		setLabelString(history_label, buff);
	} else {
		setLabelString(history_label,"History:");
	}
}

int PEventControlWindow::PrintTriggerMask(ImageData *data, char *buff)
{
	int			i, len;
	char		*sep;
	u_int32		mask;
	
	len = 0;
	for (i=0; i<32; ++i) {
		mask = (1L << i);
		if (mask & data->trigger_bitmask) {
			if (mask & data->trigger_bits_on) {
				sep = "+";
			} else if (mask & data->trigger_bits_off) {
				sep = "-";
			} else if (mask & data->trigger_bits_always) {
				sep = "*";
			} else {
				sep = "";
			}
			len += sprintf(buff+len, "%s%s%s", 
						   len ? "," : "",
						   sep,
						   SnoStr::sXsnoedTrig[i]);
		}
	}
	return(len);
}

int PEventControlWindow::PrintNhitLogic(ImageData *data, char *buff)
{
    int n = 0;
	if (data->nhit_logic_num) {
		n = 0;
		for (int i=0; ;) {
			char *typePt = nhit_type_str[data->nhit_logic_array[i].type];
			char *opPt = nhit_op_str[data->nhit_logic_array[i].op];
			char valStr[32];
			sprintf(valStr,"%d",data->nhit_logic_array[i].nhit);
			// don't show default names for simple case
			if (data->nhit_logic_num == 1 &&
				data->nhit_logic_array[i].type == NHIT_PMT_ANY)
		    {
                typePt = "";
                if (data->nhit_logic_array[i].op == NHIT_GT_OR_EQ) {
                    opPt = "";
				}
			} else if (data->nhit_logic_array[i].op == NHIT_GREATER_THAN &&
			           data->nhit_logic_array[i].nhit == 0)
			{
			    // don't show >0 for any threshold but 
                opPt = "";
                valStr[0] = '\0';
			}
			n += sprintf(buff+n, "%s%s%s", typePt, opPt, valStr);
			if (++i >= data->nhit_logic_num) break;
			if (data->nhit_logic_array[i-1].or_index >= i) {
				n += sprintf(buff+n, " or ");
			} else {
			    n += sprintf(buff+n, ",");
			}
		}
	} else {
		n = sprintf(buff,"0");
	}
	return(n);
}

void PEventControlWindow::SetPmtNcdLogic(ImageData *data, char *str)
{
	char	    *pt;
	static char *delim = " ,\t\r\n";
	
	// reset existing logic
	data->pmt_logic_num = data->ncd_logic_num = 0;

	/* convert to upper case */
	for (pt=str; ;++pt) {
		char ch = *pt;
		if (!ch) break;
		if (ch>='a' && ch<='z') {
			*pt = ch - 'a' + 'A';
		}
	}
    /* parse the PMT/NCD names */
	while ((pt = strtok(str, delim)) != NULL) {
	    str = NULL; // set str to NULL to continue parsing this string
	    // get logic type
	    EPmtNcdLogic logic_type;
	    switch (*pt) {
	        case '+':
	            logic_type = kPmtNcd_Need;
	            ++pt;
	            break;
	        case '-':
	            logic_type = kPmtNcd_Refuse;
	            ++pt;
	            break;
	        case '*':
	            logic_type = kPmtNcd_Always;
	            ++pt;
	            break;
	        default:
	            logic_type = kPmtNcd_Want;
	            break;
	    }
	    if (!*pt) break;    // make sure we have a token
		// get number in brackets if it exists
		char *brac = strchr(pt,'(');
		int num;
		if (brac) {
		    num = atoi(brac + 1);
		}
		int logic_num;
		// is this a PMT?  (Note: P1,P2, etc would be NCD names...)
		if (*pt == 'P' && (pt[1] < '0' || pt[1] > '9')) {
		    // parse as a PMT
		    logic_num = data->pmt_logic_num;
		    if (logic_num >= kMaxPmtNcdLogic) {
		        printf("Exceeded maximum number of PMT's in logic statement\n");
		        continue;
		    }
		    if (brac) {
		        // PMT specified by index
		        if ((unsigned)num >= (unsigned)data->num_tube_coordinates) {
		            printf("Bad PMT number %d\n", num);
		            continue;
		        }
		    } else {
		        // search for this PMT id
		        int bc = bc2int(pt,'P');
		        if (bc < 0) {
		            printf("Bad PMT barcode: %s\n", pt);
		            continue;
		        }
		        Pmt *pmt = data->tube_coordinates;
		        for (num=0; num<data->num_tube_coordinates; ++num, ++pmt) {
		            if (pmt->tube == bc && !(pmt->status & PMT_LOW_GAIN)) break;
		        }
		        if (num >= data->num_tube_coordinates) {
		            printf("No PMT with barcode: %s\n", pt);
		            continue;
		        }
		    }
		    // add this to the PMT logic
            data->pmt_logic_array[logic_num].num = num;
            data->pmt_logic_array[logic_num].logic = logic_type;
            ++data->pmt_logic_num;

		} else {

		    // parse as an NCD
		    logic_num = data->ncd_logic_num;
		    if (logic_num >= kMaxPmtNcdLogic) {
		        printf("Exceeded maximum number of NCD's in logic statement\n");
		        continue;
		    }
		    if (brac) {
		        // NCD specified by index
		        // find this NCD by string number
		        int string_num = num;
		        for (num=0; num<(int)kMaxNCDs; ++num) {
		            if (string_num == data->ncdMap[num].string_number) {
		                break;
		            }
		        }
		        if ((unsigned)num >= (unsigned)kMaxNCDs) {
		            printf("No NCD exists with string number: %d\n", string_num);
		            continue;
		        }
		    } else {
		        // find this NCD by name
		        for (num=0; num<(int)kMaxNCDs; ++num) {
		            if (!strcmp(pt, data->ncdMap[num].label)) break;
		        }
		        if (num >= (int)kMaxNCDs) {
		            printf("No NCD exists with name: %s\n", pt);
		            continue;
		        }
		    }
		    // add this to the NCD logic
            data->ncd_logic_array[logic_num].num = num;
            data->ncd_logic_array[logic_num].logic = logic_type;
            ++data->ncd_logic_num;
		}
    }
}

int PEventControlWindow::PrintPmtNcdLogic(ImageData *data, char *buff)
{
    int         i, n;
	char        *pt = buff;
	PmtNcdLogic *logic;
	
    for (i=0; i<data->pmt_logic_num; ++i) {
        n = pt - buff;
        if (n > 225) {
            printf("PMT/NCD logic string too long\n");
            break;
        }
        if (n) *(pt++) = ',';
        logic = data->pmt_logic_array + i;
        switch (logic->logic) {
            case kPmtNcd_Need:
                *(pt++) = '+';
                break;
            case kPmtNcd_Refuse:
                *(pt++) = '-';
                break;
            case kPmtNcd_Always:
                *(pt++) = '*';
                break;
            default:
                break;
        }
        Pmt *pmt = data->tube_coordinates + logic->num;
        // use barcode for any tube except low gain (which will have a
        // normal channel with the same barcode)
        if (pmt->tube >= 0 && !(pmt->status & PMT_LOW_GAIN)) {
            pt += sprintf(pt,"%s",int2bc(pmt->tube,'P'));
        } else {
            *(pt++) = 'P';
        }
        pt += sprintf(pt,"(%d)",logic->num);
    }
    for (i=0; i<data->ncd_logic_num; ++i) {
        n = pt - buff;
        if (n > 225) {
            printf("PMT/NCD logic string too long\n");
            break;
        }
        if (n) *(pt++) = ',';
        logic = data->ncd_logic_array + i;
        switch (logic->logic) {
            case kPmtNcd_Need:
                *(pt++) = '+';
                break;
            case kPmtNcd_Refuse:
                *(pt++) = '-';
                break;
            case kPmtNcd_Always:
                *(pt++) = '*';
                break;
            default:
                break;
        }
        if (data->ncdMap[logic->num].label[0]) {
            pt += sprintf(pt,"%s",data->ncdMap[logic->num].label);
        } else {
            *(pt++) = 'N';
        }
        pt += sprintf(pt,"(%d)",(int)data->ncdMap[logic->num].string_number);
    }
    *pt = 0;    // make sure string is null terminated (necessary if no logic)
    return(pt - buff);
}

void PEventControlWindow::SetPmtNcdLogicText()
{
	ImageData	*data = GetData();
	char		*buff = data->pmt_ncd_logic_buff;
	
	PrintPmtNcdLogic(data, buff);
	setTextString(pmt_ncd_text, buff);
}

void PEventControlWindow::SetTriggerMaskText()
{
	ImageData	*data = GetData();
	char		*buff = data->trigger_mask_buff;
	
	if (!PrintTriggerMask(data, buff)) {
		strcpy(buff,"<any>");
	}
	setTextString(trigger_text,buff);
	
	// set settings to new string
	data->trigger_mask = buff;
}

// SetTriggerMaskLogic - set ImageData trigger logic variables from str
void PEventControlWindow::SetTriggerMaskLogic(ImageData *data, char *str)
{
	char	type;
	int		i, j;
	char	ch, *pt;
	static char *delim = " ,\t\r\n";
	
	/* convert to upper case */
	for (pt=str; ;++pt) {
		ch = *pt;
		if (!ch) break;
		if (ch>='a' && ch<='z') {
			*pt = ch - 'a' + 'A';
		}
	}
	/* parse the trigger names */
	pt = strtok(str, delim);
	data->trigger_bitmask = 0;
	data->trigger_bits_on = 0;
	data->trigger_bits_off = 0;
	data->trigger_bits_always = 0;
	while (pt) {
		if (*pt=='+' || *pt=='-' || *pt=='*') {
			type = *pt;
			++pt;
			/* find next token */
			if (!*pt) {
				pt = strtok(NULL,delim);
				if (!pt) break;
			}
		} else {
			type = '\0';
		}
		// special case: remove leading "NHIT"
// huh?
//		if (strstr(pt, "NHIT") == pt) pt += 4;
		// look for the closest matching trigger name
		int match_len = 0;
		long match_mask = 0;
		for (i=0; i<32; ++i) {
			char *pt2 = SnoStr::sXsnoedTrig[i];
			// see how many characters are the same
			for (j=0; pt[j]==pt2[j]; ++j) { 
				if (!pt[j]) {	// did the whole string match?
					match_mask = (1L << i);
					match_len = j;
					i = 32;		// don't compare with remaining strings
					break;
				}
			}
			if (i >= 32) break;	// done if complete match found
			if (j > match_len) {
				match_mask = (1L << i);
				match_len = j;
			}
		}
#ifndef SNOPLUS
		// special case: expand "NCD" into NCDMUX and NCDSHAP bits
		if ((match_mask & TRIGGER_NCD) && match_len <= 3) {
		    match_mask = TRIGGER_NCD;
		}
#endif
		if (match_mask) {
			data->trigger_bitmask |= match_mask;
			switch (type) {
				case '+':
					data->trigger_bits_on |= match_mask;
					break;
				case '-':
					data->trigger_bits_off |= match_mask;
					break;
				case '*':
					data->trigger_bits_always |= match_mask;
					break;
			}
		}
		pt = strtok(NULL, delim);	// continue parsing string
	}
}

void PEventControlWindow::SetNhitText()
{
	ImageData	*data = GetData();

    PrintNhitLogic(data, data->nhit_logic_buff);
    
	setTextString(thresh_text,data->nhit_logic_buff);

	// set settings to new string
	data->nhit_logic = data->nhit_logic_buff;
}

void PEventControlWindow::SetNhitLogic(ImageData *data, char *str)
{
	int			thresh, flag, is_or, num;
	char		*pt;
	int     	pmtType = -1;
	enum {
		kFlagEqualTo		= NHIT_EQUAL_TO,
		kFlagGreaterThan	= NHIT_GREATER_THAN,
		kFlagLessThan		= NHIT_LESS_THAN,
		kFlagGotNumber		= 0x08
	};

	data->nhit_logic_num = 0;
	data->nhit_pmt_mask = 0;
	flag = 0;
	thresh = 0;
	is_or = 0;
	for (pt=str; ; ++pt) {
		if (*pt>='0' && *pt<='9') {
			thresh = thresh * 10 + (*pt - '0');
			flag |= kFlagGotNumber;	// set flag indicating we got a number
		} else {
		    if (pmtType>=0 && !flag && (*pt==',' || *pt=='\0' || *pt=='|' || isalpha(*pt))) {
		        // assume pmtType > 0 if not specified
		        flag = kFlagGreaterThan | kFlagGotNumber;
		    }
			if (flag & kFlagGotNumber) {
				// we have reached the end of the integer number
				// - set the threshold appropriately
				if (flag == kFlagGotNumber) {
					// use default logic if not specified
					flag |= kFlagGreaterThan | kFlagEqualTo;
				}
				// don't save any condition of ">= 0"
				if (thresh!=0 || flag!=(kFlagGotNumber|kFlagGreaterThan|kFlagEqualTo)) {
				
					ENhitOp	nhit_op = (ENhitOp)(flag & 0x07);
					
					if (nhit_op != NHIT_BAD_OP) {
						num = data->nhit_logic_num;
						
						// add this condition to nhit logic
						data->nhit_logic_array[num].nhit = thresh;
						data->nhit_logic_array[num].op = nhit_op;
						if (pmtType > NHIT_PMT_ANY) {
						    data->nhit_logic_array[num].type = (ENhitPMT)pmtType;
							data->nhit_pmt_mask |= (1 << pmtType);
						} else {
						    data->nhit_logic_array[num].type = NHIT_PMT_ANY;
						}
						if (is_or && num) {
							// add this to the list of 'or' pointers.
							// NOTE: all pointers must be forward referenced, except
							// for the last 'or' in the list, which refers to the first in the list
							data->nhit_logic_array[num].or_index = data->nhit_logic_array[num-1].or_index;
							data->nhit_logic_array[num-1].or_index = num;
						} else {
							data->nhit_logic_array[num].or_index = num;	// set to no 'or' (or with itself)
						}
		
						if (++data->nhit_logic_num >= kMaxNhitCuts) {
							break;	// reached maximum number of conditions
						}
					}
				}
				// reset flags
				thresh = 0;
				flag = 0;
				is_or = 0;
				pmtType = -1;
			}
			/* continue evaluating string */
			switch (*pt) {
				case '>':
					flag |= kFlagGreaterThan;
					break;
				case '<':
					flag |= kFlagLessThan;
					break;
				case '=':
					flag |= kFlagEqualTo;
					break;
				case ' ':
				case '\t':
				case '\0':
				case ',':
					break;
				default:
					if (pmtType<0 && !flag) {
						switch (toupper(*pt)) {
							case '|':	// Or
								is_or = 1;
								break;
							case 'B':	// Butts
								pmtType = NHIT_PMT_BUTTS;
								break;
							case 'F':	// FECD
								pmtType = NHIT_PMT_FECD;
								break;
#ifndef SNOPLUS
						    case 'G':   // General NCD record
						        pmtType = NHIT_NCD_GENERAL;
						        break;
#endif
							case 'L':	// Low gain
								pmtType = NHIT_PMT_LOW_GAIN;
								break;
#ifndef SNOPLUS
							case 'M':
							    pmtType = NHIT_NCD_MUX;
							    break;
#endif
							case 'N':	// Nhit (any) or Neck or Normal
								switch (toupper(pt[1])) {
								    case 'H':
								        pmtType = NHIT_PMT_ANY;
								        break;
									case 'E':
										pmtType = NHIT_PMT_NECK;
										break;
									case 'O':
										pmtType = NHIT_PMT_NORMAL;
										break;
								}
								break;
							case 'O':	// Owl or Or
								switch (toupper(pt[1])) {
									case 'R':
										is_or = 1;
										break;
									case 'W':
										pmtType = NHIT_PMT_OWL;
										break;
								}
								break;
#ifndef SNOPLUS
							case 'S':   // Shaper or Scope
							    switch (toupper(pt[1])) {
							        case 'C':
							            pmtType = NHIT_NCD_SCOPE;
							            break;
							        case 'H':
							            pmtType = NHIT_NCD_SHAPER;
							            break;
							    }
							    break;
#endif
						}
						// skip to the end of this token
						while (isalpha(pt[1])) ++pt;
					}
					break;
			}
		}
		if (!*pt) break;
	}
}

/* set trigger logic variables from strings in text widgets */
void PEventControlWindow::SetEventFilter(ImageData *data)
{
	char	*str;
	PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
	
	if (pe_win) {
		str = XmTextGetString(pe_win->thresh_text);
		SetNhitLogic(data, str);
		XtFree(str);

		pe_win->SetNhitText();	// reset text to standard form

		str = XmTextGetString(pe_win->trigger_text);
		SetTriggerMaskLogic(data, str);
		XtFree(str);
		
		pe_win->SetTriggerMaskText();	// reset text to standard form

		str = XmTextGetString(pe_win->pmt_ncd_text);
		SetPmtNcdLogic(data, str);
		XtFree(str);
		
		pe_win->SetPmtNcdLogicText();	// reset text to standard form
	}
}

void PEventControlWindow::SetGotoType(int type)
{
	
	if (GetData()->goto_run_num != type) {
		GetData()->goto_run_num = type;
/*
		int			n = 0;
		Arg			wargs[10];
		static char	*strs[] = { "Goto GTID:", "Goto Run:", "Goto Time:" };
		setLabelString(goto_button, strs[type]);
		
		n = 0;
		XtSetArg(wargs[n], XmNwidth, 108); ++n;
		XtSetValues(goto_button, wargs, n);*/
		UpdateEventNumber();
	}
}

void PEventControlWindow::GotoProc(Widget w,PEventControlWindow *pe_win, caddr_t call_data)
{
	ImageData *	data = pe_win->GetData();
	char	  *	pt = XmTextGetString(pe_win->event_text);
	long		gotoValue;
	static char	*errStr = "Date format error -- use format mm/dd/yyyy hh:mm:ss\x07\n";
	
	if (data->goto_run_num == kGotoTime) {
		// convert time string
		struct tm tms;
		time_t zeroTime = 0;
		// initialize tms with zero time in GMT
		tms = *gmtime(&zeroTime);
		int gotCode = 0;	// bitmask for codes obtained
		char *pt2 = strtok(pt, " \t\n\r");
		while (pt2) {
			int count;
			int dateCode = 0;	// 0=none, 1=date, 2=time
			int val[3];
			memset(val, 0, sizeof(val));
			for (count=0; ; ) {
				val[count] = atoi(pt2);
				if (++count >= 3) break;	// all done
				char *pt3 = strchr(pt2,'/');	// look for next element in date string
				if (pt3) {
					if (dateCode == 2) {
						Printf(errStr);
						return;
					}
					dateCode = 1;	// this is a date
					pt2 = pt3 + 1;
					continue;
				}
				pt3 = strchr(pt2,':');
				if (pt3) {
					if (dateCode == 1) {
						Printf(errStr);
						return;
					}
					dateCode = 2;	// this is a time
					pt2 = pt3 + 1;		// point to start of next value
					continue;
				}
				break;	// no more elements
			}
			
			switch (dateCode) {
				case 0:		// unknown
					Printf(errStr);
					return;
				case 1:		// date
					if (gotCode & 0x01 || count != 3 || val[0]>12) {
						Printf(errStr);
						return;
					}
					tms.tm_mon = val[0] - 1;
					tms.tm_mday = val[1];
					if (val[2] > 1900) val[2] -= 1900;		// allow "1999" and "2000"
					else if (val[2] < 80) val[2] += 100;	// allow "99" and "00"
					tms.tm_year = val[2];
					break;
				case 2:		// time
					if (gotCode & 0x02) {
						Printf(errStr);
						return;
					}
					tms.tm_hour = val[0];
					tms.tm_min = val[1];
					tms.tm_sec = val[2];
					break;
			}
			gotCode |= dateCode;
			pt2 = strtok(NULL, " \t\n\r");
		}
		if (!(gotCode & 0x01)) {	// require the date string
			Printf(errStr);
			return;
		}
		gotoValue = getTime(&tms, data->time_zone);
	} else if (!memcmp(pt,"0x",2)) {
		// convert hex number
		sscanf(pt,"%lx",&gotoValue);
	} else {
		// convert decimal number
		gotoValue = atol(pt);
	}
	XtFree(pt);
	
	if (!data->infile) {
		Printf("No event file open!\x07\n");
	} else {
		// reset the trigger if it was armed - PH 10/08/99
		if (data->trigger_flag != TRIGGER_OFF) {
			setTriggerFlag(data, TRIGGER_OFF);
		}
		// were we going to a run number?
		if (data->goto_run_num == kGotoRun) {
			// if filename is standard run number, try to open appropriate run file
			int is_standard_run_file = 0;
			char filebuff[FILELEN];
			strncpy(filebuff, data->mEventFile.GetString(), FILELEN);
			filebuff[FILELEN-1] = '\0';
			char *filename = strrchr(filebuff, '/');
			if (filename) ++filename;
			else filename = filebuff;
			char *fmt = "SNO_##########_###.zdab";
			for (char *pt=filename; ; ++pt, ++fmt) {
				switch (*fmt) {
					case '#':
						if (*pt>='0' && *pt<='9') continue;
						break;
					case '\0':
						if (*pt == '\0') {
							is_standard_run_file = 1;
						}
						break;
						
					default:
						if (*pt == *fmt) continue;
						break;
				}
				break;
			}
			if (is_standard_run_file) {
				long run_num = atol(filename+4);
				// skip to new file if it is a different run
				if (run_num != gotoValue) {
					data->event_num = 0;
					sprintf(filename,"SNO_%.10ld_000.zdab",(long)gotoValue);
					// try to open it first
					FILE *fp = fopen(filebuff, "r");
					if (fp) {
						fclose(fp);
						data->mEventFile.SetString(filebuff);	// set new filename
						open_event_file(data,0);
					} else {
						// print message and leave old file open
						Printf("Can't open file %s\x07\n", filebuff);
					}
				} else {
					Printf("Run file %ld already open\x07\n", run_num);
				}
				return;
			}
		}
		load_event(data, gotoValue, data->goto_run_num);
	}
}


void PEventControlWindow::WriteProc(Widget w,PEventControlWindow *pe_win, caddr_t call_data)
{
#ifdef DEMO_VERSION
	if (XSnoedWindow::IsProtected()) return;
#endif
	for (int i=0; i<NUM_WRITERS; ++i) {
		if (w==pe_win->outfile_text[i] || w==pe_win->outfile_button[i]) {
			ImageData *	data = pe_win->GetData();
			char *pt = XmTextGetString(pe_win->outfile_text[i]);
			char *theString = pt;
			int	len = strlen(pt);
			const short kBuffSize=512;
			if (len < kBuffSize) {
				char buff[kBuffSize];
				strcpy(buff, pt);
				// remove spaces and control characters from filename
				pt = strtok(buff, " \t\n\r");
				if (!pt) pt="";	// send pointer to NULL string (don't send NULL pointer)
				xsnoed_save_event(data, pt, i);
			} else {
				Printf("Output file name too long!\n");
			}
			XtFree(theString);
			break;
		}
	}
}

void PEventControlWindow::DoMenuCommand(int anID)
{
	switch (anID) {
		case kGotoGTID+1:
		case kGotoRun+1:
		case kGotoTime+1:
			SetGotoType(anID - 1);
			break;
	}
}



