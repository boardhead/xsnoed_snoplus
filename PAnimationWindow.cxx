//
// File:		PAnimationWindow.cxx
//
// Description:	Time-animated hit display
//
// Created:		05/15/01 - Phil Harvey
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#ifndef NO_FORK
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#endif
#include "PAnimationWindow.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "ImageData.h"
#include "calibrate.h"
#include "xsnoed.h"

#define WINDOW_WIDTH			444
#define WINDOW_HEIGHT			246

PAnimationWindow::PAnimationWindow(ImageData *data)
			: PWindow(data)
{
	int			n;
	Arg			wargs[16];
	Widget		w, but;
	
	mSliderValue = 0;
	mHaveTimeRange = 0;
	mHiddenHits = 0;
	mIsWindow = 0;
	
  	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Animation"); ++n;
	XtSetArg(wargs[n], XmNx, 400); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNminWidth, WINDOW_WIDTH); ++n;
	XtSetArg(wargs[n], XmNminHeight, WINDOW_HEIGHT); ++n;
	SetShell(CreateShell("animatePop",data->toplevel,wargs,n));
	w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0);
	SetMainPane(w);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 201); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	but = XtCreateManagedWidget("Animate",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)AnimateProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 348); ++n;
	XtSetArg(wargs[n], XmNy, 201); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	but = XtCreateManagedWidget("Done",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)DoneProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 20); ++n;
	XtCreateManagedWidget("Hit Time (%):",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 156); ++n;
	XtSetArg(wargs[n], XmNy, 10); ++n;
	XtSetArg(wargs[n], XmNwidth, 272); ++n;
	XtSetArg(wargs[n], XmNminimum, 0); ++n;
	XtSetArg(wargs[n], XmNmaximum, 1000); ++n;
	XtSetArg(wargs[n], XmNdecimalPoints, 1); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
	XtSetArg(wargs[n], XmNvalue, 0); ++n;
	time_slide = XtCreateManagedWidget("Scale",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(time_slide,XmNvalueChangedCallback,(XtCallbackProc)ScaleProc,this);
	XtAddCallback(time_slide,XmNdragCallback,(XtCallbackProc)ScaleProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 60); ++n;
	XtCreateManagedWidget("Time Window (%):", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 156); ++n;
	XtSetArg(wargs[n], XmNy, 55); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	width_text = XtCreateManagedWidget("widthText",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(width_text,XmNactivateCallback,(XtCallbackProc)ValueChangedProc,this);
	setTextString(width_text,"100.0");
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 246); ++n;
	XtSetArg(wargs[n], XmNy, 55 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	win_toggle = XtCreateManagedWidget("Apply Window",xmToggleButtonWidgetClass,w,wargs,n);
	XtAddCallback(win_toggle,XmNvalueChangedCallback,(XtCallbackProc)WindowToggleProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 95); ++n;
	XtCreateManagedWidget("Start Time (%):", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 156); ++n;
	XtSetArg(wargs[n], XmNy, 90); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	start_text = XtCreateManagedWidget("startText",xmTextWidgetClass,w,wargs,n);
	setTextString(start_text,"0.0");

	n = 0;
	XtSetArg(wargs[n], XmNx, 246); ++n;
	XtSetArg(wargs[n], XmNy, 95); ++n;
	XtCreateManagedWidget("End Time (%):", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 356); ++n;
	XtSetArg(wargs[n], XmNy, 90); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	end_text = XtCreateManagedWidget("endText",xmTextWidgetClass,w,wargs,n);
	setTextString(end_text,"100.0");

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 130); ++n;
	XtCreateManagedWidget("Number of Frames:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 156); ++n;
	XtSetArg(wargs[n], XmNy, 125); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	frames_text = XtCreateManagedWidget("framesText",xmTextWidgetClass,w,wargs,n);
	setTextString(frames_text,"20");

	n = 0;
	XtSetArg(wargs[n], XmNx, 246); ++n;
	XtSetArg(wargs[n], XmNy, 130); ++n;
	XtCreateManagedWidget("Delay (sec):", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 356); ++n;
	XtSetArg(wargs[n], XmNy, 125); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	delay_text = XtCreateManagedWidget("delText",xmTextWidgetClass,w,wargs,n);
	setTextString(delay_text,"0.05");
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 160 + RADIO_OFFSET); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	file_toggle = XtCreateManagedWidget("Write Animated GIF:",xmToggleButtonWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 194); ++n;
	XtSetArg(wargs[n], XmNy, 160); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	file_text = XtCreateManagedWidget("fileText",xmTextWidgetClass,w,wargs,n);
	setTextString(file_text,"xsnoed_animation.gif");

	// listen for messages from xsnoed speaker
	data->mSpeaker->AddListener(this);
}


PAnimationWindow::~PAnimationWindow()
{
	TurnOffWindowing();
}

void PAnimationWindow::TurnOffWindowing()
{
	// turn off window toggle if not done already
	if (mIsWindow) {
		setToggle(win_toggle, 0);
		mIsWindow = 0;
	}
	if (mHiddenHits) {
		mHiddenHits = 0;
		// redraw all hits
		ImageData *data = GetData();
		int num = data->hits.num_nodes;
		HitInfo *hi = data->hits.hit_info;
		for (int i=0; i<num; ++i,++hi) {
			hi->flags &= ~HIT_HIDDEN;
		}
		sendMessage(data, kMessageHitsChanged);
	}
}

void PAnimationWindow::DoWindowHits()
{
	int			i, num;
	float		tmp;
	double		val;
	HitInfo	  *	hi;
	ImageData *	data = GetData();
	
	// turn on window toggle if not done already
	if (!mIsWindow) {
		setToggle(win_toggle, 1);
		mIsWindow = 1;
	}
		
	if (!mHaveTimeRange) {
		
		ValuesChanged();	// make sure all values are current

		// run through events to calculate time range
		if ((num=data->hits.num_nodes) != 0) {

			hi = data->hits.hit_info;
			for (i=0; i<num; ++i,++hi) {
				if (data->wCalibrated != IDM_UNCALIBRATED) {
					tmp = hi->calibrated;
					setCalibratedTac(data,hi,num);
					if ((val = hi->calibrated) != INVALID_CAL) {
						if (!mHaveTimeRange) {
							mHaveTimeRange = 1;
							mMinTime = mMaxTime = val;
						} else if (mMaxTime < val) {
							mMaxTime = val;
						} else if (mMinTime > val) {
							mMinTime = val;
						}
					}
					hi->calibrated = tmp;
				} else {
					// Note: min and max are reversed here because time scale is backwards
					val = hi->tac;
					if (!mHaveTimeRange) {
						mHaveTimeRange = 1;
						mMinTime = mMaxTime = val;
					} else if (mMaxTime > val) {
						mMaxTime = val;
					} else if (mMinTime < val) {
						mMinTime = val;
					}
				}
			}
		} else {
			mHaveTimeRange = 1;
			mMinTime = 0;
			mMaxTime = 1;
		}
		mTimeRange = mMaxTime - mMinTime;
		if (!mTimeRange) mTimeRange = 1;	// protect against divide by zero
	}
	
	// run through events to hide ones that are out of the window
	num = data->hits.num_nodes;

	mHiddenHits = 0;
	
	hi = data->hits.hit_info;
	for (i=0; i<num; ++i,++hi) {
		if (data->wCalibrated != IDM_UNCALIBRATED) {
			tmp = hi->calibrated;
			setCalibratedTac(data,hi,num);
			val = hi->calibrated;
			hi->calibrated = tmp;
		} else {
			val = hi->tac;
		}
		// calculate time value as a percent of full range
		val = (val - mMinTime) / mTimeRange;
		
		if (mWindowWidth > 0) {
			if ((val >= mSliderValue-mWindowWidth) && (val <= mSliderValue)) {
				hi->flags &= ~HIT_HIDDEN;
			} else {
				hi->flags |= HIT_HIDDEN;
				++mHiddenHits;
			}
		} else {
			if ((val >= mSliderValue) && (val <= mSliderValue-mWindowWidth)) {
				hi->flags &= ~HIT_HIDDEN;
			} else {
				hi->flags |= HIT_HIDDEN;
				++mHiddenHits;
			}
		}
	}
	
	sendMessage(data, kMessageHitsChanged);
}

void PAnimationWindow::ValuesChanged()
{
	mHaveTimeRange = 0;	// force recalculation of time range
	
	char *str = XmTextGetString(width_text);
	mWindowWidth = atof(str) / 100.0;
	XtFree(str);	// must free the string
}

void PAnimationWindow::DoAnimation()
{
	int		i;
	
	char *str = XmTextGetString(frames_text);
	int num_frames = atoi(str) + 1;		// do one more frame than they wanted
	XtFree(str);	// must free the string
	
	if (num_frames<2 || num_frames>10000) {
		Printf("Invalid number of frames!  Must be between 2 and 10000\x07\n");
		return;
	}
	
	mHaveTimeRange = 0;		// force re-calculation of time scale
	
	str = XmTextGetString(start_text);
	double start_time = atof(str) / 100.0;
	XtFree(str);	// must free the string
	
	str = XmTextGetString(end_text);
	double end_time = atof(str) / 100.0;
	XtFree(str);	// must free the string
	
	// get the delay time (sec)
	str = XmTextGetString(delay_text);
	double delay = atof(str);
	XtFree(str);	// must free the string

	// get the animation filename
	char filename[512];
	str = XmTextGetString(file_text);
	strncpy(filename, str, 512);
	filename[511] = '\0';
	XtFree(str);	// must free the string
	
	if (delay > 10) {
		Printf("Hmmm, that delay time sure is long!\x07\n");
		return;
	}
	if (start_time == end_time) {
		Printf("Invalid start/end times!\x07\n");
		return;
	}
	
	int write_gif = 0;
	Arg warg;
	XtSetArg(warg, XmNset, &write_gif);
	XtGetValues(file_toggle, &warg, 1);
	
#ifndef NO_FORK
	pid_t	pid;
	char *file_tok = NULL;
	char *exec_args[10];
	char arg_buff[32];
	char fname_buff[512];
#endif

	if (write_gif) {
#ifdef NO_FORK
		Printf("Sorry, can't write animated GIF on system without fork() capability!\x07\n");
		return;
#else
		// write window ID of main image canvas into 'arg_buff'
		Window windowID = XtWindow(GetData()->mMainWindow->GetImage()->GetCanvas());
		sprintf(arg_buff,"0x%lx",(long)windowID);
		file_tok = strtok(filename," \t");	// remove leading/trailing spaces
		if (file_tok) {
			char *pt = strstr(file_tok,".gif");
			if (!pt) {
				Printf("Animation file name must end with '.gif'!\x07\n");
				return;
			} else {
				*pt = '\0';		// remove .gif extension
			}
			exec_args[0] = "import";
			exec_args[1] = "-silent";
			exec_args[2] = "-window";
			exec_args[3] = arg_buff;
			exec_args[4] = fname_buff;
			exec_args[5] = NULL;
		} else {
			Printf("Invalid file name for animated GIF!\x07\n");
			return;
		}
#endif
	}
	
	double incr = (end_time - start_time) / (num_frames - 1);
	
	// loop through all frames
	for (i=0; i<num_frames; ++i) {
	
		mSliderValue = start_time + i * incr;
	
		// set the slider value
		Arg warg;
		XtSetArg(warg, XmNvalue, (int)(mSliderValue * 1000 + 0.5));
		XtSetValues(time_slide, &warg, 1);
        XmUpdateDisplay(time_slide);	// update this window
		
		DoWindowHits();

		// NOTE: The XmUmpdateDisplay() MUST come after the PWindow::HandleUpdates()
		// to give the X server a chance to do it's thing after we draw the window
		// since we are in a tight loop here.  Otherwise, the image doesn't get
		// drawn before we want to capture it.
        PWindow::HandleUpdates();		// update all image windows now
        // make sure the window is updated
        XmUpdateDisplay(GetData()->mMainWindow->GetMainPane());
        
#ifndef NO_FORK
		if (file_tok) {
			pid = fork();
			if (pid == -1) {
				Printf("Fork error while capturing image\x07\n");
				return;
			} else if (pid) {
				waitpid(pid, NULL, 0);	// wait for capture to complete
			} else {
				// capture to .bmp format because it is way faster than .gif
				Printf("Capturing frame %d...\n", i);
				sprintf(fname_buff,"%s_%.5d.bmp",file_tok,i);
				execvp(exec_args[0], exec_args);
				Printf("%s error executing %s\x07\n",strerror(errno),exec_args[0]);
				// this form of exit does not call cleanup routines
				// - we don't want it to -- otherwise settings will be saved, etc.
				_exit(1);
			}
		} else {
			usleep_ph((unsigned long)(delay * 1e6));	// do the delay
		}
#else
		usleep_ph((unsigned long)(delay * 1e6));	// do the delay
#endif
	}
#ifndef NO_FORK
	if (file_tok) {
		pid = fork();
		if (pid == -1) {
			Printf("Fork error while creating animation\x07\n");
			return;
		} else if (pid) {
			waitpid(pid, NULL, 0);	// wait for capture to complete
			// clean up files
			for (i=0; i<num_frames; ++i) {
				sprintf(fname_buff,"%s_%.5d.bmp",file_tok,i);
				unlink(fname_buff);
			}
			Printf("Done creating %s.gif\n", file_tok);
		} else {
			Printf("Converting to animated gif format...\n",file_tok);
			sprintf(fname_buff,"%s_?????.bmp", file_tok);
			sprintf(arg_buff,"%d",(int)(delay * 100 + 0.5));
			*strchr(file_tok,'\0') = '.';	// put extension back on filename
			exec_args[0] = "convert";
			exec_args[1] = "-delay";
			exec_args[2] = arg_buff;
			exec_args[3] = fname_buff;
			exec_args[4] = file_tok;
			exec_args[5] = NULL;
			execvp(exec_args[0], exec_args);
			Printf("%s error executing %s\x07\n",strerror(errno),exec_args[0]);
			// this form of exit does not call cleanup routines
			// - we don't want it to -- otherwise settings will be saved, etc.
			_exit(1);
		}
	}
#endif
}

// Show - show the window (at the proper size!)
void PAnimationWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		Resize(WINDOW_WIDTH, WINDOW_HEIGHT);
	}
}

void PAnimationWindow::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageNewEvent:
			mHiddenHits = 0;
			mHaveTimeRange = 0;
			if (mIsWindow) {
				DoWindowHits();		// window the new event
			}
			break;
			
		case kMessageCalibrationChanged:
			ValuesChanged();	// just in case we switched from calibrated to uncalibrated data
			break;
	}
}

void PAnimationWindow::AnimateProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data)
{
	animateWin->DoAnimation();
}

void PAnimationWindow::ValueChangedProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data)
{
	animateWin->ValuesChanged();
	animateWin->DoWindowHits();
}

void PAnimationWindow::WindowToggleProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data)
{
	if (animateWin->mIsWindow ^= 0x01) {
		animateWin->DoWindowHits();
	} else {
		animateWin->TurnOffWindowing();
	}
}

void PAnimationWindow::ScaleProc(Widget w, PAnimationWindow *animateWin, XmScaleCallbackStruct *call_data)
{
	double	value = call_data->value / 1000.0;
	
	if (animateWin->mSliderValue != value) {
		animateWin->mSliderValue = value;
		animateWin->DoWindowHits();
	}
}

void PAnimationWindow::DoneProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data)
{
	// delete the window
	delete animateWin;
}


