#ifndef __PSettingsWindow_h__
#define __PSettingsWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "ImageData.h"


class PSettingsWindow : public PWindow, public PListener {
public:
	PSettingsWindow(ImageData *data);

	virtual void	Show();
	virtual void	Listen(int message, void *dataPt);
	
private:
	void			SetHexID(int on);
	void			SetAngle(int flag);
	void			SetLabel(int show_label);
	void			SetTimeZone(int tz);
	void			AddLabel(char *aString);
	int				SetLabelFormat();
	
	static void		SetHitXYZ(ImageData *data, int on);
	static void		SetHitSize(ImageData *data, float hit_size);
	static void		SetNCDSize(ImageData *data, float ncd_size);
	
	static void		DecProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		HexProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		AngleProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		LabelFormatProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		LabelProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		AddRunProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		AddGTIDProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		AddTimeProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		AddNHitProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		ClearProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		SudburyProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		LocalProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		UTCProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		OkProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		CancelProc(Widget w, PSettingsWindow *set_win, caddr_t call_data);
	static void		ScaleMovedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data);
	static void 	ScaleChangedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data);
	static void		NCDMovedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data);
	static void 	NCDChangedProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data);
	static void 	XYZProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data);
	static void 	SaveConfigProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data);
	
	int				mSave_hex_id;
	int				mSave_time_zone;
	int				mSave_angle_rad;
	int				mSave_show_label;
	int				mSave_hit_xyz;
	float			mSave_hit_size;
	float           mSave_ncd_size;
	int				mSave_save_config;
	char			mSave_label_format[FORMAT_LEN];

	Widget			hex_radio, dec_radio;
	Widget			angle_radio[3];
	Widget			utc_radio, local_radio, sudbury_radio;
	Widget			label_toggle;
	Widget			label_text;
	Widget          clear_sum_text, clear_sum_toggle;
	
	static double	sUpdateTime;
};

#endif // __PSettingsWindow_h__
