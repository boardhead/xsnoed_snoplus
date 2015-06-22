#include <X11/StringDefs.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include "POpticalWindow.h"
#include "PUtils.h"
#include "menu.h"
#include "xsnoed.h"


POpticalWindow::POpticalWindow(ImageData *data)
			  : PWindow(data)
{
	int		n;
	Arg		wargs[16];
	Widget	w;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Optical Constants"); ++n;
	XtSetArg(wargs[n], XmNx, 250); ++n;
	XtSetArg(wargs[n], XmNy, 250); ++n;
	XtSetArg(wargs[n], XmNminWidth, 356); ++n;
	XtSetArg(wargs[n], XmNminHeight, 240); ++n;
	SetShell(CreateShell("ocaPop",data->toplevel,wargs,n));
	SetMainPane(w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 18); ++n;
	XtCreateManagedWidget("Source Position:",xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 150); ++n;
	XtSetArg(wargs[n], XmNy, 13); ++n;
	XtSetArg(wargs[n], XmNwidth, 190); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	oca_pos_text = XtCreateManagedWidget("ocaPos",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(oca_pos_text,XmNactivateCallback,(XtCallbackProc)OcaPosProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 69); ++n;
	XtSetArg(wargs[n], XmNy, 52); ++n;
	XtCreateManagedWidget("Attenuation Coefficients (1/m)",xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 90); ++n;
	XtCreateManagedWidget("Acrylic:",xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 90); ++n;
	XtSetArg(wargs[n], XmNy, 75); ++n;
	XtSetArg(wargs[n], XmNwidth, 250); ++n;
	XtSetArg(wargs[n], XmNminimum, 0); ++n;
	XtSetArg(wargs[n], XmNmaximum, 500); ++n;
	XtSetArg(wargs[n], XmNdecimalPoints, 2); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
	oca_acrylic = XtCreateManagedWidget("Scale1",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(oca_acrylic,XmNvalueChangedCallback,(XtCallbackProc)OcaProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 130); ++n;
	XtCreateManagedWidget("D2O:",xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 90); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtSetArg(wargs[n], XmNwidth, 250); ++n;
	XtSetArg(wargs[n], XmNminimum, 0); ++n;
	XtSetArg(wargs[n], XmNmaximum, 1000); ++n;
	XtSetArg(wargs[n], XmNdecimalPoints, 4); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
	oca_d2o = XtCreateManagedWidget("Scale2",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(oca_d2o,XmNvalueChangedCallback,(XtCallbackProc)OcaProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 170); ++n;
	XtCreateManagedWidget("H2O:",xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 90); ++n;
	XtSetArg(wargs[n], XmNy, 155); ++n;
	XtSetArg(wargs[n], XmNwidth, 250); ++n;
	XtSetArg(wargs[n], XmNminimum, 0); ++n;
	XtSetArg(wargs[n], XmNmaximum, 1000); ++n;
	XtSetArg(wargs[n], XmNdecimalPoints, 4); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
	oca_h2o = XtCreateManagedWidget("Scale3",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(oca_h2o,XmNvalueChangedCallback,(XtCallbackProc)OcaProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 205); ++n;
	oca_intens_label = XtCreateManagedWidget("ocaIntens",xmLabelWidgetClass,w,wargs,n);
	
	UpdateOpticalConstants(UPDATE_OSA_ALL);
}


void POpticalWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		/* set size of our window */
		Resize(356,205);
	}
}


void POpticalWindow::UpdateOpticalConstants(ImageData *data, int update_flags)
{
	POpticalWindow *oca_win = (POpticalWindow *)data->mWindow[OPTICAL_WINDOW];
	if (oca_win) {
		oca_win->UpdateOpticalConstants(update_flags);
	}
}

void POpticalWindow::UpdateOpticalConstants(int update_flags)
{
	char		buff[256];
	Arg			wargs[1];
	int			val;
	ImageData	*data = GetData();
	
	if (update_flags & UPDATE_OSA_POSITION) {
		sprintf(buff,"%g %g %g",data->source_pos[0],data->source_pos[1],data->source_pos[2]);
		setTextString(oca_pos_text, buff);
	}
	if (update_flags & UPDATE_OSA_ATTEN) {
		val = (int)(data->u_acrylic * 10000 + 0.5);
		if (val>=0 && val<=500) {
			XtSetArg(wargs[0], XmNvalue, val);
			XtSetValues(oca_acrylic,wargs,1);
		}
		val = (int)(data->u_h2o * 1000000 + 0.5);
		if (val>=0 && val<=1000) {
			XtSetArg(wargs[0], XmNvalue, val);
			XtSetValues(oca_h2o,wargs,1);
		}
		val = (int)(data->u_d2o * 1000000 + 0.5);
		if (val>=0 && val<=1000) {
			XtSetArg(wargs[0], XmNvalue, val);
			XtSetValues(oca_d2o,wargs,1);
		}
	}
	if (update_flags & UPDATE_OSA_INTENSITY) {
		double frac;
		u_int32 tot = data->missed_window_count + data->total_sum_nhit;
		if (tot) frac = 100.0 * data->total_sum_nhit / tot;
		else frac = 0;
		sprintf(buff,"Source Intensity:     %.4g     Prompt:    %.1f %%",
				data->source_intensity, frac);
		setLabelString(oca_intens_label, buff);
	}
}

/* handle changes to source position */
void POpticalWindow::OcaPosProc(Widget w,POpticalWindow *oca_win, caddr_t call_data)
{
	ImageData *data = oca_win->GetData();
	
	char *pt = XmTextGetString(oca_win->oca_pos_text);
	sscanf(pt,"%f %f %f",data->source_pos,data->source_pos+1,data->source_pos+2);
	XtFree(pt);
	
	/* recalculate and redisplay images if necessary */
	if (data->wDataType==IDM_NHIT && data->wCalibrated!=IDM_UNCALIBRATED) {
		calcOCA(data, 1);
		calcCalibratedVals(data);
		sendMessage(data, kMessageHitsChanged);
	}
}

/* handle changes in attenuation controls */
void POpticalWindow::OcaProc(Widget w,POpticalWindow *oca_win, caddr_t call_data)
{
	Arg			wargs[1];
	int			value, len = 0;
	char		buff[512];
	ImageData *	data = oca_win->GetData();
	
	XtSetArg(wargs[0],XtNvalue,&value);
	XtGetValues(oca_win->oca_acrylic,wargs,1);
	len += sprintf(buff+len,"Acrylic: %g\n",0.0001 * value);
	
	XtGetValues(oca_win->oca_h2o,wargs,1);
	len += sprintf(buff+len,"H2O: %g\n",0.000001 * value);
	
	XtGetValues(oca_win->oca_d2o,wargs,1);
	len += sprintf(buff+len,"D2O: %g\n",0.000001 * value);
	
	/* store change in root window property */
	/* (we listen for a change in this property, and will update accordingly) */
	XStoreBuffer(data->display, buff, len, OCA_BUFFER_NUM);
}


