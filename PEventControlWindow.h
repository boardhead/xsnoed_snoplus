#ifndef __PEventControlWindow_h__
#define __PEventControlWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PMenu.h"
#include "ImageData.h"

enum TriggerFlag {
	TRIGGER_OFF,						/* trigger is off */
	TRIGGER_SINGLE,						/* capture next event only */
	TRIGGER_CONTINUOUS					/* run continuously */
};


class PEventControlWindow : public PWindow, public PListener, public PMenuHandler {
public:
	PEventControlWindow(ImageData *data);
	
	virtual void	Listen(int message, void *dataPt);
	virtual void	Show();
	virtual void	DoMenuCommand(int anID);
	
	void			UpdateTriggerText();
	void			UpdateEventNumber();
	void			UpdateHistoryLabel(int isHistory);
	int             GetTriggerFlag()    { return mTriggerFlag; }
	
	static void		UpdateTriggerText(ImageData *data);
	static void		UpdateEventNumber(ImageData *data);
	static void		SetEventFilter(ImageData *data);
	static void		SetNhitLogic(ImageData *data, char *str);
	static void		SetTriggerMaskLogic(ImageData *data, char *str);
	static void     SetPmtNcdLogic(ImageData *data, char *str);
	static int		PrintTriggerMask(ImageData *data, char *buff);
	static int      PrintNhitLogic(ImageData *data, char *buff);
	static int      PrintPmtNcdLogic(ImageData *data, char *buff);

	static void		GotoProc(Widget w,PEventControlWindow *pe_win, caddr_t call_data);
	static void		WriteProc(Widget w,PEventControlWindow *pe_win, caddr_t call_data);
	
private:
	void			SetNhitText();
	void			SetTriggerMaskText();
	void            SetPmtNcdLogicText();
	void			SetGotoType(int type);
	void            CreateTriggerRadio(int num);
	
	int				mTriggerFlag;	// flag for radio button activated
	Widget			trigger_radio[3];	/* trigger types */
	Widget			event_text, thresh_text, trigger_text;
	Widget			trigger_label, history_label, goto_button;
	Widget          pmt_ncd_text;
	Widget			outfile_text[NUM_WRITERS];
	Widget			outfile_button[NUM_WRITERS];
	Widget          mDeadButton;
};

#endif // __PEventControlWindow_h__
