#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class QSnoed;
#pragma link C++ class QXsnoedSystem;
#ifndef NO_DISPATCH
#pragma link C++ class QDispatch;
#endif
#pragma link C++ class QPmtEventRecord;
#pragma link C++ class QSnoDBInterface;
#pragma link C++ class QZdabFile;
#pragma link C++ class QRootFile;
#pragma link C++ class QEventReader;
#pragma link C++ class QBufferedEventReader;
#pragma link C++ class QRecordBuffer;
#pragma link C++ class QListFile;
#pragma link C++ class QSimpleCal;
#pragma link C++ class QTokenizer;

#pragma link C++ function CreateEventReader;
#pragma link C++ function CompareEventIDs;

//#pragma link C++ global gSnoed;
//#pragma link C++ global gPCA;
#pragma link C++ global gDispatch;

//#pragma link C++ enum EMessageTypes;

#endif
