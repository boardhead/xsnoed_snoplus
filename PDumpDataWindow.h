//added MAH 04/21/00
#ifndef __PDumpDataWindow_h__
#define __PDumpDataWindow_h__

#include "PWindow.h"
#include "PListener.h"
#include "PMenu.h"

const int kCommentHistoryNum2	= 100;

class PDumpDataWindow : public PWindow, public PListener
{
public:
	PDumpDataWindow(ImageData *data);
	~PDumpDataWindow();
	
	virtual void	Show();
	virtual void	Listen(int message, void *dataPt);
	
	void		DumpDisplayData();

private:
	void		SwitchComment(int dir);
	void		SaveSettings();
	
	static void	WriteProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data);
	static void	AppendProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data);
	static void	OverwriteProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data);
	static void	DoneProc(Widget w, PDumpDataWindow *dumpWin, caddr_t call_data);

	Widget		filename_text, comment_text, append_radio, overwrite_radio;
	char		*mComments[kCommentHistoryNum2];
	int			mNumComments;
	int			mCurComment;
	int			mOverwrite;
};


#endif // __PDumpDataWindow_h__
