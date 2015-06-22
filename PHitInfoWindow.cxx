#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include "PHitInfoWindow.h"
#include "PImageWindow.h"
#include "PProjImage.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"
#include "calibrate.h"

// this really should be 12, but for some reason this doesn't
// work on all Motif's, so set it higher to force the extra
// labels to the bottom of the RowColumn. - as per NT suggestion - PH 05/08/00
const short		kExtraHitDataRowNumber	= 15;	// row number for extra hit data

enum QlxFlag {
	LABEL_QLX,
	LABEL_QLS,
	LABEL_QLL
};


//---------------------------------------------------------------------------------
// PHitInfoWindow constructor
//
PHitInfoWindow::PHitInfoWindow(ImageData *data)
			  : PWindow(data)
{
	Widget	rc1, rc2;
	int		n;
	Arg		wargs[16];
		
	mLastNum = -1;
	mCheckExtraNum = 1;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Hit Info"); ++n;
	XtSetArg(wargs[n], XmNx, 200); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNminWidth, 100); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	SetShell(CreateShell("hiPop",data->toplevel,wargs,n));
	SetMainPane(XtCreateManagedWidget("xsnoedForm", xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	rc1 = XtCreateManagedWidget("hiRC1",xmRowColumnWidgetClass,GetMainPane(),wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNleftWidget, rc1); ++n;
	XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment,	XmATTACH_FORM); ++n;
	rc2 = XtCreateManagedWidget("eiRC2",xmRowColumnWidgetClass,GetMainPane(),wargs,n);
	
	XtCreateManagedWidget("GTID:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Tac:",    xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Qhs:",    xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Qhl:",    xmLabelWidgetClass,rc1,NULL,0);
	hi_qlx_label.CreateLabel("Qlx:", rc1,NULL,0);
	if (data->wDataType==IDM_CMOS_RATES)
	    hi_hit_label.CreateLabel("Rate:",  rc1,NULL,0);
	else
	    hi_hit_label.CreateLabel("Hit Cnt:",  rc1,NULL,0);
	XtCreateManagedWidget("Crate:",  xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Card:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Channel:",xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Cell:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Type:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Panel:",  xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("PMT:",    xmLabelWidgetClass,rc1,NULL,0);
	
	// create XYZ labels invisible
	hi_xyz_labels[0].CreateLabel("X:",  rc1,NULL,0);
	hi_xyz_labels[1].CreateLabel("Y:",  rc1,NULL,0);
	hi_xyz_labels[2].CreateLabel("Z:",  rc1,NULL,0);
	
	hi_gt 		.CreateLabel("hiGT",     rc2,NULL,0);
	hi_tac  	.CreateLabel("hiTac",    rc2,NULL,0);
	hi_qhs  	.CreateLabel("hiQhs",    rc2,NULL,0);
	hi_qhl 		.CreateLabel("hiQhl",    rc2,NULL,0);
	hi_qlx 		.CreateLabel("hiQlx",    rc2,NULL,0);
	hi_nhit		.CreateLabel("hiNhit",   rc2,NULL,0);
	hi_crate 	.CreateLabel("hiCrate",  rc2,NULL,0);
	hi_card 	.CreateLabel("hiCard",   rc2,NULL,0);
	hi_channel	.CreateLabel("hiChannel",rc2,NULL,0);
	hi_cell 	.CreateLabel("hiCell",   rc2,NULL,0);
	hi_type 	.CreateLabel("hiType",   rc2,NULL,0);
	hi_panel 	.CreateLabel("hiPanel",  rc2,NULL,0);
	hi_pmt  	.CreateLabel("hiPMT",    rc2,NULL,0);
	
	// create XYZ labels invisible
	hi_xyz[0].CreateLabel("hiX",  	rc2,NULL,0);
	hi_xyz[1].CreateLabel("hiY",  	rc2,NULL,0);
	hi_xyz[2].CreateLabel("hiZ",  	rc2,NULL,0);
	
	// manage XYZ labels if hit_xyz is on
	if (!data->hit_xyz) ManageXYZ(0);
	
	// add extra labels
	mFirstExtraRow = kExtraHitDataRowNumber;
	SetExtraPanes(rc1, rc2);
	
	qlxFlag = LABEL_QLX;
	
	data->mSpeaker->AddListener(this);	// listen for cursor motion
	
	ClearEntries();
}

PHitInfoWindow::~PHitInfoWindow()
{
}

void PHitInfoWindow::ClearEntries()
{
	int		i;
	
	char	*str = "-";
	hi_gt.SetString(str);
	hi_tac.SetString(str);
	hi_qhs.SetString(str);
	hi_qhl.SetString(str);
	hi_qlx.SetString(str);
	hi_nhit.SetString(str);
	hi_crate.SetString(str);
	hi_card.SetString(str);
	hi_channel.SetString(str);
	hi_cell.SetString(str);
	hi_type.SetString(str);
	hi_panel.SetString(str);
	hi_pmt.SetString(str);
	for (i=0; i<3; ++i) {
		hi_xyz[i].SetString(str);
	}
	ClearExtraLabels();
}

/* UpdateSelf - set hit information window for PMT nearest the specified cursor position */
/* Note: hits must be tranformed to appropriate projection BEFORE calling this routine */
void PHitInfoWindow::UpdateSelf()
{
	int			i, labelNum;
	HitInfo		*hi;
	char		*str, buff[64];
	float		tr;
	ImageData	*data = mData;
	int			num = data->cursor_hit;	// current hit number near cursor
	static char *qlx_label_str[3] = { "Qlx", "Qls", "Qll" };
	
#ifdef PRINT_DRAWS
	Printf("-updateHitInfo\n");
#endif
	mLastNum = num;
	if (num == -1 ) {
		ClearEntries();
	} else {
		hi = data->hits.hit_info + num;
		sprintf(buff,data->hex_id ? "0x%.6lx" : "%ld",hi->gt);
		hi_gt.SetString(buff);
		if (data->wCalibrated != IDM_UNCALIBRATED) {
			tr = hi->calibrated;
			setCalibratedTac(data,hi,num);
			if (hi->calibrated != INVALID_CAL) {
				sprintf(buff,"%.1f ns",hi->calibrated);
			} else {
				strcpy(buff,"n/a");
			}
			hi_tac.SetString(buff);
			setCalibratedQhs(data,hi,num);
			if (hi->calibrated != INVALID_CAL) {
				sprintf(buff,"%.1f",hi->calibrated);
			} else {
				strcpy(buff,"n/a");
			}
			hi_qhs.SetString(buff);
			setCalibratedQhl(data,hi,num);
			if (hi->calibrated != INVALID_CAL) {
				sprintf(buff,"%.1f",hi->calibrated);
			} else {
				strcpy(buff,"n/a");
			}
			hi_qhl.SetString(buff);
			setCalibratedQlx(data,hi,num);
			if (hi->calibrated != INVALID_CAL) {
				sprintf(buff,"%.1f",hi->calibrated);
			} else {
				strcpy(buff,"n/a");
			}
			hi_qlx.SetString(buff);
#ifdef OPTICAL_CAL
			setCalibratedNHIT(data,hi,num);
			sprintf(buff,"%.3g",hi->calibrated);
			hi_nhit.SetString(buff);
#endif
			hi->calibrated = tr;
		} else {
			sprintf(buff,"%d",(int)hi->tac);
			hi_tac.SetString(buff);
			sprintf(buff,"%d",(int)hi->qhs);
			hi_qhs.SetString(buff);
			sprintf(buff,"%d",(int)hi->qhl);
			hi_qhl.SetString(buff);
			sprintf(buff,"%d",(int)hi->qlx);
			hi_qlx.SetString(buff);
#ifdef OPTICAL_CAL
			sprintf(buff,"%ld",(long)hi->nhit);
			hi_nhit.SetString(buff);
#endif
		}
#ifndef OPTICAL_CAL
		sprintf(buff,"%ld",(long)hi->nhit);
		hi_nhit.SetString(buff);
#endif
		sprintf(buff,"%d",(int)hi->crate);
		hi_crate.SetString(buff);
		sprintf(buff,"%d",(int)hi->card);
		hi_card.SetString(buff);
		sprintf(buff,"%d",(int)hi->channel);
		hi_channel.SetString(buff);
		sprintf(buff,"%d",(int)hi->cell);
		hi_cell.SetString(buff);
		if (hi->flags & HIT_NORMAL) {
			str = "Normal";
		} else if (hi->flags & HIT_FECD) {
			str = "FECD";
		} else if (hi->flags & HIT_LOW_GAIN) {
			str = "Low Gain";
		} else if (hi->flags & HIT_OWL) {
			str = "Owl";
		} else if (hi->flags & HIT_BUTTS) {
			str = "BUTTS";
		} else if (hi->flags & HIT_NECK) {
			str = "Neck";
		} else {
			str = "Unknown";
		}
		if (hi->flags & HIT_DISCARDED) {
			sprintf(buff,"(%s)",str);
			str = buff;
		}
		hi_type.SetString(str);
		if (data->sum) {
			labelNum = LABEL_QLX;
		} else if (hi->flags & HIT_LGISEL) {
			labelNum = LABEL_QLL;
		} else {
			labelNum = LABEL_QLS;
		}
		if (qlxFlag != labelNum) {
			qlxFlag = labelNum;
			hi_qlx_label.SetString(qlx_label_str[labelNum]);
		}
		Pmt *pmt = data->tube_coordinates + hi->index;
		if (pmt->panel) {
			sprintf(buff,"H%.3d-%.2d", pmt->panel, pmt->cell);
		} else {
			strcpy(buff,"-");
		}
		hi_panel.SetString(buff);
		str = int2bc(pmt->tube,'P');
		if (str[0] != 'P') str = "-";
		sprintf(buff,"%s (%d)",str,(int)hi->index);
		hi_pmt.SetString(buff);
		// make sure the correct number of extra hit data items are displayed
		if (mCheckExtraNum) {
			mCheckExtraNum = 0;
			if (SetExtraNum(data->extra_hit_num)) ResizeToFit();
		}
		// update the extra hit data
		for (i=0; i<GetExtraNum(); ++i) {
			sprintf(buff,data->extra_hit_fmt[i],*((float *)(data->extra_hit_data[i] + 1) + num));
			SetExtraLabel(i, buff);
		}
		float *xyz = &(data->hits.nodes[num].x3);
		float r = data->tube_coordinates[hi->index].r;
		if (r) {
			for (i=0; i<3; ++i) {
				sprintf(buff,"%.1f",xyz[i] * r);
				hi_xyz[i].SetString(buff);
			}
		} else {
			strcpy(buff,"-");
			for (i=0; i<3; ++i) {
				hi_xyz[i].SetString(buff);
			}
		}
	}
}

void PHitInfoWindow::SetRateMode()
{
	if (mData->wDataType==IDM_CMOS_RATES)
	    hi_hit_label.SetString("Rate:");
	else
	    hi_hit_label.SetString("Hit Cnt:");
}

void PHitInfoWindow::ManageXYZ(int manage)
{
	Widget		widgets[6];
	
	for (int i=0; i<3; ++i) {
		widgets[i] = hi_xyz[i].GetWidget();
		widgets[i+3] = hi_xyz_labels[i].GetWidget();
	}
	if (manage) {
	    // must manage right and left rowcol widgets separately
		XtManageChildren(widgets, 3);
		XtManageChildren(widgets+3, 3);
	} else {
		XtUnmanageChildren(widgets, 3);
		XtUnmanageChildren(widgets+3, 3);
	}
}

// ResizeToFit - resize shell height to fit the labels
void PHitInfoWindow::ResizeToFit()
{
	Widget		last_label;

	if (GetData()->hit_xyz) {
		last_label = hi_xyz[2].GetWidget();
	} else if (GetExtraNum()) {
		last_label = GetExtraWidget(GetExtraNum() - 1);
	} else {
		last_label = hi_pmt.GetWidget();
	}
	PWindow::ResizeToFit(last_label);
}

// SetHitXYZ - show or hide XYZ labels according to data->hit_xyz setting
void PHitInfoWindow::SetHitXYZ()
{
	if (GetData()->hit_xyz) {
		// show the XYZ labels
		ManageXYZ(1);
	} else {
		// hide the XYZ labels
		ManageXYZ(0);
	}
	ResizeToFit();
}

char * PHitInfoWindow::GetLabelString(int num)
{
	return(GetData()->extra_hit_data[num]->name);
}

void PHitInfoWindow::Listen(int message, void *message_data)
{
	switch (message) {
		case kMessageNewEvent:
			mCheckExtraNum = 1;	// must check extra hit data after each new event is displayed
			if (mLastNum >= 0) {
				// the event has changed, so the displayed hit data is now invalid
				// -> set the last displayed hit number to something invalid too
				//    to force the new data to be displayed
				mLastNum = 99999;
			}
			SetDirty();
			break;
		case kMessageCursorHit:
			// only dirty if this is a different hit
			if (mLastNum != mData->cursor_hit) {
				SetDirty();
			}
			break;
		case kMessageHitDiscarded:
			SetDirty();
			break;
		case kMessageDataModeChanged:
			SetRateMode();
			break;
		case kMessageHitXYZChanged:
			SetHitXYZ();
			break;
	}
}
