//-------------------------
// QUtils.h
//
#ifndef __QUtils_h__
#define __QUtils_h__

#include "Rtypes.h"

class QTokenizer {
public:
	QTokenizer();
	
	Text_t		  *	NextToken(Text_t *start=NULL, Text_t *delim=NULL);
	
private:
	Text_t		  *	mNextToken;	// pointer to next character after last token

	ClassDef(QTokenizer,0)	// Tokenize a string (object-oriented version of strtok)
};


Int_t CompareEventIDs(Int_t gtid1, Int_t gtid2);


#endif // __QUtils_h__
