#include <stdio.h>
#include <stdlib.h>
#include "PSpeaker.h"
#include "PListener.h"

const int	kListInc	= 10;

PSpeaker::PSpeaker()
{
	mListeners = NULL;
	mNumListeners = 0;
	mMaxListeners = 0;
}


PSpeaker::~PSpeaker()
{
	Speak(kMessageSpeakerDying, this);
	
	while (mNumListeners) {
		RemoveListener(*mListeners);
	}
	delete [] mListeners;
}


void PSpeaker::AddListener(PListener *aListener) 
{
	int	i;
	
	// don't add a listener twice
	for (i=0; i<mNumListeners; ++i) {
		if (aListener == mListeners[i]) return;
	}
	/* add listener to this speaker's list */
	if (mNumListeners >= mMaxListeners) {
		PListener **tmp = new PListener*[mMaxListeners + kListInc];
		if (!tmp) {
			fprintf(stderr,"Out of memory in PSpeaker::AddListener\n");
			exit(1);
		}
		for (i=0; i<mNumListeners; ++i) tmp[i] = mListeners[i];
		delete [] mListeners;
		mListeners = tmp;
		mMaxListeners += kListInc;
	}
	mListeners[mNumListeners++] = aListener;
	
	/* add this speaker to listener's list */
	aListener->AddSpeaker(this);
}

void PSpeaker::RemoveListener(PListener *aListener)
{
	for (int i=0; i<mNumListeners; ++i) {
		if (aListener == mListeners[i]) {
			for (int j=i+1; j<mNumListeners; ++j) {
				mListeners[j-1] = mListeners[j];
			}
			--mNumListeners;
		}
	}
	
	/* remove this speaker from the listener's list */
	aListener->RemoveSpeaker(this);
}

void PSpeaker::Speak(int message, void *dataPt)
{
	for (int i=0; i<mNumListeners; ++i) {
		if (!mListeners[i]->IsIgnoring()) {
			mListeners[i]->Listen(message, dataPt);
		}
	}
}
