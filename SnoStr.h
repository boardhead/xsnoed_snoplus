#ifndef __SnoStr_h__
#define __SnoStr_h__

#include "include/sno_sys.h"

const unsigned kNumInteractionTypes	= 62;
const unsigned kNumParticleTypes	= 24;
const unsigned kNumManipStatus		= 16;
const unsigned kNumEPEDTypes		= 4;
const unsigned kNumEPEDFlags		= 5;

enum ESourceOrientation {
    kOrientation_Unknown,
    kOrientation_North,
    kOrientation_East,
    kOrientation_South,
    kOrientation_West,
    kNumSourceOrientations
};

struct LookupStruct {
	int code;
	char *name;
};


class SnoStr {
public:
	static int				GetList(char *outbuf, char **strs, u_int32 mask);
	
	static char			  *	Lookup(int code, LookupStruct *lookup, int num);
	static int				GetIndex(int code, LookupStruct *lookup, int num);

	static char			  *	LookupInteraction(int n){ return Lookup(n,sInteractionLookup,kNumInteractionTypes); }
	static char			  *	LookupParticle(int n)	{ return Lookup(n,sParticleLookup,kNumParticleTypes); }
	static int				GetParticleIndex(int n)	{ return GetIndex(n,sParticleLookup,kNumParticleTypes); }
/*
** static member variables
*/
	static LookupStruct		sInteractionLookup[kNumInteractionTypes];
	static LookupStruct		sParticleLookup[kNumParticleTypes];		

	static char			  *	sRunType[32];
	static char			  *	sTrigMask[32];
	static char			  *	sXsnoedTrig[32];
#ifdef SNOPLUS
	static char           * sFECD[32];
#endif
	static char			  *	sSourceMask[32];
	static char			  *	sManipStatus[kNumManipStatus];
	static char			  *	sEPEDFlag[kNumEPEDFlags];
	static char			  *	sEPEDType[kNumEPEDTypes];
	static char           * sSourceOrientation[kNumSourceOrientations];
};

#endif // __SnoStr_h__
