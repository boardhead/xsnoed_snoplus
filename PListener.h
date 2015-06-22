#ifndef __PListener_h__
#define __PListener_h__

#include "messages.h"

class PSpeaker;

class PListener {
public:
	friend class PSpeaker;
	PListener();
	virtual ~PListener();
	
	virtual void	Listen(int message, void *dataPt) = 0;
	
	int				IsIgnoring()				{ return mIgnoring;		 }
	void			Ignore(int ignore_on=1) 	{ mIgnoring = ignore_on; }
	
private:
	void			AddSpeaker(PSpeaker *aSpeaker);
	void			RemoveSpeaker(PSpeaker *aSpeaker);
	
	int				mIgnoring;
	PSpeaker	 **	mSpeakers;
	int				mNumSpeakers;
	int				mMaxSpeakers;
};

#endif // __PListener_h__
