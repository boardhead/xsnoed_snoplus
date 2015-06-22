#include <stdio.h>
#include <stdlib.h>
#include "PListener.h"
#include "PSpeaker.h"

const int kListInc = 10;

PListener::PListener()
{
	mIgnoring = 0;
	mSpeakers = (PSpeaker **)0;
	mNumSpeakers = 0;
	mMaxSpeakers = 0;
}

PListener::~PListener()
{
	while (mNumSpeakers) {
		mSpeakers[0]->RemoveListener(this);
	}
	delete [] mSpeakers;
}

void PListener::AddSpeaker(PSpeaker *aSpeaker) 
{
	int	i;
	
	// don't add a speaker twice
	for (i=0; i<mNumSpeakers; ++i) {
		if (aSpeaker == mSpeakers[i]) return;
	}
	/* add speaker to this listener's list */
	if (mNumSpeakers >= mMaxSpeakers) {
		PSpeaker **tmp = new PSpeaker*[mMaxSpeakers + kListInc];
		if (!tmp) {
			fprintf(stderr,"Out of memory in PListener::AddSpeaker\n");
			exit(1);
		}
		for (i=0; i<mNumSpeakers; ++i) tmp[i] = mSpeakers[i];
		delete [] mSpeakers;
		mSpeakers = tmp;
		mMaxSpeakers += kListInc;
	}
	mSpeakers[mNumSpeakers++] = aSpeaker;
}

void PListener::RemoveSpeaker(PSpeaker *aSpeaker)
{
	for (int i=0; i<mNumSpeakers; ++i) {
		if (aSpeaker == mSpeakers[i]) {
			for (int j=i+1; j<mNumSpeakers; ++j) {
				mSpeakers[j-1] = mSpeakers[j];
			}
			--mNumSpeakers;
		}
	}
}

