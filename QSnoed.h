// File:	QSnoed.h
// Author:	Phil Harvey - 11/21/98

#ifndef __QSnoed__
#define __QSnoed__

#include "TObject.h"
#include "PListener.h"

class QEvent;
class QTree;
class QFit;
class TFile;
class TEventList;
class TH1;
class QCal;
class ImageData;

typedef void (* QSnoedCallbackPtr)(void);

/* QSnoed class definition */
class QSnoed : public TObject, public PListener
{
public:
					QSnoed();
					QSnoed(ImageData *data);
	virtual 	   ~QSnoed();
	
	virtual void 	OpenDisplay();		// re-open the xsnoed display
	virtual void 	CloseDisplay();		// close windows and free memory
	
	virtual Bool_t	IsOpen()			{ return mData!=NULL;	}
	
	virtual void	View(Text_t *aFilename="localhost");
	virtual void	View(TFile *aFile);
	virtual void	View(QTree *aTree);
	virtual void	View(TEventList *aList, QTree *aTree=NULL);
	virtual void	View(QEvent *anEvent);
	virtual void	View(Int_t anEventNum);
	virtual void	View(TH1 *hist_qhl, TH1 *hist_tac=NULL, TH1 *hist_qhs=NULL, TH1 *hist_qlx=NULL);
	
	virtual void	SetAutoScale(Bool_t on);
	
	virtual void	Goto(Int_t aGTID);
	virtual void	Cut(Text_t *aCutString = NULL);
	virtual void	CloseFile();
	
	virtual void	SetRch(TFile *aFile = NULL);
	virtual void	SetRch(char *filename);
	
	virtual void	Add(QFit *aFit);
	
	virtual void	SetEventCallback(QSnoedCallbackPtr = NULL);
	virtual void	SetCal(QCal *pca); 	//mgb must delete when done.
	
	int				SelectMenuItem(int item_id, int window_id=0);
	void			SetMenuItemText(int item_id, char *str, int window_id=0);
    int		        PrintImage(char *filename, int flags=0, int window_id=0);
	void			SetScale(float minval, float maxval, Bool_t doUpdate=kTRUE);
	
	QEvent		  *	GetEvent();
	QTree		  *	GetTree();
	TFile		  *	GetFile();
	TEventList	  *	GetEventList();
	
	void			Next();
	void			Prev();
	void			Cont(float sec=0);
	void			Stop();
	void			Clear();
	void			Service();
	void			Home();
	void			Sun();
	
	virtual void	Listen(int message, void *message_data);
	
	ClassDef(QSnoed,0)		// Interface to SNO Event Display
	
private:
	void			Init();
	void			Init(ImageData *data);
	
	ImageData	  *	mData;
	TFile		  *	mRchFile;
	Bool_t			mAutoScale;
};

extern QSnoed	*gSnoed;	// global QSnoed object

#endif
