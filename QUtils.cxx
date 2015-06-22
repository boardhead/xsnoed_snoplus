////////////////////////////////////////////////
// SNO Utility routines
// - QTokenizer class
// - CompareEventIDs()
////////////////////////////////////////////////

//*-- Author :	Phil Harvey - 02/26/99

#include <string.h>
#include "QUtils.h"

ClassImp(QTokenizer)


QTokenizer::QTokenizer()
{
	mNextToken = NULL;
}

Text_t *QTokenizer::NextToken(Text_t *input_str, Text_t *delim)
{
	// Get next token in string (custom strtok routine)
	Text_t		*the_token;
	static Text_t *sDelim = " ,\t\r\n";
	
	if (!delim) delim = sDelim;	// use default delimiters
	
	if (input_str) mNextToken = input_str;
	
	if ((the_token = mNextToken) != NULL) {
		// find first non-delimiter
		while (*the_token && strchr(delim,*the_token)) ++the_token;
		if (*the_token) {
			// find delimiter for end of this token
			Text_t *token_end = the_token + 1;
			while (*token_end && !strchr(delim,*token_end)) ++token_end;
			if (*token_end) {
				*token_end = '\0';	// terminate the token
				mNextToken = token_end + 1;	// save pointer to start of next token
			} else {
				// token ends at end of string - no next token
				mNextToken = NULL;
			}
		} else {
			the_token = NULL;	// no token found
			mNextToken = NULL;
		}
	}
	return(the_token);
}


Int_t CompareEventIDs(Int_t gtid1, Int_t gtid2)
{
	// compare two event GTID's within a run
	// returns: -1 -> (gtid1 comes before gtid2)
	//           0 -> (gtid1 = gtid2)
	//           1 -> (gtid1 comes after gtid2)
	Int_t diff = gtid2 - gtid1;
	if (!diff) return(0);
	if (diff < 0) diff += 0x01000000L;	// convert to the range 0-0x00ffffffL
	if (diff < 0x00800000L) {
		return(-1);		// id1 is before id2 in the run
	} else {
		return(1);		// id1 is after id2 in the run
	}
}

