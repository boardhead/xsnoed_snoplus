#ifndef __PFitterWindow_h__
#define __PFitterWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PLabel.h"

enum EFitQuality {
	kFitQuality,
	kFitSigma,
	kFitChisq
};

struct RconEvent;

class PFitterWindow : public PWindow, public PListener {
public:
	PFitterWindow(ImageData *data);
	~PFitterWindow();
	
	virtual void	Show();
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);

	static void		NextFitProc(Widget w, ImageData *data, caddr_t call_data);
	static void		FitChangedProc(Widget w, PFitterWindow *fit_win, caddr_t call_data);
	static void		NewFitProc(Widget w, ImageData *data, caddr_t call_data);
	
private:
	static void		CalcFitSigma(ImageData *data, RconEvent *rcon);
	
	Widget    		fi_pos,fi_time,fi_dir,fi_angle;
	PLabel			fi_qual, fi_id, fi_rad, fi_chi2,fi_nhit,fi_nhitw;
	
	int				last_qual;

public:
	static PFitterWindow *sCurrent;
};


#endif // __PFitterWindow_h__
