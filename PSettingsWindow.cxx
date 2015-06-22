#include <stdio.h>
#include <stdlib.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include "PSettingsWindow.h"
#include "PEventControlWindow.h"
#include "PHitInfoWindow.h"
#include "PSpeaker.h"
#include "PResourceManager.h"
#include "PPrintWindow.h"
#include "XSnoedWindow.h"
#include "PUtils.h"
#include "xsnoed.h"

#define	WINDOW_WIDTH			360
#define WINDOW_HEIGHT			389
#define HIT_INFO_DELTA_HEIGHT	69

#ifdef LESSTIF
#define SLIDER_ROUNDOFF		1
#else
#define SLIDER_ROUNDOFF		0
#endif

const double	kMaxUpdateTime	= 0.5;	// maximum drawing time for continuous slider updates
const int		kMaxNHIT		= 400;	// max NHIT for continuous slider updates

// static member declarations
double PSettingsWindow::sUpdateTime	= 0;


//-----------------------------------------------------------------------------------
// PSettingsWindow constructor
//
PSettingsWindow::PSettingsWindow(ImageData *data)
			   : PWindow(data)
{
	int		n;
	Arg		wargs[10];
	Widget	w, but;
	long	label_flags = data->mMainWindow->GetLabelFlags();
	
	if (data->angle_rad<0 || data->angle_rad>2) {
		data->angle_rad = 0;
	}
	
	mSave_hex_id		= data->hex_id;
	mSave_time_zone		= data->time_zone;
	mSave_hit_size		= data->hit_size;
	mSave_ncd_size		= data->ncd_size;
	mSave_angle_rad		= data->angle_rad;
	mSave_show_label	= data->show_label;
	mSave_hit_xyz		= data->hit_xyz;
	mSave_save_config	= data->save_config;
	strcpy(mSave_label_format, data->label_format);
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "XSNOED Settings"); ++n;
	XtSetArg(wargs[n], XmNx, 350); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNminWidth, WINDOW_WIDTH); ++n;
	XtSetArg(wargs[n], XmNminHeight, WINDOW_HEIGHT); ++n;
	SetShell(CreateShell("setPop",data->toplevel,wargs,n));
	SetMainPane(w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 348); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	but = XtCreateManagedWidget("OK",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)OkProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNy, 348); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	but = XtCreateManagedWidget("Cancel",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)CancelProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 14); ++n;
	XtCreateManagedWidget("GTID's:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 10 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->hex_id == 0); ++n;
	but = XtCreateManagedWidget("Decimal",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)DecProc, this);
	dec_radio = but;
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 190); ++n;
	XtSetArg(wargs[n], XmNy, 10 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->hex_id != 0); ++n;
	but = XtCreateManagedWidget("Hexadecimal",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)HexProc, this);
	hex_radio = but;
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 44); ++n;
	XtCreateManagedWidget("Angles:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->angle_rad == 0); ++n;
	but = XtCreateManagedWidget("Degrees",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)AngleProc, this);
	angle_radio[0] = but;
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 190); ++n;
	XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->angle_rad == 1); ++n;
	but = XtCreateManagedWidget("Radians",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)AngleProc, this);
	angle_radio[1] = but;
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 280); ++n;
	XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->angle_rad == 2); ++n;
	but = XtCreateManagedWidget("None",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)AngleProc, this);
	angle_radio[2] = but;
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 74); ++n;
	XtCreateManagedWidget("Times:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 70 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->time_zone == kTimeZoneSudbury); ++n;
	but = XtCreateManagedWidget("Sudbury",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)SudburyProc, this);
	sudbury_radio = but;

	n = 0;
	XtSetArg(wargs[n], XmNx, 190); ++n;
	XtSetArg(wargs[n], XmNy, 70 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->time_zone == kTimeZoneLocal); ++n;
	but = XtCreateManagedWidget("Local",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)LocalProc, this);
	local_radio = but;

	n = 0;
	XtSetArg(wargs[n], XmNx, 280); ++n;
	XtSetArg(wargs[n], XmNy, 70 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
	XtSetArg(wargs[n], XmNset, data->time_zone == kTimeZoneUTC); ++n;
	but = XtCreateManagedWidget("UTC",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)UTCProc, this);
	utc_radio = but;

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 104); ++n;
	XtCreateManagedWidget("Hit Info:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 100 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNset, data->hit_xyz != 0); ++n;
	but = XtCreateManagedWidget("Show Hit XYZ",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)XYZProc, data);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 134); ++n;
	XtCreateManagedWidget("Settings:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 130 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNset, data->save_config != 0); ++n;
	but = XtCreateManagedWidget("Save Settings on Quit",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)SaveConfigProc, data);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 170); ++n;
	XtCreateManagedWidget("Hit Size:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 160); ++n;
	const int kSliderWidth = 244;
	XtSetArg(wargs[n], XmNwidth, kSliderWidth); ++n;
	XtSetArg(wargs[n], XmNminimum, 10); ++n;
	// the slider scale is set to give even values when sliding...
	XtSetArg(wargs[n], XmNmaximum, 10 + (kSliderWidth - 38) * 5 + SLIDER_ROUNDOFF); ++n;
	XtSetArg(wargs[n], XmNdecimalPoints, 2); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(wargs[n], XmNvalue, (int)(data->hit_size * 100 + 0.5)); ++n;
	but = XtCreateManagedWidget("HitSize",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)ScaleChangedProc,data);
	XtAddCallback(but,XmNdragCallback,(XtCallbackProc)ScaleMovedProc,data);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 210); ++n;
	XtCreateManagedWidget("NCD Size:",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNwidth, kSliderWidth); ++n;
	XtSetArg(wargs[n], XmNminimum, 10); ++n;
	// the slider scale is set to give even values when sliding...
	XtSetArg(wargs[n], XmNmaximum, 10 + (kSliderWidth - 38) * 5 + SLIDER_ROUNDOFF); ++n;
	XtSetArg(wargs[n], XmNdecimalPoints, 2); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(wargs[n], XmNvalue, (int)(data->ncd_size * 100 + 0.5)); ++n;
	but = XtCreateManagedWidget("NCDSize",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)NCDChangedProc,data);
	XtAddCallback(but,XmNdragCallback,(XtCallbackProc)NCDMovedProc,data);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 245 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNset, data->show_label); ++n;
	label_toggle = XtCreateManagedWidget("Label:",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(label_toggle,XmNvalueChangedCallback,(XtCallbackProc)LabelProc, this);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 245); ++n;
	XtSetArg(wargs[n], XmNwidth, 55); ++n;
	XtSetArg(wargs[n], XmNset, (label_flags & kLabelRun) != 0); ++n;
	but = XtCreateManagedWidget("Clear",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)ClearProc, this);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 163); ++n;
	XtSetArg(wargs[n], XmNy, 245); ++n;
	XtSetArg(wargs[n], XmNwidth, 55); ++n;
	XtSetArg(wargs[n], XmNset, (label_flags & kLabelRun) != 0); ++n;
	but = XtCreateManagedWidget("+Run",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)AddRunProc, this);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 226); ++n;
	XtSetArg(wargs[n], XmNy, 245); ++n;
	XtSetArg(wargs[n], XmNwidth, 55); ++n;
	XtSetArg(wargs[n], XmNset, (label_flags & kLabelGTID) != 0); ++n;
	but = XtCreateManagedWidget("+GTID",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)AddGTIDProc, this);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 289); ++n;
	XtSetArg(wargs[n], XmNy, 245); ++n;
	XtSetArg(wargs[n], XmNwidth, 55); ++n;
	XtSetArg(wargs[n], XmNset, (label_flags & kLabelTime) != 0); ++n;
	but = XtCreateManagedWidget("+Time",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)AddTimeProc, this);
	
/*	n = 0;
	XtSetArg(wargs[n], XmNx, 320); ++n;
	XtSetArg(wargs[n], XmNy, 245); ++n;
	XtSetArg(wargs[n], XmNset, (label_flags & kLabelNhit) != 0); ++n;
	but = XtCreateManagedWidget("+NHit",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)AddNHitProc, this);
*/	
	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 16); ++n;
	XtSetArg(wargs[n], XmNy, 279); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	label_text = XtCreateManagedWidget("LabelFormat",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(label_text,XmNactivateCallback,(XtCallbackProc)LabelFormatProc,this);
	setTextString(label_text, data->label_format);
    
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 311 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNset, data->reset_sum ? TRUE : FALSE); ++n;
	clear_sum_toggle = XtCreateManagedWidget("Reset Event Sum every",xmToggleButtonWidgetClass,w,wargs,n);
    
	n = 0;
	XtSetArg(wargs[n], XmNx, 208); ++n;
	XtSetArg(wargs[n], XmNy, 311); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNwidth, 65); ++n;
	clear_sum_text = XtCreateManagedWidget("ClearText",xmTextWidgetClass,w,wargs,n);
	char buff[128];
	sprintf(buff,"%g",data->reset_sum_time);
	setTextString(clear_sum_text, buff);
    
	n = 0;
	XtSetArg(wargs[n], XmNx, 281); ++n;
	XtSetArg(wargs[n], XmNy, 316); ++n;
	XtCreateManagedWidget("minutes",xmLabelWidgetClass,w,wargs,n);
	
    data->mSpeaker->AddListener(this);
}

void PSettingsWindow::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageShowLabelChanged:
			setToggle(label_toggle, GetData()->show_label);
			break;
	}
}

void PSettingsWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		/* set size of our window */
		Resize(WINDOW_WIDTH,WINDOW_HEIGHT);
	}
}

void PSettingsWindow::SetHexID(int on)
{
	ImageData *data = GetData();
	if (data->hex_id != on) {
		data->hex_id = on;
		setToggle(on ? dec_radio : hex_radio, 0);
		newTitle(data);
		sendMessage(data, kMessageGTIDFormatChanged);
	} else {
		setToggle(on ? hex_radio : dec_radio, 1);
	}
}

void PSettingsWindow::HexProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->SetHexID(1);
}

void PSettingsWindow::DecProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->SetHexID(0);
}

void PSettingsWindow::SetTimeZone(int tz)
{
	ImageData *data = GetData();
	if (data->time_zone != tz) {
		switch (data->time_zone) {
			default:	// case kTimeZoneSubury:
				setToggle(sudbury_radio, 0);
				break;
			case kTimeZoneLocal:
				setToggle(local_radio, 0);
				break;
			case kTimeZoneUTC:
				setToggle(utc_radio, 0);
				break;
		}
		data->time_zone = tz;
		sendMessage(data, kMessageTimeFormatChanged);
	} else {
		switch (data->time_zone) {
			default:	// case kTimeZoneSubury:
				setToggle(sudbury_radio, 1);
				break;
			case kTimeZoneLocal:
				setToggle(local_radio, 1);
				break;
			case kTimeZoneUTC:
				setToggle(utc_radio, 1);
				break;
		}
	}
}

void PSettingsWindow::SudburyProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->SetTimeZone(kTimeZoneSudbury);
}

void PSettingsWindow::LocalProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->SetTimeZone(kTimeZoneLocal);
}

void PSettingsWindow::UTCProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->SetTimeZone(kTimeZoneUTC);
}

void PSettingsWindow::SetAngle(int flag)
{
	ImageData *data = GetData();
	if (data->angle_rad != flag) {
		// turn off old toggle
		setToggle(angle_radio[data->angle_rad], 0);
		data->angle_rad = flag;
		switch (flag) {
			case 0:		// 0=degrees
				data->angle_conv = 180 / PI;
				break;
			default:	// 1=radians, 2=not shown
				data->angle_conv = 1.0;
				break;
		}
		sendMessage(data, kMessageAngleFormatChanged);
	} else {
		// change the radio back
		setToggle(angle_radio[data->angle_rad], 1);
	}
}

void PSettingsWindow::AngleProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	for (int i=0; i<3; ++i) {
		if (w == set_win->angle_radio[i]) {
			set_win->SetAngle(i);
		}
	}
}

void PSettingsWindow::SetLabel(int show_label)
{
	setLabel(GetData(), show_label);
}

void PSettingsWindow::LabelProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	ImageData *data = set_win->GetData();
	if (!data->show_label) {
		// make sure label format is current when labels are turned on
		set_win->SetLabelFormat();
	}
	setLabel(data, !data->show_label);
}

int PSettingsWindow::SetLabelFormat()
{
	int	labelChanged;
	char *str = XmTextGetString(label_text);
	ImageData *data = GetData();
	if (strcmp(data->label_format,str)) {
		strncpy(data->label_format, str, FORMAT_LEN-1);
		labelChanged = 1;
	} else {
		labelChanged = 0;
	}
	XtFree(str);	// must free the string
	return(labelChanged);
}

void PSettingsWindow::LabelFormatProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	ImageData *data = set_win->GetData();
	
	if (set_win->SetLabelFormat()) {
		if (!data->label_format[0]) {
			// label is "" -- turn off labels
			setLabel(data, 0);
		} else if (!data->show_label) {
			// label is non-null, but labels were off
			// so turn on labels
			setLabel(data, 1);
		} else {
			// the label format has changed while labels are on...
			// get main window to make the new label string
			data->mMainWindow->LabelFormatChanged();
		}
	}
}

void PSettingsWindow::AddLabel(char *aString)
{
	char	buff[512];
	
	char 	*str = XmTextGetString(label_text);
	
	if (strlen(str) < 450) {
		strcpy(buff, str);
		if (buff[0]) strcat(buff,"  ");
		strcat(buff, aString);
		setTextString(label_text, buff);
		LabelFormatProc(label_text, this, NULL);
	}
	XtFree(str);	// must free the string
}

void PSettingsWindow::ClearProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	setTextString(set_win->label_text, "");
	LabelFormatProc(set_win->label_text, set_win, NULL);
}

void PSettingsWindow::AddRunProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->AddLabel("Run: %rn");
}

void PSettingsWindow::AddGTIDProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->AddLabel("GTID: %gt");
}

void PSettingsWindow::AddTimeProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->AddLabel("%da %ti");
}

void PSettingsWindow::AddNHitProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	set_win->AddLabel("Nhit: %nh");
}

void PSettingsWindow::OkProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	ImageData *data = set_win->GetData();
	// update label settings if changed without pressing RETURN
	if (set_win->SetLabelFormat()) {
		if (data->show_label) {
			data->mMainWindow->LabelFormatChanged();
		}
	}
	// update sum settings
	char *str = XmTextGetString(set_win->clear_sum_text);
	float clear_time = atof(str);
	if (clear_time) {
	    data->reset_sum_time = clear_time;
	} else {
	    printf("Bad time entered in Reset Sum option\n");
	}
	XtFree(str);
	Arg warg;
	XtSetArg(warg, XmNset, &data->reset_sum);
	XtGetValues(set_win->clear_sum_toggle, &warg, 1);

	delete set_win;
}

void PSettingsWindow::CancelProc(Widget w, PSettingsWindow *set_win, caddr_t call_data)
{
	ImageData *data = set_win->GetData();
	
	if (set_win->mSave_hex_id != data->hex_id) {
		set_win->SetHexID(set_win->mSave_hex_id);
	}
	if (set_win->mSave_time_zone != data->time_zone) {
		set_win->SetTimeZone(set_win->mSave_time_zone);
	}
	if (set_win->mSave_hit_size != data->hit_size) {
		SetHitSize(data, set_win->mSave_hit_size);
	}
	if (set_win->mSave_ncd_size != data->ncd_size) {
		SetNCDSize(data, set_win->mSave_ncd_size);
	}
	if (set_win->mSave_angle_rad != data->angle_rad) {
		set_win->SetAngle(set_win->mSave_angle_rad);
	}
	if (set_win->mSave_show_label != data->show_label) {
		set_win->SetLabel(set_win->mSave_show_label);
	}
	if (strcmp(data->label_format, set_win->mSave_label_format)) {
		strcpy(data->label_format, set_win->mSave_label_format);
		data->mMainWindow->LabelFormatChanged();
	}
	if (set_win->mSave_hit_xyz != data->hit_xyz) {
		SetHitXYZ(data, set_win->mSave_hit_xyz);
	}
	data->save_config = set_win->mSave_save_config;
	
	delete set_win;
}

void PSettingsWindow::SetHitSize(ImageData *data, float hit_size)
{
	if (data->hit_size != hit_size) {
		data->hit_size = hit_size;
		// update necessary windows if any hits are visible
		if (data->num_disp != 0) {
			sendMessage(data, kMessageHitSizeChanged);
		}
	}
}

void PSettingsWindow::SetNCDSize(ImageData *data, float ncd_size)
{
	if (data->ncd_size != ncd_size) {
		data->ncd_size = ncd_size;
		// update necessary windows
		sendMessage(data, kMessageNCDSizeChanged);
	}
}

void PSettingsWindow::ScaleMovedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
	if (data->event_nhit>0 && data->num_disp<kMaxNHIT && sUpdateTime<kMaxUpdateTime) {
		// update as slider moves if not too slow
		double cur_time = double_time();
		SetHitSize(data, call_data->value / 100.0);
		// do update immediately so we can time how long it takes
		PWindow::HandleUpdates();
		sUpdateTime = double_time() - cur_time;
	}
}

void PSettingsWindow::ScaleChangedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
	SetHitSize(data, call_data->value / 100.0);
	sUpdateTime = 0;
}

void PSettingsWindow::NCDMovedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
	if (data->num_disp<kMaxNHIT && sUpdateTime<kMaxUpdateTime) {
		// update as slider moves if not too slow
		double cur_time = double_time();
		SetNCDSize(data, call_data->value / 100.0);
		// do update immediately so we can time how long it takes
		PWindow::HandleUpdates();
		sUpdateTime = double_time() - cur_time;
	}
}

void PSettingsWindow::NCDChangedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
	SetNCDSize(data, call_data->value / 100.0);
	sUpdateTime = 0;
}

// SetHitXYZ - turn on or off the hit XYZ display of the Hit Info window
void PSettingsWindow::SetHitXYZ(ImageData *data, int on)
{
	if (data->hit_xyz != on) {
		data->hit_xyz = on;
		sendMessage(data,kMessageHitXYZChanged);
	}
}

void PSettingsWindow::XYZProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
	SetHitXYZ(data, !data->hit_xyz);
}
void PSettingsWindow::SaveConfigProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
	data->save_config ^= 1;
}


