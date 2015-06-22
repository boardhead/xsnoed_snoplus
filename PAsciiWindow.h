#ifndef __PAsciiWindow_h__
#define __PAsciiWindow_h__

#include "PWindow.h"
#include "PListener.h"
#include "PMenu.h"

const int kCommentHistoryNum	= 100;

class PZdabWriter;

class PAsciiWindow : public PWindow, public PListener, public PMenuHandler {
public:
	PAsciiWindow(ImageData *data);
	~PAsciiWindow();
	
	virtual void	Show();
	virtual void	Listen(int message, void *dataPt);
	virtual void	DoMenuCommand(int anID);
	
	void			WriteComment();
	
private:
	void			SwitchComment(int dir);
	void			SaveSettings();
	
	static void		WriteProc(Widget w, PAsciiWindow *asciiWin, caddr_t call_data);
	static void		AutoProc(Widget w, PAsciiWindow *asciiWin, caddr_t call_data);
	static void		DoneProc(Widget w, PAsciiWindow *asciiWin, caddr_t call_data);
	static void		Previous(Widget w, XEvent *ev, String *params, Cardinal *num_params);
	static void		Next(Widget w, XEvent *ev, String *params, Cardinal *num_params);

	Widget			filename_text, format_text, comment_text;
	int				mAutoNext;
	char		  *	mComments[kCommentHistoryNum];
	int				mNumComments;
	int				mCurComment;
	int				mExtraHeight;
	int				mCurMenuItem;
	PZdabWriter	  *	mWriter;
};


#endif // __PAsciiWindow_h__
