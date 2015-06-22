//------------------------------------------------------------------------
// Header information from Richard Ford's dumping program,
//  oca_visual_calc.for.  This file outputs in binary from FORTRAN, and
//  here we need to read it in to be stored for use by C analysis
//  programs.
//
// oca_visual.bin currently only contains information for points along the
//  vertical detector axis:
//  x,y,z = 0, 0, 500 cm
//          0, 0, 250
//          0, 0,   0
//          0, 0,-250
//          0, 0,-500
// Limits on actual laserball position are not imposed, but this routine
//  takes the closest available point, and calculates all quantities based
//  on the derivative information at the "best" tabulated value.
//
// owl:~moffat/sno/oca/oca.cxx 
// Bryce Moffat, 27-10-98

#include "oca.h"

// ------------------------------------------------------------------------
//  Global variables
// ------------------------------------------------------------------------
Bool_t NeedToFlip = kFALSE;

// ------------------------------------------------------------------------
//  Utility functions
// ------------------------------------------------------------------------

void Char_tswap(Char_t* a, Char_t* b)
{  Char_t temp = *a;
  *a = *b;
  *b = temp;
}

// --------------------

void flipInt_t(Int_t* toflip) 
{
  const Int_t nbytes = sizeof(Int_t);
  Char_t* cflip = (Char_t*) toflip;
  for (Int_t i=0;i<nbytes/2;i++) Char_tswap(&cflip[i],&cflip[nbytes-i-1]);
}

// --------------------

void flipFloat_t(Float_t* toflip)
{
  const Int_t nbytes = sizeof(Float_t);
  Char_t* cflip = (Char_t*) toflip;
  for (Int_t i=0;i<nbytes/2;i++) Char_tswap(&cflip[i],&cflip[nbytes-i-1]);
}

// ------------------------------------------------------------------------
//  OCA Sub-classes
// ------------------------------------------------------------------------
// Class pmtinfo - Contains all PMT-level information (distances in D2O,
//                 Acrylic, H2O, expected time T0, and predicted response)
//                 for a specific PMT

pmtinfo::pmtinfo()
{
  PMTStat = -1;
  Response = 0;
}

// --------------------

Int_t pmtinfo::ReadFile(FILE* infile)
{
  Int_t head,tail;

  Int_t iretcode = fread((void *) &head, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &PMTStat, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &D2ODist, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &AcrDist, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &H2ODist, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &D2ODistdx, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &D2ODistdy, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &D2ODistdz, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &AcrDistdx, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &AcrDistdy, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &AcrDistdz, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &H2ODistdx, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &H2ODistdy, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &H2ODistdz, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &T0, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &Response, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &tail, sizeof(Int_t), 1, infile);

  if (iretcode != 0) {
    if (NeedToFlip) FlipBytes();

    if (kDEBUG) {
    printf("* %8x %8d %10.4g %10.4g %10.4g ",head,PMTStat,
	   D2ODist,AcrDist,H2ODist);
    printf(" %10.4g %10.4g %8x \n",T0,Response,tail);
    }
  }

  return iretcode;
}

// --------------------

void pmtinfo::FlipBytes()
{
  flipInt_t(&PMTStat);
  flipFloat_t(&D2ODist);
  flipFloat_t(&AcrDist);
  flipFloat_t(&H2ODist);
  flipFloat_t(&D2ODistdx);
  flipFloat_t(&D2ODistdy);
  flipFloat_t(&D2ODistdz);
  flipFloat_t(&AcrDistdx);
  flipFloat_t(&AcrDistdy);
  flipFloat_t(&AcrDistdz);
  flipFloat_t(&H2ODistdx);
  flipFloat_t(&H2ODistdy);
  flipFloat_t(&H2ODistdz);
  flipFloat_t(&T0);
  flipFloat_t(&Response);
}

// --------------------

void pmtinfo::CopyFrom(pmtinfo* pmtsrc)
{
  PMTStat = pmtsrc->PMTStat;

  D2ODist = pmtsrc->D2ODist;
  AcrDist = pmtsrc->AcrDist;
  H2ODist = pmtsrc->H2ODist;

  D2ODistdx = pmtsrc->D2ODistdx;
  D2ODistdy = pmtsrc->D2ODistdy;
  D2ODistdz = pmtsrc->D2ODistdz;
  AcrDistdx = pmtsrc->AcrDistdx;
  AcrDistdy = pmtsrc->AcrDistdy;
  AcrDistdz = pmtsrc->AcrDistdz;
  H2ODistdx = pmtsrc->H2ODistdx;
  H2ODistdy = pmtsrc->H2ODistdy;
  H2ODistdz = pmtsrc->H2ODistdz;

  T0 = pmtsrc->T0;
  Response = pmtsrc->Response;
}

// ------------------------------------------------------------------------
// Class ballposition - Header information for a specific laserball position

ballposition::ballposition()
{
  NFile = -1;
  XYZ[0] = kPosOutOfRange;
  XYZ[1] = kPosOutOfRange;
  XYZ[2] = kPosOutOfRange;
  pmt = NULL;
}

// --------------------

ballposition::~ballposition()
{
  NFile = -1;
  XYZ[0] = kPosOutOfRange;
  XYZ[1] = kPosOutOfRange;
  XYZ[2] = kPosOutOfRange;
  if (pmt != NULL) delete[] pmt;
}

// --------------------

Int_t ballposition::ReadFile(FILE* infile)
{
  Int_t head,tail;
  
  Int_t iretcode = fread((void *) &head, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &NFile, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &WaveLength, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &MaxPmts, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &Toff, sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &Window, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &XYZ, 3*sizeof(Float_t), 1, infile);
  iretcode += fread((void *) &tail, sizeof(Int_t), 1, infile);
  
  if (iretcode != 0) {
    if (NeedToFlip) FlipBytes();

    if (kDEBUG) {
      printf("=== %8x %8d %8d %8d %10.4g %8d %10.4g %10.4g %10.4g %8x \n",head,
	     NFile,WaveLength,MaxPmts,Toff,Window,XYZ[0],XYZ[1],XYZ[2],tail);
    }

    pmt = new pmtinfo[kNCrate*kNCard*kNChannel];
    if (pmt == NULL) iretcode += 100;
    else
      for (Int_t i=0;i<kNCrate*kNCard*kNChannel;i++) pmt[i].ReadFile(infile);
  }

  return iretcode;
}

// --------------------

void ballposition::FlipBytes()
{
  flipInt_t(&NFile);
  flipInt_t(&WaveLength);
  flipInt_t(&MaxPmts);
  flipFloat_t(&Toff);
  flipInt_t(&Window);
  flipFloat_t(&XYZ[0]);
  flipFloat_t(&XYZ[1]);
  flipFloat_t(&XYZ[2]);
}

// ------------------------------------------------------------------------
// Class datafile - Data file header, especially Magic Number to tell if
//                  byte swap is necessary (depends on hardware bit ordering)

datafile::datafile()
{
  NumPositions = -1;
  data = NULL;
}

// --------------------

datafile::~datafile()
{
  NumPositions = -1;
  if (data != NULL) delete[] data;
}

// --------------------

Int_t datafile::ReadFile(FILE *infile)
{
  Int_t head,tail;

  // First, file integrity information
  Int_t iretcode = fread((void *) &head, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &MagicNumber, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &OptHeadSize, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &RecLen, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &RecNum, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &tail, sizeof(Int_t), 1, infile);

  // Read in information about amount of data in file
  iretcode += fread((void *) &head, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &NumPositions, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &OptHeadSize, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &MaxPmts, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &Izero, sizeof(Int_t), 1, infile);
  iretcode += fread((void *) &tail, sizeof(Int_t), 1, infile);

  if (iretcode != 0) {
    if (MagicNumber != (Int_t)kMagicNumber) {
      NeedToFlip = kTRUE;
      FlipBytes();
    }
    else NeedToFlip = kFALSE;

    if (kDEBUG) {
      printf("### %8x %8x %8d %8d %8d %8x \n",head,MagicNumber,OptHeadSize,
	     RecLen, RecNum,tail);
      printf("--- %8x %8d %8d %8d %8d %8x \n",head,NumPositions,OptHeadSize,
	     MaxPmts, Izero,tail);
    }
    
    data = new ballposition[NumPositions];
    if (data == NULL) iretcode = -1;
    else for (Int_t i=0;i<NumPositions;i++) data[i].ReadFile(infile);
  }
  
  return iretcode;
}

// --------------------

void datafile::FlipBytes(void)
{
  flipInt_t(&MagicNumber);
  flipInt_t(&OptHeadSize);
  flipInt_t(&RecLen);
  flipInt_t(&RecNum);
  
  flipInt_t(&NumPositions);
  flipInt_t(&MaxPmts);
  flipInt_t(&Izero);
}

//------------------------------------------------------------------------
//  Class OCA - Optical calibration class


OCA::OCA(Char_t* ocafname)
{
  Raw_OCA_Loaded = kFALSE;
  OCA_Distances_Computed = kFALSE;
  OCA_Atten_Computed = kFALSE;

  Set_Distance_Tolerance(-1);
  if (ocafname) strcpy(Raw_OCA_Filename,ocafname);

  OCA_Datafile = NULL;
  PMT_Array = new pmtinfo[kNCrate*kNCard*kNChannel];
}  

// ------------------------------------------------------------------------

OCA::~OCA()
  // OCA destructor
{
  Unload_Raw_OCA();
  delete OCA_Datafile;
  delete[] PMT_Array;
}

// ------------------------------------------------------------------------

Int_t OCA::Load_Raw_OCA(FILE *fp)
{
  Unload_Raw_OCA();

  // create new datafile (was deleted by unloading)
  OCA_Datafile = new datafile;
  if (!OCA_Datafile) return(1);

  if (!fp) return(-1);
  
  if (!OCA_Datafile->ReadFile(fp)) {
    return 1;
  } else {
    Raw_OCA_Loaded = kTRUE;
    return 0;
  }
}

// --------------------

Int_t OCA::Load_Raw_OCA(void)
{
  FILE *fp = fopen(Raw_OCA_Filename,"r");
  
  if ( !fp ) {
    printf(" oca: Failure to open input file : \"%s\"\n",Raw_OCA_Filename);
    return -1;
  }
  
  Int_t rtnVal = Load_Raw_OCA(fp);
  
  fclose(fp);
  
  return rtnVal;
}

// --------------------

Int_t OCA::Load_Raw_OCA(Char_t* ocafname)
  // Set filename and load
{
  Set_Raw_OCA_Filename(ocafname);
  return Load_Raw_OCA();
}

// ------------------------------------------------------------------------

Int_t OCA::Unload_Raw_OCA()
  // Rid memory of Raw OCA data (ie. destroy OCA_Datafile)
{
  if (OCA_Datafile != NULL) {
  	delete OCA_Datafile;
  	OCA_Datafile = NULL;
  }
  Raw_OCA_Loaded = kFALSE;
  return 0;
}

// ------------------------------------------------------------------------

Int_t OCA::Compute_OCA()
{
  Int_t i;
  pmtinfo *thePmt, *theLastPmt;
  
  if (!Raw_OCA_Loaded) return -1;

  if (!OCA_Distances_Computed) {
    // Find closest matching tabulated position within distance tolerance
    Float_t distsq;
    Float_t mindistsq = 1.e12;
    Int_t i_mindistsq = -1;
    
    for (i=0;i<OCA_Datafile->NumPositions;i++) {
      distsq = 
	pow(OCA_Datafile->data[i].XYZ[0] - XBall,(float)2.) +
	pow(OCA_Datafile->data[i].XYZ[1] - YBall,(float)2.) +
	pow(OCA_Datafile->data[i].XYZ[2] - ZBall,(float)2.);
      if (distsq < mindistsq) {
	mindistsq = distsq;
	i_mindistsq = i;
      }
    }
    
    if (mindistsq > Distance_Tolerance_sq) return 1;
    
    // Compute step away from tabulated value
    Float_t dx = OCA_Datafile->data[i_mindistsq].XYZ[0] - XBall;
    Float_t dy = OCA_Datafile->data[i_mindistsq].XYZ[1] - YBall;
    Float_t dz = OCA_Datafile->data[i_mindistsq].XYZ[2] - ZBall;
    
    // Compute new values and fill into table
    for (i=0;i<kNCrate*kNCard*kNChannel;i++) {
      thePmt = PMT_Array + i;
      thePmt->CopyFrom(&(OCA_Datafile->data[i_mindistsq].pmt[i]));
      Float_t dD2ODist = 
	thePmt->D2ODistdx * dx +
	thePmt->D2ODistdy * dy +
	thePmt->D2ODistdz * dz;
      thePmt->D2ODist = thePmt->D2ODist + dD2ODist;
      
      Float_t dAcrDist =
	thePmt->AcrDistdx * dx +
	thePmt->AcrDistdy * dy +
	thePmt->AcrDistdz * dz;
      thePmt->AcrDist = thePmt->AcrDist + dAcrDist;
      
      Float_t dH2ODist = 
	thePmt->H2ODistdx * dx +
	thePmt->H2ODistdy * dy +
	thePmt->H2ODistdz * dz;
        thePmt->H2ODist = thePmt->H2ODist + dH2ODist;
      
      thePmt->T0 += 
	(dD2ODist+dH2ODist)*1.33/30. + dAcrDist*1.5/30.;
    }
    OCA_Distances_Computed = kTRUE;
  }
  
  if (!OCA_Atten_Computed) {
    // Compute new values and fill into table
    thePmt = PMT_Array;
    theLastPmt = PMT_Array + kNCrate*kNCard*kNChannel;
    do {
      thePmt->Occupancy = thePmt->Response *
	exp(-D2O_Atten * thePmt->D2ODist
	    -Acrylic_Atten * thePmt->AcrDist
	    -H2O_Atten * thePmt->H2ODist);
    } while (++thePmt < theLastPmt);
    OCA_Atten_Computed = kTRUE;
  }
  return 0;
}

// --------------------

Int_t OCA::Compute_OCA(Float_t D2OA,Float_t AcrA,Float_t H2OA)
{
  Set_Atten(D2OA,AcrA,H2OA);

  return Compute_OCA();
}

// --------------------

Int_t OCA::Compute_OCA(Float_t xb, Float_t yb, Float_t zb,
		    Float_t D2OA,Float_t AcrA,Float_t H2OA)
{
  Set_Ball_Coordinates(xb,yb,zb);
  Set_Atten(D2OA,AcrA,H2OA);

  return Compute_OCA();
}

// ------------------------------------------------------------------------

Int_t OCA::Get_Status(Int_t PMT_Index)
{
  if (OCA_Distances_Computed) return PMT_Array[PMT_Index].PMTStat;
  else return -1;
}

// ------------------------------------------------------------------------

Float_t OCA::Get_Occupancy(Int_t PMT_Index)
{
  if (OCA_Atten_Computed) return PMT_Array[PMT_Index].Occupancy;
  else return -1;
}

// ------------------------------------------------------------------------

Float_t OCA::Get_T0(Int_t PMT_Index)
{
  if (OCA_Distances_Computed) return PMT_Array[PMT_Index].T0;
  else return -1;
}

// ------------------------------------------------------------------------

void OCA::Set_Raw_OCA_Filename(Char_t* ocafname)
{
  strcpy(Raw_OCA_Filename,ocafname);
}

// ------------------------------------------------------------------------

void OCA::Set_Ball_Coordinates(Float_t xb,Float_t yb,Float_t zb)
{
  XBall = xb;
  YBall = yb;
  ZBall = zb;

  OCA_Distances_Computed = kFALSE;
  OCA_Atten_Computed = kFALSE;
}

//------------------------------------------------------------------------

void OCA::Set_Atten(Float_t D2OA,Float_t AcrA,Float_t H2OA)
{
  D2O_Atten = D2OA;
  Acrylic_Atten = AcrA;
  H2O_Atten = H2OA;

  OCA_Atten_Computed = kFALSE;
}

//------------------------------------------------------------------------

void OCA::Set_D2O_Atten(Float_t Atten)
{
  D2O_Atten = Atten;
  OCA_Atten_Computed = kFALSE;
}

//------------------------------------------------------------------------

void OCA::Set_Acrylic_Atten(Float_t Atten)
{
  Acrylic_Atten = Atten;
  OCA_Atten_Computed = kFALSE;
}

//------------------------------------------------------------------------

void OCA::Set_H2O_Atten(Float_t Atten)
{
  H2O_Atten = Atten;
  OCA_Atten_Computed = kFALSE;
}

//------------------------------------------------------------------------

void OCA::Set_Distance_Tolerance(Float_t DistTol)
  // Use negative argument to reset to default
{
  if (DistTol >= 0) Distance_Tolerance_sq = DistTol*DistTol;
  else Distance_Tolerance_sq = kDistTol*kDistTol;
}
