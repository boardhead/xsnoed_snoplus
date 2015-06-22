#ifndef __PMonteCarloWindow_h__
#define __PMonteCarloWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PMenu.h"

struct MonteCarloVertex;
class PProjImage;

class PMonteCarloWindow : public PWindow, public PListener, public PMenuHandler {
public:
	PMonteCarloWindow(ImageData *data);
	~PMonteCarloWindow();
	
	virtual void			DoMenuCommand(int anID);
	virtual void			Listen(int message, void *message_data);
	
	static char			  *	InteractionName(MonteCarloVertex *vertex);
	static char			  *	ParticleName(MonteCarloVertex *vertex);
	static unsigned long	ParticleMask(MonteCarloVertex *vertex);

private:
	int						ShowTrackInfo(MonteCarloVertex *vertex, MonteCarloVertex *flagged=NULL);
	int						ShowTrackInfo(int x,int y, int forced,PProjImage *proj);
	void					SetParticleItems();
	void					ClearEntries();
	void					HighlightTrack(MonteCarloVertex *vertex);
	
	Widget					mTitleLabel, mTrackLabel;
	MonteCarloVertex	  *	mLastVertex;
	MonteCarloVertex	  *	mLastFlagged;
	int						mNeedUpdate;
};


#endif // __PMonteCarloWindow_h__
