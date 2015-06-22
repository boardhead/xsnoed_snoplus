#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <stdlib.h>
#include <math.h>
#include "PFitterWindow.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "xsnoed.h"
#include "menu.h"

#define	ONLINE_FITTER_NAME			"Online"
#define MANUAL_FITTER_NAME			"Manual"
#define FI_WIDTH					284
#define FI_HEIGHT					315

PFitterWindow* PFitterWindow::sCurrent = NULL;

static void fitProc( Widget w, ImageData *data, caddr_t call_data )
{
#ifdef FITTR
	doFit(data, 1);
#endif
}

//---------------------------------------------------------------------------------
// PFitterWindow constructor
//
PFitterWindow::PFitterWindow(ImageData *data)
			  : PWindow(data)
{
	Widget	w, but;
	int		n;
	Arg		wargs[16];
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Fit Information"); ++n;
	XtSetArg(wargs[n], XmNx, 400); ++n;
	XtSetArg(wargs[n], XmNy, 400); ++n;
	XtSetArg(wargs[n], XmNwidth, FI_WIDTH); ++n;
	XtSetArg(wargs[n], XmNheight, FI_HEIGHT); ++n;
	SetShell(CreateShell("fiPop",data->toplevel,wargs,n));
	SetMainPane(w = XtCreateManagedWidget( "xsnoedForm", xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 10); ++n;
	XtCreateManagedWidget( "Fitter:", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 40); ++n;
	XtCreateManagedWidget( "Position:", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 70); ++n;
	XtCreateManagedWidget( "Radius:", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 100); ++n;
	XtCreateManagedWidget( "Time (ns):", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 130); ++n;
	XtCreateManagedWidget( "Direction:", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 160); ++n;
	XtCreateManagedWidget( "Cone Angle:", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 190); ++n;
	fi_qual.CreateLabel("Quality",  w, wargs, n);
	last_qual = kFitQuality;

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 220); ++n;
	XtCreateManagedWidget( "Fit Hits:", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 250); ++n;
	XtCreateManagedWidget( "NHITW:", xmLabelWidgetClass, w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 110); ++n;
	XtSetArg(wargs[n], XmNy, 10); ++n;
	fi_id.CreateLabel("fit_id", w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 103); ++n;
	XtSetArg(wargs[n], XmNy, 35); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNwidth, 171); ++n;
	fi_pos = XtCreateManagedWidget("fit_pos", xmTextWidgetClass, w, wargs, n );
	XtAddCallback(fi_pos,XmNactivateCallback,(XtCallbackProc)FitChangedProc,this);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 110); ++n;
	XtSetArg(wargs[n], XmNy, 70); ++n;
	fi_rad.CreateLabel("fit_rad", w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 103); ++n;
	XtSetArg(wargs[n], XmNy, 95); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNwidth, 171); ++n;
	fi_time= XtCreateManagedWidget("fit_time", xmTextWidgetClass, w, wargs, n );
	XtAddCallback(fi_time,XmNactivateCallback,(XtCallbackProc)FitChangedProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 103); ++n;
	XtSetArg(wargs[n], XmNy, 125); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNwidth, 171); ++n;
	fi_dir = XtCreateManagedWidget("fit_dir", xmTextWidgetClass, w, wargs, n );
	XtAddCallback(fi_dir,XmNactivateCallback,(XtCallbackProc)FitChangedProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 103); ++n;
	XtSetArg(wargs[n], XmNy, 155); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	XtSetArg(wargs[n], XmNwidth, 171); ++n;
	fi_angle = XtCreateManagedWidget("fit_angle", xmTextWidgetClass, w, wargs, n );
	XtAddCallback(fi_angle,XmNactivateCallback,(XtCallbackProc)FitChangedProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 110); ++n;
	XtSetArg(wargs[n], XmNy, 190); ++n;
	fi_chi2.CreateLabel("fit_chi2", w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 110); ++n;
	XtSetArg(wargs[n], XmNy, 220); ++n;
	fi_nhit.CreateLabel("fit_nhit", w, wargs, n );

	n = 0;
	XtSetArg(wargs[n], XmNx, 110); ++n;
	XtSetArg(wargs[n], XmNy, 250); ++n;
	fi_nhitw.CreateLabel("fit_nhitw", w, wargs, n );
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 210); ++n;
	XtSetArg(wargs[n], XmNy, 280); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 5); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 5); ++n;
	but = XtCreateManagedWidget( "Do Fit", xmPushButtonWidgetClass, w, wargs, n );
	XtAddCallback( but, XmNactivateCallback, (XtCallbackProc)fitProc, data );

	n = 0;
	XtSetArg(wargs[n], XmNx, 120); ++n;
	XtSetArg(wargs[n], XmNy, 280); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 5); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 5); ++n;
	but = XtCreateManagedWidget( "Next Fit", xmPushButtonWidgetClass, w, wargs, n );
	XtAddCallback( but, XmNactivateCallback, (XtCallbackProc)NextFitProc, data );

	n = 0;
	XtSetArg(wargs[n], XmNx, 10); ++n;
	XtSetArg(wargs[n], XmNy, 280); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 5); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 5); ++n;
	but = XtCreateManagedWidget( "Manual Fit", xmPushButtonWidgetClass, w, wargs, n );
	XtAddCallback( but, XmNactivateCallback, (XtCallbackProc)NewFitProc, data );
	
	data->mSpeaker->AddListener(this);
}

PFitterWindow::~PFitterWindow()
{
}

void PFitterWindow::Listen(int message, void *dataPt)
{
	int			i;
	ImageData	*data;
	
	switch (message) {
		// update calculated sigmas when any hit is discarded
		case kMessageHitDiscarded:
			data = GetData();
			for (i=0; i<data->nrcon; ++i) { 
				if (!strcmp(data->rcon[i].name, MANUAL_FITTER_NAME)) {
					CalcFitSigma(data, data->rcon + i);
				}
			}
			break;
		
		case kMessageNewEvent:
		case kMessageFitChanged:
		case kMessageWaterChanged:
			SetDirty();
			break;
	}
}


void PFitterWindow::CalcFitSigma(ImageData *data, RconEvent *rcon)
{
	int	num_hits = data->hits.num_nodes;
	
	if (num_hits) {
		HitInfo *	hi = data->hits.hit_info;
		Node *		node = data->hits.nodes;
		int			num = 0;
		double		sum2 = 0;
		
		int old_wCalibrated = calibrated_access_begin(data);
		
		for (int i=0; i<num_hits; ++i,++hi,++node) {
			// only consider normal tubes
			if ((hi->flags & (HIT_PMT_MASK | HIT_DISCARDED)) != HIT_NORMAL) continue;
			double old = hi->calibrated;
			if (setCalibratedTac(data,hi,i)) continue;
			// don't use invalid values (usually set to +-9999)
			if (hi->calibrated>-9998 && hi->calibrated<9998) {
				++num;
				// Calculate time differences from the straight line path
				double len = data->tube_radius * vectorLen(node->x3 - rcon->pos[0],
														   node->y3 - rcon->pos[1],
														   node->z3 - rcon->pos[2]);
				double diff = hi->calibrated - (rcon->time + len / data->light_speed);
				sum2 += diff * diff;
			}
			hi->calibrated = old;	// restore original calibrated value
		}
		rcon->fit_pmts = num;
		if (num) {
			rcon->chi_squared = sqrt(sum2 / num);
		} else {
			rcon->chi_squared = 0;
		}
		calibrated_access_end(data, old_wCalibrated);
	} else {
		rcon->chi_squared = 0;
		rcon->fit_pmts = 0;
	}
}


void PFitterWindow::UpdateSelf()
{
	ImageData	*data = GetData();
	char 		buff[256];
	int			this_qual;
	float 		len;
	RconEvent	*rcon;

#ifdef PRINT_DRAWS
	Printf("-updateFitInfo\n");
#endif
	if( data->nrcon ) {
		rcon = data->rcon + data->curcon;
		if (!strcmp(rcon->name,ONLINE_FITTER_NAME)) {
			this_qual = kFitChisq;
		} else if (!strcmp(rcon->name,MANUAL_FITTER_NAME)) {
			this_qual = kFitSigma;
		} else {
			this_qual = kFitQuality;
		}
		if (last_qual != this_qual) {
			last_qual = this_qual;
			switch (this_qual) {
				case kFitChisq:
					fi_qual.SetString("Chisq:");
					break;
				case kFitSigma:
					fi_qual.SetString("Sigma:");
					break;
				default:
					fi_qual.SetString("Quality:");
					break;
			}
		}
		sprintf(buff,"%s (%d)",rcon->name,data->curcon);
		fi_id.SetString(buff);
		sprintf(buff,"(%.3f,%.3f,%.3f)", rcon->dir[0], rcon->dir[1], rcon->dir[2]);
		setTextString(fi_dir,buff);
		sprintf(buff,"(%.1f,%.1f,%.1f)", rcon->pos[0] * data->tube_radius,
						rcon->pos[1] * data->tube_radius, rcon->pos[2] * data->tube_radius);
		setTextString(fi_pos,buff);
		sprintf(buff,"%.2f", rcon->cone_angle * 180 / PI);
		setTextString(fi_angle,buff);
		sprintf(buff,"%.2f", rcon->time);
		setTextString(fi_time,buff);
		len = vectorLen( rcon->pos[0], rcon->pos[1], rcon->pos[2] );
		sprintf(buff,"%.1f cm",len * data->tube_radius);
		fi_rad.SetString(buff);
		sprintf(buff,"%.3f", (rcon->pos[0] * rcon->dir[0] +
							  rcon->pos[1] * rcon->dir[1] +
							  rcon->pos[2] * rcon->dir[2]) / len);
		sprintf(buff,"%.3g",rcon->chi_squared);
		if (this_qual == kFitSigma) strcat(buff," ns");
		fi_chi2.SetString(buff);
		sprintf(buff, "%d", rcon->fit_pmts );
		fi_nhit.SetString(buff);
		if (!strcmp(rcon->name,ONLINE_FITTER_NAME)) {
			sprintf(buff, "%d", data->fit_nhitw[data->curcon] );
			fi_nhitw.SetString(buff );
		} else {
			strcpy(buff,"-");
			fi_nhitw.SetString(buff);
		}
	} else {
		strcpy(buff,"-");
		fi_id.SetString(buff);
		setTextString(fi_dir,buff);
		setTextString(fi_pos,buff);
		setTextString(fi_time,buff);
		setTextString(fi_angle,buff);
		fi_rad.SetString(buff);
		fi_chi2.SetString(buff);
		fi_nhit.SetString(buff);
		fi_nhitw.SetString(buff);
	}
}


void PFitterWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		/* set size of our event control window */
		Resize(FI_WIDTH,FI_HEIGHT);
	}
}

void PFitterWindow::NewFitProc(Widget w, ImageData *data, caddr_t call_data)
{
	RconEvent rcon;
	
	rcon.pos[0] = 0;
	rcon.pos[1] = 0;
	rcon.pos[2] = 0;
	rcon.dir[0] = 0;
	rcon.dir[1] = 0;
	rcon.dir[2] = 1;
	rcon.time = 0;
	rcon.chi_squared = 0;
	rcon.fit_pmts = 0;
	strcpy(rcon.name,MANUAL_FITTER_NAME);
	CalcFitSigma(data, &rcon);
	xsnoed_add_rcons(data, &rcon, 1, 1);
}

void PFitterWindow::NextFitProc(Widget w, ImageData *data, caddr_t call_data)
{
	if (data->nrcon > 1) {
		if (++data->curcon >= data->nrcon) data->curcon = 0;
		// must re-calculate time differences only if they are being displayed
		if (data->wDataType == IDM_DELTA_T) {
			calcCalibratedVals(data);
		}
		if (data->auto_vertex) {
			sendMessage(data, kMessageSetToVertex);
		}
		newTitle(data);
		sendMessage(data, kMessageFitChanged);
	}
}


static int GetVector3(char *pt, Vector3 vec)
{
	Vector3	tmp;
	char	buff[256];
	
	strncpy(buff,pt,256);
	buff[255] = '\0';
	pt = strtok(pt," ()[],\t\n\r");
	for (int i=0; ;) {
		if (!pt) break;
		tmp[i] = atof(pt);
		// are we done?
		if (++i >= 3) {
			// return vector
			memcpy(vec, tmp, sizeof(tmp));
			return(0);
		}
		pt = strtok(NULL," ()[],\t\n\r");
	}
	return(-1);
}
 
/* a fit parameter has changed.  Redisplay event and recalculate derived quantities */
void PFitterWindow::FitChangedProc(Widget w, PFitterWindow *fit_win, caddr_t call_data)
{
	int i;
	char *str;
	ImageData *data = fit_win->GetData();
	
	if( data->nrcon ) {
	
		RconEvent *rcon = data->rcon + data->curcon;
		
		str = XmTextGetString(fit_win->fi_pos);
		GetVector3(str, rcon->pos);
		XtFree(str);
		for (i=0; i<3; ++i) {
			rcon->pos[i] /= data->tube_radius;
		}
		
		str = XmTextGetString(fit_win->fi_dir);
		GetVector3(str, rcon->dir);
		XtFree(str);

		str = XmTextGetString(fit_win->fi_time);
		rcon->time = atof(str);
		XtFree(str);
		
		str = XmTextGetString(fit_win->fi_angle);
		rcon->cone_angle = atof(str) * PI / 180;
		XtFree(str);
		
		if (isRealFit(data)) {
			// change to manual fit type
			strcpy(data->rcon[data->curcon].name, MANUAL_FITTER_NAME);
			// calculate sigma for this fit
			CalcFitSigma(data, rcon);
		}

		// set reconstructed image nodes
		setRconNodes(data->rcon+data->curcon);
		// transform the rcon nodes to the current projection
		data->mMainWindow->GetImage()->Transform(data->rcon[data->curcon].nodes, data->rcon[data->curcon].num_nodes);
		if (data->wDataType == IDM_DELTA_T) {
			calcCalibratedVals(data);
		}
		sendMessage(data, kMessageFitChanged);
	}
}


