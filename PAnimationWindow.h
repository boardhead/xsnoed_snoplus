#ifndef __PAnimationWindow_h__
#define __PAnimationWindow_h__

#include "PWindow.h"
#include "PListener.h"

class PAnimationWindow : public PWindow, public PListener {
public:
	PAnimationWindow(ImageData *data);
	~PAnimationWindow();
	
	virtual void	Show();
	virtual void	Listen(int message, void *dataPt);
	
private:
	void			DoAnimation();
	void			DoWindowHits();
	void			ValuesChanged();
	void			TurnOffWindowing();
	
	static void		AnimateProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data);
	static void		ValueChangedProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data);
	static void		WindowToggleProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data);
	static void		DoneProc(Widget w, PAnimationWindow *animateWin, caddr_t call_data);
	static void		ScaleProc(Widget w, PAnimationWindow *animateWin, XmScaleCallbackStruct *call_data);
	
	Widget			start_text, end_text, width_text, frames_text, delay_text, file_text;
	Widget			time_slide, file_toggle, win_toggle;
	
	double			mSliderValue, mWindowWidth;
	double			mMinTime, mMaxTime, mTimeRange;
	int				mHaveTimeRange;
	int				mHiddenHits;
	int				mIsWindow;
};


#endif // __PAnimationWindow_h__
