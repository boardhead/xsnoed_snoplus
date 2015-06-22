#include <stdlib.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>

#include "PMenu.h"
//#include "PWindow.h"
#include <Xm/FileSB.h>

#include "PScopeImage.h"
#include "PResourceManager.h"
#include "PScale.h"
#include "PImageWindow.h"
//#include "ImageData.h"
#include "PUtils.h"

#define HIST_MARGIN_BOTTOM			(25 * GetScaling())
#define HIST_MARGIN_LEFT			(48 * GetScaling())
#define HIST_MARGIN_TOP				(14 * GetScaling())
#define HIST_MARGIN_RIGHT			(24 * GetScaling())
#define HIST_LABEL_Y				(-10 * GetScaling())

#define WIN_BORDER				50 /*(mWidth - 500)/2*/
#define GRID_COLS				10
#define GRID_ROWS				8

/*
static MenuStruct file_menu[] = {
	{ "Open File...",  0,   XK_O,	IDM_SCOPE_OPEN,		NULL, 0, 0},
	{ "Close File...", 0,   XK_C,	IDM_SCOPE_CLOSE,		NULL, 0, 0},
	{ "Close Display", 0,   XK_l,	IDM_SCOPE_EXIT,		NULL, 0, 0}
};

	
static MenuStruct settings_menu[] = {
	{ "Change Settings...",			0, XK_h,	IDM_SCOPE_CHANGE, NULL, 0, 0},
	{ "Load Scope Settings from File...",	0, XK_L,	IDM_SCOPE_LOAD,	  NULL, 0, 0},
	{ "Save Scope Settings to File...",	0, XK_S,	IDM_SCOPE_SAVE,	  NULL, 0, 0}
};


static MenuStruct main_menu[] = {
	{ "File",     0, 0, 0, file_menu,      XtNumber(file_menu),      0},
	{ "Settings", 0, 0, 0, settings_menu,  XtNumber(settings_menu),  0}
};
*/
	
//---------------------------------------------------------------------------------------
// PScopeImage constructor
//
PScopeImage::PScopeImage(PImageWindow *owner, Widget canvas, int createCanvas)
		  : PImageCanvas(owner,canvas,PointerMotionMask|ButtonPressMask|ButtonReleaseMask)
{
	mXScale			= NULL;
	mYScale			= NULL;
	mIsLog			= 0;
	mHistogram		= NULL;
	mOverlay		= NULL;
	mOverlayScale	= 1.0;
	mLabel			= NULL;
	mNumBins		= 0;
	mNumCols		= 0;
	mHistCols		= NULL;
	mYMax			= 10;
	mXMin			= 0;
	mXMax			= 10;
	mXScaleFlag		= INTEGER_SCALE;
	mStyle			= kScopeStyleSteps;
	mGrabFlag		= 0;
	mXMinMin		= 0;
	mXMaxMax		= 10;
	mXMinRng		= 1;
	
	mData = owner->GetData();
	mRunStop = mData->scopeData.acquireState;
	// See PScopeImage::DrawWaveform()
	//ch_decrement 		= mData->scopeData.numberOfActiveChannels;
	//init_y_pos		= 0;
	//prev_wfsize		= -1;
	//firstTimeThrough	= true;
	
	AllowLabel(FALSE);      // no event label on scope display
	
	if (!canvas && createCanvas) {
// add this back again when we want to do something with our menus
//		owner->CreateMenu(NULL,main_menu,XtNumber(main_menu),this);
		CreateCanvas("scopeCanvas");		
	}
}

PScopeImage::~PScopeImage()
{
	delete mXScale;
	delete mYScale;
	
	// delete histogram, label string, and colours
	delete [] mHistogram;
	delete [] mOverlay;
	delete [] mLabel;
	delete [] mHistCols;
	
	ImageData *data = mOwner->GetData();
	
	if (data->mScaleScope == this) {
		data->mScaleScope = NULL;
		if (data->mWindow[SCALE_WINDOW]) {
			delete data->mWindow[SCALE_WINDOW];
		}
	}
}


// CreateData - create histogram data (can be used to delete data if numbins=0)
void PScopeImage::CreateData(int numbins)
{
	if (!mHistogram || numbins!=mNumBins) {
		delete [] mHistogram;
		mHistogram = NULL;
	
		if (numbins > 0) {
			mHistogram = new long[numbins];
			mNumBins = numbins;
		}
	}
}

void PScopeImage::SetIntegerXScale(int is_int)
{
	if (is_int) {
		mXScaleFlag |= INTEGER_SCALE;
	} else {
		mXScaleFlag &= ~INTEGER_SCALE;
	}
	if (mXScale) {
		mXScale->SetInteger(is_int);
	}
}

void PScopeImage::CreateScaleWindow()
{
	int			n;
	Position	xpos, ypos;
	Arg			wargs[20];
	char		buff[256], *pt;
	Widget		window, but, w;
	ImageData	*data = mOwner->GetData();
	
	n = 0;
	XtSetArg(wargs[n], XmNx, &xpos); ++n;
	XtSetArg(wargs[n], XmNy, &ypos); ++n;
	if (data->mWindow[SCALE_WINDOW]) {
		if (data->mScaleScope == this) {
			data->mWindow[SCALE_WINDOW]->Raise();	// raise to top
			return;	// already opened scale for this hist
		}
		XtGetValues(data->mWindow[SCALE_WINDOW]->GetShell(), wargs, n);
		// destroy old scale window
		delete data->mWindow[SCALE_WINDOW];
	} else {
  		// center the scale popup on the histogram
		XtGetValues(mOwner->GetShell(), wargs, n);
		xpos += 50;
		ypos += 40;
	}
	data->mScaleScope = this;

	pt = mOwner->GetTitle();
  	sprintf(buff,"%s Scales",pt);

  	n = 0;
	XtSetArg(wargs[n], XmNtitle, buff); ++n;
	XtSetArg(wargs[n], XmNx, xpos); ++n;
	XtSetArg(wargs[n], XmNy, ypos); ++n;
	XtSetArg(wargs[n], XmNminWidth, 271); ++n;
	XtSetArg(wargs[n], XmNminHeight, 155); ++n;
	window = PWindow::CreateShell("scalePop",data->toplevel,wargs,n);
	w = XtCreateManagedWidget("xsnoedForm",
                                  xmFormWidgetClass,window,NULL,0);
	data->mWindow[SCALE_WINDOW] = new PWindow(data,window,w);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 18); ++n;
	XtCreateManagedWidget("X Scale Minimum:", 
                              xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 155); ++n;
	XtSetArg(wargs[n], XmNy, 13); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	sp_min = XtCreateManagedWidget("ScaleMin",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(sp_min,XmNactivateCallback,
                      (XtCallbackProc)ScaleOKProc,this);
	sprintf(buff,"%g",mXMin);
	setTextString(sp_min,buff);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 53); ++n;
	XtCreateManagedWidget("X Scale Maximum:", 
                              xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 155); ++n;
	XtSetArg(wargs[n], XmNy, 48); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	sp_max = XtCreateManagedWidget("ScaleMax",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(sp_max,XmNactivateCallback,
                      (XtCallbackProc)ScaleOKProc,this);
	sprintf(buff,"%g",mXMax);
	setTextString(sp_max,buff);

	n = 0;
	XtSetArg(wargs[n], XmNx, 184); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 18); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 18); ++n;
	but = XtCreateManagedWidget("OK",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, 
                           (XtCallbackProc)ScaleOKProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 102); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 7); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 7); ++n;
	but = XtCreateManagedWidget("Apply",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, 
                           (XtCallbackProc)ApplyProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 5); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 5); ++n;
	but = XtCreateManagedWidget("Cancel",
                                    xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, 
                           (XtCallbackProc)CancelProc, window);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 45); ++n;
	XtSetArg(wargs[n], XmNy, 83); ++n;
	XtSetArg(wargs[n], XmNset, mIsLog); ++n;
	sp_log = XtCreateManagedWidget("Logarithmic Y Scale",
                      xmToggleButtonWidgetClass,w,wargs,n);
//	XtAddCallback(sp_log,XmNvalueChangedCallback,
//                          (XtCallbackProc)ScaleOKProc, this);

	data->mWindow[SCALE_WINDOW]->Show();		// show the window
}

void PScopeImage::SetScaleLimits(float min, float max, float min_rng)
{
	mXMinMin = min;
	mXMaxMax = max;
	mXMinRng = min_rng;
}

void PScopeImage::SetLabel(char *str)
{
	if (mLabel) delete [] mLabel;
	int len = strlen(str);
	mLabel = new char[len+1];
	if (mLabel) {
		strcpy(mLabel, str);
	}
}

void PScopeImage::UpdateScaleInfo()
{
	char		buff[128];
	Arg			wargs[1];
	ImageData	*data = mOwner->GetData();
	
	if (!data->mWindow[SCALE_WINDOW]) return;
	
	if (data->mScaleScope == this) {
	
		sprintf(buff,"%g",mXMin);
		setTextString(sp_min,buff);
	
		sprintf(buff,"%g",mXMax);
		setTextString(sp_max,buff);
	
		XtSetArg(wargs[0], XmNset, mIsLog);
		XtSetValues(sp_log,wargs,1);
	}
}

void PScopeImage::SetLog(int on)
{
	if (mIsLog != on) {
		mIsLog = on;
		// must re-create y scale if log changes
		delete mYScale;
		mYScale = NULL;
	}
}

void PScopeImage::SetCursorForPos(int x, int y)
{
	if (IsInLabel(x, y)) {
		PImageCanvas::SetCursorForPos(x,y);
	} else if (y >= mHeight - HIST_MARGIN_BOTTOM) {
		SetCursor(CURSOR_MOVE_H);
	} else if (x <= HIST_MARGIN_LEFT) {
		SetCursor(CURSOR_MOVE_V);
	} else {
		SetCursor(CURSOR_MOVE_4);
	}
}


void PScopeImage::Listen(int message, void *dataPt)
{   
	switch (message) {
		case kMessageNewScopeData:
		case kMessageColoursChanged:
			mRunStop = mData->scopeData.acquireState;
			SetDirty();
//			printf(" PScopeImage:: Updating Display...\n");
			break;
		default:
			PImageCanvas::Listen(message, dataPt);
			break;
	}
}

void PScopeImage::DrawSelf()
{
	int i;
#ifdef PRINT_DRAWS
	Printf("drawScope\n");
#endif
	
	MakeHistogram();
	//SetHistogramLabel();

	PImageCanvas::DrawSelf();
	
	// start drawing
	
	// Grid Stuff
	SetForeground(GRID_COL);
	DrawGrid();
	
	// Scope Text (Bottom Info)
	SetForeground(TEXT_COL);
	SetFont(PResourceManager::sResource.label_font);
	
	int colorArray[4] = 	{ SCOPE0_COL, // green
				SCOPE1_COL,    // light blue
				SCALE_COL2,     // yellow
				SCALE_COL3      // orange
				} ; // max 4 waveforms
	
	int nActvCh = mData->scopeData.numberOfActiveChannels;
	int ich; // ich = channel index = channel -1
	
	if (nActvCh > SCOPEMAXCHANNELS) {
	    printf("Too many scope channels (%d).  Ignoring extras.\n", nActvCh);
	    nActvCh = SCOPEMAXCHANNELS;
	}
	// Drawing Top Left Info
	DrawRunStop();
	DrawTriggerMode();
	DrawTriggerType();
	DrawTriggerCoupling();
	DrawTriggerHoldoff();
	
	// Drawing Bottom left Info
	// NOTE: This stuff is hardcoded for 2 waveforms at the moment!
	SetForeground(colorArray[0]); // Green
	DrawCh1();
	DrawVpd(1);
	SetForeground(colorArray[1]); // light blue
	DrawCh2();
	DrawVpd(2);
	SetForeground(TEXT_COL); //reset color to gray
	DrawTpd();
	DrawTriggerSource();
	DrawTriggerSlope();
	DrawTriggerLevel();	
	DrawEventNumber();
	
	// Drawing Trigger
	SetForeground(colorArray[2]);
	SetLineWidth(2);
	DrawTrigger();
	
	// This will work for 1 to 4 waveforms but the above code will not!
	for (i=0; i < nActvCh; i++) {
	  //if(1){
          //  printf("xsnoed::DrawSelf() i=%d activeChannels[i]=%d\n",i,mData->scopeData.activeChannels[i]);
          //}
           ich = mData->scopeData.activeChannels[i]-1;
	   SetForeground(colorArray[i]);
	   DrawWaveform(ich, i);
	}
	
	//reset color
	SetForeground(TEXT_COL);
	//reset line width
	SetLineWidth(THIN_LINE_WIDTH);
	
		
	// Draw histogram label
	//if (mLabel) {
	//	SetForeground(TEXT_COL);
	//	SetFont(PResourceManager::sResource.hist_font);
	//	DrawString(x2,y1+HIST_LABEL_Y,mLabel,kTextAlignTopRight);
	//}
}

void PScopeImage::Resize()
{
	if (mXScale) {
		delete mXScale;
		mXScale = NULL;
	}
	if (mYScale) {
		delete mYScale;
		mYScale = NULL;
	}
	SetDirty();
}

// this must only be called when the scale window is open
void PScopeImage::ReadScaleValues()
{
	Arg			wargs[1];
	Boolean		isLog = FALSE;
	float		xmin, xmax;
	char		*str;

	xmin = atof(str = XmTextGetString(sp_min));
	XtFree(str);
	xmax = atof(str = XmTextGetString(sp_max));
	XtFree(str);
	
	XtSetArg(wargs[0], XmNset, &isLog);
	XtGetValues(sp_log, wargs, 1);
	
	SetLog(isLog);

	// check scale range
	SetScaleLimits();
	mGrabFlag |= GRAB_X_ACTIVE | GRAB_X;
	DoGrab(xmin, xmax);
	mGrabFlag &= ~GRABS_ACTIVE;
	SetDirty();
	DoneGrab();
	UpdateScaleInfo();
}

void PScopeImage::CheckScaleRange()
{
	if (mXMin < mXMinMin) {
		mXMin = mXMinMin;
	}
	if (mXMax > mXMaxMax) {
		mXMax = mXMaxMax;
	}
	if (mXMax - mXMin < mXMinRng) {
		mXMax = mXMin + mXMinRng;
		if (mXMax > mXMaxMax) {
			mXMax = mXMaxMax;
			mXMin = mXMax - mXMinRng;
		}
	}
}


void PScopeImage::ScaleOKProc(Widget w, PScopeImage *hist, caddr_t call_data)
{
	ImageData *data = hist->mOwner->GetData();
	
	hist->ReadScaleValues();
	if (data->mWindow[SCALE_WINDOW]) {
		delete data->mWindow[SCALE_WINDOW];
	}
}

void PScopeImage::ApplyProc(Widget w, PScopeImage *hist, caddr_t call_data)
{
	hist->ReadScaleValues();
}

void PScopeImage::CancelProc(Widget w, Widget aShell, caddr_t call_data)
{
	XtDestroyWidget(aShell);
}



// ++++++++++++++++++++++++++++++++++++++++++++
// Scope Specific Drawing Methods
// ++++++++++++++++++++++++++++++++++++++++++++

void PScopeImage::DrawGrid()
{
  // Draw the scope grid

  int i;
  float x[2],y[2];
  
  float fXmin, fXmax;
  int fXsteps;

  float fYmin, fYmax;
  int fYsteps;
  
  fXmin   = WIN_BORDER; 
  // how many pixels from the side (initially defined as 50)
  fXmax   = mWidth - fXmin;
  fXsteps = GRID_COLS; // initially set at 10 (see top of page)
  
  fYmin   = fXmin;
  fYmax   = mHeight - fYmin;
  fYsteps = GRID_ROWS; // initially set at 8 (see top of page)
    
  float fXwid = (fXmax - fXmin)/(float)fXsteps;
  float fYwid = (fYmax - fYmin)/(float)fYsteps;
  
  // draw horizontal grid lines
  for(i=0;i<=fYsteps;i++){
    x[0] = fXmin;
    y[0] = fYmax - (fYwid *(float)i);
    x[1] = fXmax;   
    y[1] = y[0];
    if(i==0 || i==fYsteps || i == (fYsteps/2) ){
      SetLineWidth(2);
    } else {
      SetLineWidth(THIN_LINE_WIDTH);
    }
    DrawLine( (int)x[0], (int)y[0], (int)x[1], (int)y[1]);
    //printf("line: %d --> x[0] = %lf, y[0] = %lf, x[1] = %lf, y[1] = %lf\n", 
    //       i,x[0],y[0],x[1],y[1]);    
   }
   
  // draw vertical grid lines
  for(i=0;i<=fXsteps;i++){
    x[0] = fYmin + (fXwid *(float)i);
    y[0] = fYmin;   
    x[1] = x[0];
    y[1] = fYmax;   
    
    //l->SetLineColor(fDisplayColor);
    if(i==0 || i==fXsteps || i==(fXsteps/2) ){
      SetLineWidth(3);//THICK_LINE_WIDTH);      
    } else {
      SetLineWidth(THIN_LINE_WIDTH);//THICK_LINE_WIDTH);
    }
    DrawLine( (int)x[0], (int)y[0], (int)x[1], (int)y[1]);
    //printf("line: %d --> x[0] = %lf, y[0] = %lf, x[1] = %lf, y[1] = %lf\n",
    //       i,x[0],y[0],x[1],y[1]);
  }
  // reset line width
  SetLineWidth(THIN_LINE_WIDTH);
}

// --------------------------------
// Top Left Information
// --------------------------------
void PScopeImage::DrawRunStop()
{
   if(mRunStop) {
      DrawString(50,30,"RUN",kTextAlignTopLeft);
   }
   else {
      DrawString(50,30,"STOP",kTextAlignTopLeft);
   }
}

void PScopeImage::DrawTriggerMode()
{
   if(mRunStop) {
      char trigMode[32];
      sprintf(trigMode,"%s", mData->scopeData.triggerMode);   
      DrawString(100,30,trigMode,kTextAlignTopLeft);
   }   
}

void PScopeImage::DrawTriggerType()
{
   if(mRunStop) {
      char trigType[32];
      sprintf(trigType,"%s", mData->scopeData.triggerType);
      DrawString(170,30,trigType,kTextAlignTopLeft);
   }
}

void PScopeImage::DrawTriggerCoupling()
{
   if(mRunStop) {
      char trigCoupling[32];
      sprintf(trigCoupling,"%s", mData->scopeData.triggerCoupling);
      DrawString(230,30,trigCoupling,kTextAlignTopLeft);
   }
}

void PScopeImage::DrawTriggerHoldoff()
{
   if(mRunStop) {
      char str[132];
      double trigHoldoff = mData->scopeData.triggerHoldoff; 
      Time2String(trigHoldoff,str);
      DrawString(270,30,str,kTextAlignTopLeft);	
   }   
}

// --------------------------------
// Bottom Left Information
// --------------------------------
void PScopeImage::DrawCh1()
{
  // draw Ch1 on display
  int xpos = 50; // must be same as fXmin in DrawGrid()
  DrawString(xpos,mHeight-30,"Ch1:",kTextAlignBottomLeft);
}

void PScopeImage::DrawCh2()
{
  // draw Ch2 on display
  int xpos = 160;
  DrawString(xpos,mHeight-30,"Ch2:",kTextAlignBottomLeft);
}

void PScopeImage::DrawVpd(int ch)
{
   char str[132];
   int xpos;// = 80;
   if(ch == 1) { xpos = 80; }
   else { xpos = 190; } // must be channel 2
   int ypos = mHeight-30;
   double chScale = mData->scopeData.channelScale[ch-1]; 
   if(mRunStop) { // if we are getting data
      Volts2String(chScale, str);
      DrawString(xpos,ypos,str,kTextAlignBottomLeft);	
   }
   else {    
      DrawString(xpos,ypos,"--",kTextAlignBottomLeft);
   }   
}

void PScopeImage::DrawTpd()
{
  // draw time/div
  char str[132];
  int xpos = 250;
  double tpd = mData->scopeData.timeScale;
  if(mRunStop) { // if we are getting data
     Time2String(tpd, str);
     DrawString(xpos+10,mHeight-30,str,kTextAlignBottomLeft);
  }
   else {
      DrawString(xpos,mHeight-30,"Time/Div: --",kTextAlignBottomLeft);
      DrawString(xpos+55,mHeight-30,"--",kTextAlignBottomLeft);
   }
  
}

void PScopeImage::DrawTriggerSource()
{
   if(mRunStop) {
      char trigSource[32];
      sprintf(trigSource,"%s",mData->scopeData.triggerSource);
      DrawString(310,mHeight-30,trigSource,kTextAlignBottomLeft);	
   }
}

void PScopeImage::DrawTriggerSlope()
{
   if(mRunStop) {
      char slope[32];
      sprintf(slope,"%s",mData->scopeData.triggerSlopeString);
      DrawString(350,mHeight-30,slope,kTextAlignBottomLeft);   
   }   
}

void PScopeImage::DrawTriggerLevel()
{
   if(mRunStop) {
      char str[132];
      int xpos = 400; // bottom right
      double trigLevel = mData->scopeData.triggerLevel; 
      Volts2String(trigLevel,str);
      DrawString(xpos,mHeight-30,str,kTextAlignBottomLeft);	
   }   
}


void PScopeImage::DrawEventNumber()
{
  // draw the event number
  if(mRunStop) {
     char str[132];  
     int xpos = 50;
     int eN = mData->scopeData.eventNumber;
     sprintf(str,"Event: %d",eN);
     DrawString(xpos,mHeight-10,str,kTextAlignBottomLeft);
  }
}


void PScopeImage::DrawTrigger()
{
   if(mRunStop) {
      int ch;
      char *source = mData->scopeData.triggerSource;

      if(!strcmp(source,"CH1")){
        ch = 1;
      } else if(!strcmp(source,"CH2")){
        ch = 2;
      } else if(!strcmp(source,"CH3")){
        ch = 3;
      } else if(!strcmp(source,"CH4")){
        ch = 4;
      } else {
        ch = 1;
      }
  
      double vpd = mData->scopeData.channelScale[ch-1];
      double pos = mData->scopeData.channelPosition[ch-1];
      double level = mData->scopeData.triggerLevel;

      //printf("  vpd   = %f\n",vpd);
      //printf("  pos   = %f\n",pos);
      //printf("  level = %f\n",level);
      
      double ydiv = pos + (level/vpd);
      
      //printf("  ydiv = %f\n\n",ydiv);
      
      int y = DisplayDiv2Y(ydiv);

      int ax[2];
      int ay[2];

      // get the division sizes
      int x0 = WIN_BORDER;
      int x1 = x0 + 20;

      ax[0] = x0; ay[0] = y; 
      ax[1] = x1; ay[1] = y;
      
      DrawLine( ax[0], ay[0], ax[1], ay[1]);
      
      int x_pos_T = (x0+x1)/2;
      DrawString( x_pos_T, y-15, "T", kTextAlignTopLeft);      
   }
}

int PScopeImage::DisplayDiv2Y(double ydiv)
{
  // Convert from a vertical position in divisions
  // to the y position of the display.
  double wid = (mHeight - (WIN_BORDER*2.0))/GRID_ROWS; //height of 1 row in pixels

  double half_grid = mHeight/2.0;
  double y = half_grid - (ydiv * wid);
  
  int y_ret = (int)y;
  return y_ret;
}



void PScopeImage::DrawWaveform(int ch, int ind)
{
  //if(1){
  //   printf(" xsnoed::DrawWaveform(%d,%d)\n",ch,ind);
  //}
   if(mRunStop) {
      int i, nsize;
      //SetLineWidth(2);
      
      if (ch > SCOPEMAXCHANNELS) {
        printf("Error: Bad scope channel (%d)\n", ch);
        return;
      }
        
      nsize = mData->scopeData.size[ch];
      //printf(" size is equal to %d\n", nsize);
      if(nsize < 1) return;
      if (nsize > WAVEFORMSIZE) {
        printf("Error: Scope waveform too big (%d bytes)\n", nsize);
        return;
      }
      
      // ============== STUFFING VALUES IN X & Y ==============
      float xmax   = (float) (mWidth - WIN_BORDER);
      float xmin   = (float) WIN_BORDER;
      
      float ymax   = (float) (mHeight - WIN_BORDER);
      float ymin   = (float) WIN_BORDER;
      float yscope;
	 
      for(i=0; i<nsize; i++) {
	 
	//mX[i] = ( (xmax-xmin)/500.0 )*(float)i + xmin;
	 mX[i] = ( (xmax-xmin)/(float)nsize )*(float)i + xmin;
         
	 yscope = (float) mData->scopeData.wave[ch][i];
	 mY[i] = ((ymax - ymin)/-200.0)*yscope + ((ymin+ymax)/2.0);
         if(mY[i] > ymax){
	   mY[i] = ymax;
         } else if(mY[i] < ymin){
           mY[i] = ymin;
         }
	 //printf( "CH = %d ===== y[%d] = %lf\n", ch, i, mY[i]);
         
      }
	 
      // ============== DRAWING WAVEFORM ==============
      int nseg = nsize-1;
      XSegment *seg = new XSegment[nseg]; // there are "npoints-1" segments
      for (i=0; i<nseg; i++) { // 0 to 498 is 499 segments!
	 seg[i].x1 = (int)mX[i];
	 //printf(" seg[%d].x1 = %d\n", i, seg[i].x1); 
	 seg[i].y1 = (int)mY[i];
	 seg[i].x2 = (int)mX[i+1];
	 seg[i].y2 = (int)mY[i+1];
      }
      
      DrawSegments(seg,nseg); 
      delete seg;     
   
   } //end if(mRunStop)
   //if(1){
   //  printf(" xsnoed::DrawWaveform() returning\n");
   //}
}


void PScopeImage::ComputeLogs(int npoints)
{
//*-*-*-*-*-*-*-*-*-*-*-*Convert WC from Log scales*-*-*-*-*-*-*-*-*-*-*-*
//*-*                    ==========================
//
//   Take the LOG10 of gxwork and gywork according to the value of Options
//   and put it in gxworkl and gyworkl.
//
//  npoints : Number of points in gxwork and in gywork.
//
  int i; 
  for (i=0;i<npoints;i++) {
     //if (gPad->GetLogx()) {
        if (mX[i] > 0) {
	   mX[i] = log10(mX[i]);
	}   
        //else {
	//   gxworkl[i] = gPad->GetX1();
	//}   
     //}
     //if (!opt && gPad->GetLogy()) {
        if (mY[i] > 0) {
	   mY[i] = log10(mY[i]);
	}   
        //else               gyworkl[i] = gPad->GetY1();
     //}
  }
}


void PScopeImage::Volts2String(double vpd, char* str)
{
  // return a string with the V/div and units  
  double vpd2;
  
  if( vpd < 1) { // mV
    vpd2 = vpd * 1000.0;
    sprintf(str,"%0.0f mV",vpd2);
  } else if (vpd < 1000) { // V
    sprintf(str,"%0.0f V",vpd);
  }  else {
    strcpy(str,"<bad volts>"); 
  }
}





void PScopeImage::Time2String(double t,char *str)
{
  // return a string with the V/div and units

  double t2;

  if( t < 1.0e-6) { // ns
    t2 = t * 1.0e9;
    sprintf(str,"%0.0f ns",t2);
  } else if( t < 1.0e-3) { // us
    t2 = t * 1.0e6;
    if( t > 2.0e-6 && t < 3.0e-6){
      sprintf(str,"%0.1f us",t2);
    } else {
      sprintf(str,"%0.0f us",t2);
    }
  } else if( t < 1.0) { // ms
    t2 = t * 1.0e3;
    if( t > 2.0e-3 && t < 3.0e-3){
      sprintf(str,"%0.1f ms",t2);
    } else {
      sprintf(str,"%0.0f ms",t2);
    }
  } else { // V
    sprintf(str,"%0.0f s",t);
    if( t > 2.0 && t < 3.0){
      sprintf(str,"%0.1f s",t);
    } else {
      sprintf(str,"%0.0f s",t);
    }
  }  
}


//--------------------------------------------------------------------------
// Menu callbacks
//
void PScopeImage::DoMenuCommand(int anID)
{
   switch (anID) {
      case IDM_SCOPE_OPEN:
        printf("	'OPEN FILE' BUTTON HAS BEEN PRESSED\n");
        OpenFileBox();
        break;
      case IDM_SCOPE_CLOSE:
	    printf("	'CLOSE FILE' BUTTON HAS BEEN PRESSED\n");
	
	    break;	
      case IDM_SCOPE_EXIT:
	    printf("	'CLOSE DISPLAY' BUTTON HAS BEEN PRESSED\n");
	    XtDestroyWidget(mData->mWindow[SCOPE_WINDOW]->GetShell());
	    break;
      case IDM_SCOPE_CHANGE:
	    printf("	'CHANGE SETTINGS' BUTTON HAS BEEN PRESSED\n");
	
	    break;	
      case IDM_SCOPE_LOAD:
	    printf("	'LOAD FROM FILE' BUTTON HAS BEEN PRESSED\n");
	
	    break;	
      case IDM_SCOPE_SAVE:
	    printf("	'SAVE TO FILE' BUTTON HAS BEEN PRESSED\n");
	
	    break;      
   } // end switch
   
}



// NOTE: the open file dialog doesn't seem to work properly. 
// For example:
// 1. Open the scope client window
// 2. Click on File < Open File
// 3. Close this dialog by clicking on the 'x' on top
// 4. Click on File < Close Display
// You'll notice that the whole program closes 
// (with the message "Error: XtPopdown requires a subclass of shellWidgetClass")
// rather than just the scope client window. 
// If you don't open the file open dialog first, then the scope window closes 
// properly via the File < Close Display button.
void PScopeImage::OpenFileBox()
{
    int 		n;
    Arg 		wargs[10];
    
    Widget	file_sel_box;
    XmString	title,dir_mask;
    // file selection dialog box
    //Widget	filebox;
    if (!mFilebox) {
        n = 0;
        XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_MODELESS);	++n;
        XtSetArg(wargs[n], XmNtitle, "Open Waveform File"); ++n;
        mFilebox = XmCreateBulletinBoardDialog(mData->mWindow[SCOPE_WINDOW]->GetShell(), "xsnoedFileBox", wargs, n);
        title = XmStringCreateLtoR("Open Waveform file...", XmFONTLIST_DEFAULT_TAG);
        dir_mask = XmStringCreateLtoR("*.wfm", XmFONTLIST_DEFAULT_TAG);
        n = 0;
        XtSetArg(wargs[n], XmNdirMask, dir_mask);	++n;
        XtSetArg(wargs[n], XmNfilterLabelString, title);	++n;
        file_sel_box = XmCreateFileSelectionBox(mFilebox, "selectionbox", wargs, n);
        
        XtUnmanageChild(XmFileSelectionBoxGetChild(file_sel_box,XmDIALOG_HELP_BUTTON));
        
        XtAddCallback(file_sel_box, XmNokCallback, (XtCallbackProc)FileOK, mData);
        XtAddCallback(file_sel_box, XmNcancelCallback, (XtCallbackProc)FileCancel, mData);
        XmStringFree(title);
        XmStringFree(dir_mask);
        XtManageChild(file_sel_box);
        XtAddCallback(mFilebox,XtNdestroyCallback,(XtCallbackProc)DestroyDialogProc,&mFilebox);
    } else if (XtIsManaged(mFilebox)) {
        // unmanage then manage again to raise window to top
        XtUnmanageChild(mFilebox);
        // XRaiseWindow(mData->display,XtWindow(mFilebox));
        // return;
    }
    XtManageChild(mFilebox);
}


void PScopeImage::FileOK(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data)
{
    //XtUnmanageChild(data->mMainWindow->filebox);
    //mWindow[SCOPE_WINDOW]
    
    char filename[FILELEN];
    strncvtXm(filename, call_data->value, FILELEN);
    data->mEventFile.SetString(filename);
    data->event_num = 0;
    //open_event_file(data,0);
    
    if (data->infile) {
        //data->mMainWindow->ShowWindow(EVT_NUM_WINDOW);
    } else {
        // re-open file dialog because the file didn't open
        //XtManageChild(data->mMainWindow->filebox);
    }
}


// I wasn't able to figure out how to call the filebox widget from this method!
void PScopeImage::FileCancel(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data)
{
	//XtUnmanageChild(w);
	//XtUnmanageChild(data->mMainWindow->filebox);
	
	//XtDestroyWidget(data->mWindow[SCOPE_WINDOW]->GetFilebox());	
}

void PScopeImage::DestroyDialogProc(Widget w, Widget **dialogPtr, caddr_t call_data)
{
	// must zero our pointer since the dialog is gone
	*dialogPtr = NULL;
}
