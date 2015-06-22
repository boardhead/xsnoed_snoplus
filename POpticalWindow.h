#ifndef __POpticalWindow_h__
#define __POpticalWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"

enum {
	UPDATE_OSA_INTENSITY = 0x01,
	UPDATE_OSA_POSITION	 = 0x02,
	UPDATE_OSA_ATTEN	 = 0x04,
	UPDATE_OSA_ALL		 = UPDATE_OSA_INTENSITY | UPDATE_OSA_POSITION | UPDATE_OSA_ATTEN
};


class POpticalWindow : public PWindow {
public:
	POpticalWindow(ImageData *data);
	
	virtual void	Show();
	
	void			UpdateOpticalConstants(int update_flags);
	static void		UpdateOpticalConstants(ImageData *data, int update_flags);

	static void		OcaProc(Widget w,POpticalWindow *oca_win, caddr_t call_data);
	static void		OcaPosProc(Widget w,POpticalWindow *oca_win, caddr_t call_data);
	
private:
	Widget			oca_pos_text, oca_acrylic, oca_h2o;
	Widget			oca_d2o, oca_intens_label;
};

#endif // __POpticalWindow_h__
