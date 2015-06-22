#ifndef __PSpeaker_h__
#define __PSpeaker_h__

#include "messages.h"

// special speaker message sent before the speaker dies
const unsigned long	kMessageSpeakerDying = 0xdead;

class PListener;

class PSpeaker {
public:
	PSpeaker();
	virtual ~PSpeaker();
	
	void		Speak(int message, void *dataPt=0);
	
	void		AddListener(PListener *aListener);
	void		RemoveListener(PListener *aListener);
	
	int			GetNumListeners()	{ return mNumListeners;	}
	
private:
	PListener	 **	mListeners;
	int				mNumListeners;
	int				mMaxListeners;
};

#endif // __PSpeaker_h__
