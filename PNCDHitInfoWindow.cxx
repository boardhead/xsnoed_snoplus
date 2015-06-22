#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include "PNCDHitInfoWindow.h"
#include "PImageWindow.h"
#include "PProjImage.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"
#include "calibrate.h"


//---------------------------------------------------------------------------------
// PNCDHitInfoWindow constructor
//
PNCDHitInfoWindow::PNCDHitInfoWindow(ImageData *data)
			  : PWindow(data)
{
	Widget	w;
	int		n;
	Arg		wargs[16];
		
	mLastNum = -1;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "NCD Hit"); ++n;
	XtSetArg(wargs[n], XmNx, 200); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNminWidth, 100); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	SetShell(CreateShell("ncdHitPop",data->toplevel,wargs,n));
	
	n = 0;
	XtSetArg(wargs[n], XmNpacking,		XmPACK_COLUMN); ++n;
	XtSetArg(wargs[n], XmNnumColumns,	2); ++n;
	w = XtCreateManagedWidget("xsnoedRowCol",xmRowColumnWidgetClass,GetShell(),wargs,n);
	SetMainPane(w);
	
	XtCreateManagedWidget("NCD:",        xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Mux Hits:",   xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Shaper Hits:",xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Scope Hits:", xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Shaper Val:", xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Mux Bus:",    xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Mux Box:",    xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Mux Chan:",   xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("HV Supply:",  xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Shaper Slot:",xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Shaper Addr:",xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Shaper Chan:",xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Scope Chan:", xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Preamp:",     xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("PDS Board:",  xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("PDS Chan:",   xmLabelWidgetClass,w,NULL,0);
	XtCreateManagedWidget("Counters:",   xmLabelWidgetClass,w,NULL,0);

	mXYLabels[0].CreateLabel("X:",  w,NULL,0,0);
	mXYLabels[1].CreateLabel("Y:",  w,NULL,0,0);
	
	mString     .CreateLabel("nhString",    w,NULL,0);
	mMuxHits  	.CreateLabel("nhMuxHits",   w,NULL,0);
	mShaperHits	.CreateLabel("nhShaperHits",w,NULL,0);
	mScopeHits  .CreateLabel("nhScopeHits", w,NULL,0);
	mShaperVal  .CreateLabel("nhShaperVal", w,NULL,0);
	mMuxBus  	.CreateLabel("nhMuxBus",    w,NULL,0);
	mMuxBox  	.CreateLabel("nhMuxBox",    w,NULL,0);
	mMuxChan 	.CreateLabel("nhMuxChan",   w,NULL,0);
	mHV 		.CreateLabel("nhHVSupply",  w,NULL,0);
	mShaperSlot	.CreateLabel("nhShaperSlot",w,NULL,0);
	mShaperAddr	.CreateLabel("nhShaperAddr",w,NULL,0);
	mShaperChan .CreateLabel("nhShaperChan",w,NULL,0);
	mScopeChan  .CreateLabel("nhScopeChan", w,NULL,0);
	mPreamp 	.CreateLabel("nhPreamp",    w,NULL,0);
	mPDSBoard	.CreateLabel("nhPDSBoard",  w,NULL,0);
	mPDSChan 	.CreateLabel("nhPDSChan",   w,NULL,0);
	mCounters 	.CreateLabel("nhCounters",  w,NULL,0);
	
	// create XY labels invisible
	mXY[0].CreateLabel("nhX",  	w,NULL,0,0);
	mXY[1].CreateLabel("nhY",  	w,NULL,0,0);
	
	// manage XYZ labels if hit_xyz is on
	if (data->hit_xyz) ManageXY(1);
	
	data->mSpeaker->AddListener(this);	// listen for cursor motion
	
	ClearEntries();
}

PNCDHitInfoWindow::~PNCDHitInfoWindow()
{
}

void PNCDHitInfoWindow::ClearEntries()
{
	int		i;
	
	char	*str = "-";
	mString.SetString(str);
	mMuxHits.SetString(str);
	mShaperHits.SetString(str);
	mScopeHits.SetString(str);
	mShaperVal.SetString(str);
	mMuxBus.SetString(str);
	mMuxBox.SetString(str);
	mMuxChan.SetString(str);
	mHV.SetString(str);
	mShaperSlot.SetString(str);
	mShaperAddr.SetString(str);
	mShaperChan.SetString(str);
	mScopeChan.SetString(str);
	mPreamp.SetString(str);
	mPDSBoard.SetString(str);
	mPDSChan.SetString(str);
	mCounters.SetString(str);
	for (i=0; i<2; ++i) {
		mXY[i].SetString(str);
	}
}

/* UpdateSelf - set hit information window for NCD nearest the specified cursor position */
void PNCDHitInfoWindow::UpdateSelf()
{
	char		buff[256];
	ImageData	*data = mData;
	int			num = data->cursor_ncd;	// current hit number near cursor
	
#ifdef PRINT_DRAWS
	Printf("-updateNCDHitInfo\n");
#endif
	mLastNum = num;
	if (num == -1 ) {
		ClearEntries();
	} else {
	    NCDMap *map = data->ncdMap + num;
	    NCDHit *hit = data->ncdHit + num;
	    sprintf(buff,"%s (%d)", map->label, (int)map->string_number);
	    mString.SetString(buff);
	    sprintf(buff,"%lu", (unsigned long)hit->mux_count);
	    mMuxHits.SetString(buff);
	    sprintf(buff,"%lu", (unsigned long)hit->shaper_count);
	    mShaperHits.SetString(buff);
	    sprintf(buff,"%lu", (unsigned long)hit->scope_count);
	    mScopeHits.SetString(buff);
	    sprintf(buff,"%lu", (unsigned long)hit->shaper_value);
	    mShaperVal.SetString(buff);
	    sprintf(buff,"%d", (int)map->mux_bus);
	    mMuxBus.SetString(buff);
	    sprintf(buff,"%d", (int)map->mux_boxnum);
	    mMuxBox.SetString(buff);
	    sprintf(buff,"%d", (int)map->mux_channel);
	    mMuxChan.SetString(buff);
	    sprintf(buff,"%d", (int)map->hv_supply);
	    mHV.SetString(buff);
	    sprintf(buff,"%d", (int)map->shaper_slot);
	    mShaperSlot.SetString(buff);
	    sprintf(buff,"0x%.4x", (int)map->shaper_addr);
	    mShaperAddr.SetString(buff);
	    sprintf(buff,"%d", (int)map->shaper_channel);
	    mShaperChan.SetString(buff);
	    sprintf(buff,"%d", (int)map->scope_channel);
	    mScopeChan.SetString(buff);
	    sprintf(buff,"%s", map->preamp);
	    mPreamp.SetString(buff);
	    sprintf(buff,"%d", (int)map->pds_board);
	    mPDSBoard.SetString(buff);
	    sprintf(buff,"%d", (int)map->pds_channel);
	    mPDSChan.SetString(buff);
	    buff[0] = '\0';
	    for (unsigned i=0; i<map->num_segments; ++i) {
	        if (i) strcat(buff,",");
	        strcat(buff, map->segment_name[i]);
	    }
	    mCounters.SetString(buff);
	    sprintf(buff,"%.2f", map->x);
	    mXY[0].SetString(buff);
	    sprintf(buff,"%.2f", map->y);
	    mXY[1].SetString(buff);
	}
}

void PNCDHitInfoWindow::ManageXY(int manage)
{
	Widget		widgets[4];
	
	for (int i=0; i<2; ++i) {
		widgets[i] = mXY[i].GetWidget();
		widgets[i+2] = mXYLabels[i].GetWidget();
	}
	if (manage) {
		XtManageChildren(widgets, 4);
	} else {
		XtUnmanageChildren(widgets, 4);
	}
}

// ResizeToFit - resize shell height to fit the labels
void PNCDHitInfoWindow::ResizeToFit()
{
	Widget		last_label;

	if (GetData()->hit_xyz) {
		last_label = mXY[1].GetWidget();
	} else {
		last_label = mCounters.GetWidget();
	}
	PWindow::ResizeToFit(last_label);
}

// SetHitXY - show or hide XY labels according to data->hit_xyz setting
void PNCDHitInfoWindow::SetHitXY()
{
	if (GetData()->hit_xyz) {
		// show the XY labels
		ManageXY(1);
	} else {
		// hide the XY labels
		ManageXY(0);
	}
	ResizeToFit();
}

void PNCDHitInfoWindow::Listen(int message, void *message_data)
{
	switch (message) {
		case kMessageNewEvent:
			if (mLastNum >= 0) {
				// the event has changed, so the displayed hit data is now invalid
				// -> set the last displayed hit number to something invalid too
				//    to force the new data to be displayed
				mLastNum = 99999;
			}
			SetDirty();
			break;
		case kMessageCursorNcd:
			// only dirty if this is a different hit
			if (mLastNum != mData->cursor_ncd) {
				SetDirty();
			}
			break;
		case kMessageHitDiscarded:
			SetDirty();
			break;
		case kMessageHitXYZChanged:
			SetHitXY();
			break;
	}
}
