#ifndef __PHitInfoWindow_h__
#define __PHitInfoWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PLabel.h"
#include "PExtraLabels.h"


class PHitInfoWindow : public PWindow, public PExtraLabels, public PListener {
public:
	PHitInfoWindow(ImageData *data);
	~PHitInfoWindow();
	
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);

protected:
	virtual char *	GetLabelString(int num);
	
private:
	void			ClearEntries();
	void			SetRateMode();
	void			SetHitXYZ();
	void			ManageXYZ(int manage);
	void			ResizeToFit();
	
	PLabel			hi_gt, hi_tac, hi_qhs, hi_qhl, hi_qlx, hi_qlx_label, hi_hit_label;
	PLabel			hi_nhit, hi_crate, hi_card, hi_channel, hi_cell, hi_type, hi_panel;
	PLabel			hi_pmt, hi_xyz_labels[3], hi_xyz[3];
	int				mLastNum;
	int				mCheckExtraNum;
	int				qlxFlag;			/* hit info Qlx label (QlxFlag enum) */
};


#endif // __PHitInfoWindow_h__
