
#ifndef QUEENS_OCA
#define QUEENS_OCA

// Copyright (C) 1998 CodeSNO. All rights reserved.

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// OCA reading code                                                     //
//                                                                      //
// Reads in optical calibration data from a SNOMAN-produced file.       //
// This file contains distances from specific laserball positions to    //
//  all PMT positions in the detector.
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _MATH_H
#include <math.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

// Following ROOT defines are for Char, Int and Float types:
// Bool_t  - unsigned char,  1 byte
// Char_t  - signed char,    1 byte
// Int_t   - signed integer, 4 bytes
// Float_t - float,          4 bytes
//
// As well as:
// Bool_t kTRUE  = 1
// Bool_t kFALSE = 0
#ifndef _ROOT_Rtypes
typedef char           Char_t;      //Signed Character 1 byte
typedef unsigned char  UChar_t;     //Unsigned Character 1 byte
typedef short          Short_t;     //Signed Short integer 2 bytes
typedef unsigned short UShort_t;    //Unsigned Short integer 2 bytes
#ifdef R__INT16
typedef long           Int_t;       //Signed integer 4 bytes
typedef unsigned long  UInt_t;      //Unsigned integer 4 bytes
#else
typedef int            Int_t;       //Signed integer 4 bytes
typedef unsigned int   UInt_t;      //Unsigned integer 4 bytes
#endif
#ifdef R__B64
typedef int            Seek_t;      //File pointer
typedef long           Long_t;      //Signed long integer 4 bytes
typedef unsigned long  ULong_t;     //Unsigned long integer 4 bytes
#else
typedef int            Seek_t;      //File pointer
typedef long           Long_t;      //Signed long integer 8 bytes
typedef unsigned long  ULong_t;     //Unsigned long integer 8 bytes
#endif
typedef float          Float_t;     //Float 4 bytes
typedef double         Double_t;    //Float 8 bytes
typedef char           Text_t;      //General string
typedef unsigned char  Bool_t;      //Boolean (0=false, 1=true)
typedef unsigned char  Byte_t;      //Byte (8 bits)
typedef short          Version_t;   //Class version identifier
typedef char           Option_t;    //Option string
typedef int            Ssiz_t;      //String size
typedef float          Real_t;      //TVector and TMatrix element type

typedef void         (*VoidFuncPtr_t)();  //pointer to void function
const Bool_t kFALSE  = 0;
const Bool_t kTRUE   = 1;
#endif

// ------------------------------------------------------------------------
//  Constants
// ------------------------------------------------------------------------
#define kNCrate 20
#define kNCard 16
#define kNChannel 32
#define kMagicNumber 0xdecade91
#define kDistTol 30
#define kPosOutOfRange -9999
#define kDEBUG kFALSE

// ------------------------------------------------------------------------
//  Utility functions
// ------------------------------------------------------------------------

void Char_tswap(Char_t* a, Char_t* b);
void flipInt_t(Int_t* toflip);
void flipFloat_t(Float_t* toflip);

// ------------------------------------------------------------------------
//  OCA Sub-classes
// ------------------------------------------------------------------------
// Class pmtinfo - Contains all PMT-level information (distances in D2O,
//                 Acrylic, H2O, expected time T0, and predicted response)
//                 for a specific PMT

class pmtinfo {
 public:
  Int_t PMTStat;
  Float_t D2ODist, AcrDist, H2ODist;
  Float_t D2ODistdx, D2ODistdy,D2ODistdz;
  Float_t AcrDistdx, AcrDistdy, AcrDistdz;
  Float_t H2ODistdx, H2ODistdy, H2ODistdz;
  Float_t T0;
  Float_t Response;
  Float_t Occupancy;

  pmtinfo();
  Int_t ReadFile(FILE* infile);
  void FlipBytes(void);
  void CopyFrom(pmtinfo* pmtsrc);
};


// ------------------------------------------------------------------------
// Class ballposition - Header information for a specific laserball position

class ballposition {
 public:
  Int_t NFile, WaveLength, MaxPmts;
  Float_t Toff;
  Int_t Window;
  Float_t XYZ[3];
  pmtinfo* pmt;

  ballposition();
  ~ballposition();
  Int_t ReadFile(FILE* infile);
  void FlipBytes();
};


// ------------------------------------------------------------------------
// Class datafile - Data file header, especially Magic Number to tell if
//                  byte swap is necessary (depends on hardware bit ordering)

class datafile {
 public:
  Int_t MagicNumber,OptHeadSize,RecLen,RecNum;
  Int_t NumPositions,MaxPmts,Izero;
  ballposition* data;

  datafile();
  ~datafile();
  Int_t ReadFile(FILE* infile);
  void FlipBytes();
};


// ------------------------------------------------------------------------
//  OCA class definition
// ------------------------------------------------------------------------

class OCA {
 private:
  Bool_t Raw_OCA_Loaded;
  Bool_t OCA_Distances_Computed;
  Bool_t OCA_Atten_Computed;
  
 protected:
  Float_t XBall;
  Float_t YBall;
  Float_t ZBall;
  Float_t D2O_Atten;
  Float_t Acrylic_Atten;
  Float_t H2O_Atten;

  Float_t Distance_Tolerance_sq;

  Char_t Raw_OCA_Filename[255];

  // For raw data from the file
  datafile* OCA_Datafile;

  // For processed data at given coordinates
  pmtinfo* PMT_Array;
    
 public:
  OCA(Char_t* ocafname=NULL);
  ~OCA();

  Int_t Load_Raw_OCA(FILE *fp);
  Int_t Load_Raw_OCA(Char_t* ocafname);
  Int_t Load_Raw_OCA();
  Int_t Unload_Raw_OCA();

  Int_t Compute_OCA();
  Int_t Compute_OCA(Float_t D2OA,Float_t AcrA,Float_t H2OA);
  Int_t Compute_OCA(Float_t xb, Float_t yb, Float_t zb,
		    Float_t D2OA,Float_t AcrA,Float_t H2OA);

  Int_t Get_Status(Int_t PMT_Index);
  Float_t Get_Occupancy(Int_t PMT_Index);
  Float_t Get_T0(Int_t PMT_Index);

  void Set_Raw_OCA_Filename(Char_t* ocafname);

  void Set_Ball_Coordinates(Float_t xb,Float_t yb,Float_t zb);
  void Set_Atten(Float_t D2OA, Float_t AcrA, Float_t H2OA);
  void Set_D2O_Atten(Float_t Atten);
  void Set_Acrylic_Atten(Float_t Atten);
  void Set_H2O_Atten(Float_t Atten);

  void Set_Distance_Tolerance(Float_t DistTol);
};


#endif
