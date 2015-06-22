#ifndef __PPrintWindow_h__
#define __PPrintWindow_h__

#include "ImageData.h"
#include "PWindow.h"
#include "PNotifyRaised.h"
#include "PListener.h"

enum EPrintType {
	kPrintImage,
	kPrintWindow
};

const short	kMaxPrintArgs = 20;


class XSnoedWindow;
class PImageWindow;

class PPrintWindow : public PWindow, public PNotifyRaised, public PListener {
public:
	PPrintWindow(ImageData *data, EPrintType printType);
	~PPrintWindow();
	
	void			DoPrint();
	void			SetPrintType(EPrintType printType);
	
	int				GetSaveCol()		{ return mSaveCol; }
	int				GetSaveLabel()		{ return mSaveLabel; }
	
	virtual void	Listen(int message, void *dataPt);
	virtual void	NotifyRaised(PImageWindow *aWindow);
	
private:
	void			SetColours(int colourSet);
	void			ShowLabels(int on);
	void			SetTarget(int to_file);
	void			SaveSettings();
	void			PromptToClick();
	void			ContinuePrinting(PImageWindow *aWindow);
	
	static void		ToPrinterProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		ToFileProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		PrintProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		ColoursProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		GreyProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		PrintLabelProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		CancelProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		WarnOKProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		WarnCancelProc(Widget w, PPrintWindow *printWin, caddr_t call_data);
	static void		WarnDestroyProc(Widget w, PPrintWindow *printWin, caddr_t call_data);

	EPrintType		mPrintType;			// type of print window open
	int				mSaveCol;			// save colours before printing
	int				mSaveLabel;			// save labels before printing
	Widget			print_text, cmd_label, print_button, cancel_button;
	Widget			col_toggle, grey_toggle, label_toggle;
	Widget			target_label, target_radio[2];
	
	int				mArgc;
	char		  *	mArgs[kMaxPrintArgs];
	char			mPrintName[FILELEN];
	char			mTempFilename[256];
	int				mPrintFlags;
	int				mToFile;
	Widget			mWarnDialog;
};


#endif // __PPrintWindow_h__
