#ifndef __QRchHist_h__
#define __QRchHist_h__

#include "PHistImage.h"
#include "PMenu.h"

class TFile;

class QRchHist : public PHistImage, public PMenuHandler {
public:
	QRchHist(PImageWindow *owner, int anID, Widget canvas=0);
	virtual ~QRchHist();
	
	virtual void	DoGrab(float xmin, float xmax);
	virtual void	MakeHistogram();
	virtual void	DoMenuCommand(int anID);
	virtual void	Listen(int message, void *dataPt);

	void			ReadRchData();
	
	static void		OpenRchFile(ImageData *data, QRchHist *hist);
	static void		CreateRchWindow(ImageData *data,char *name, int anID);
	
private:
	static void		RchFileOK(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data);
	static void		RchFileCancel(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data);
	static void		DestroyRchFileboxProc(Widget w, ImageData *data, caddr_t call_data);
	
	TFile		  *	mRchFile;
	int				mRchIndex;
	long			mDataMax;
	TFile		  *	mRchOverlayFile;
	int				mOverlayBins;
	
	static char	  *	sRchName[4];
};

#endif // __QRchHist_h__
