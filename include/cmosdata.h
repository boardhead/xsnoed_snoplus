#include "include/sno_sys.h"

#ifdef SWAP_BYTES

typedef struct CmosCrateHeader {
  unsigned BoardMask :16;
  unsigned CrateID    :5;
  unsigned DataType :11; //1=Rate Data 2=Board data
} aCmosCrateHeader, *aCmosCrateHptr;

#else // SWAP_BYTES

typedef struct CmosCrateHeader {
  unsigned DataType :11; //1=Rate Data 2=Board data
  unsigned CrateID    :5;
  unsigned BoardMask :16;
} aCmosCrateHeader, *aCmosCrateHptr;

#endif // SWAP_BYTES

typedef struct CmosBoardHeader {
  unsigned ChannelMask :32;
} aCmosBoardHeader, *aCmosBoardHptr;

typedef struct CmosBoardData {
  float temperature;
  float pos24;
  float neg24;
  float pos6;
  float neg6;
  float blah;
  float blahh;
  float blaah;
  float bablah;
  float blaahh;
  float blaugh;
  float balah;
  float blaaugh;
  float blaughh;
} aCmosBoardData, *aCmosBoardDptr;

typedef struct CmosChannelData {
  float rate;
} aCmosChannelData, *aCmosChannelDptr;
