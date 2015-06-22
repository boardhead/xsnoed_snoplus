// ScopeData.h
// by Paul Vaillancourt

#ifndef __SCOPEDATA_H__
#define __SCOPEDATA_H__

// For WAVEFORMSIZE define
//#include "QWaveform.h"

#define WAVEFORMSIZE  2500

#define SCOPEMAXCHANNELS 2

#define SCOPE_DATA_SIZE 7604    // size of packed scope data structure - PH 10/16/03

// CURRENT STRUCT
typedef struct ScopeData{

   // acquire
   char   acquireStopafter[32];
   int    acquireState;
   char   acquireMode[32];
   int    acquireNumenv;
   int    acquireNumavg;

   // channel
   double voltSteps[256];
   int    numVoltSteps;
   double channelScale[SCOPEMAXCHANNELS];
   double channelPosition[SCOPEMAXCHANNELS];
   double channelOffset[SCOPEMAXCHANNELS];
   char   channelCoupling[SCOPEMAXCHANNELS][32];
   char   channelBandwidth[SCOPEMAXCHANNELS][32];

   // time
   double timeScale;

   // trigger
   char   triggerMode[32];
   char   triggerType[32];
   double triggerLevel;
   double triggerHoldoff;
   char   triggerSource[32];
   char   triggerCoupling[32];
   int    triggerSlope;
   char   triggerSlopeString[32];
   
   int   channelFlag[SCOPEMAXCHANNELS];
   int   activeChannels[SCOPEMAXCHANNELS];
   int   numberOfActiveChannels;

   
   // === Waveform data ===

   float vDiv[SCOPEMAXCHANNELS]; // volts/div
   float tDiv[SCOPEMAXCHANNELS]; // time/div

   int   size[SCOPEMAXCHANNELS];   // size of waveform
   char  wave[SCOPEMAXCHANNELS][WAVEFORMSIZE]; // waveform array

   // conversion constants
   int   npoints[SCOPEMAXCHANNELS];
   float yoff[SCOPEMAXCHANNELS];
   float ymult[SCOPEMAXCHANNELS];
   float xincr[SCOPEMAXCHANNELS];
   int   pointoff[SCOPEMAXCHANNELS];

   int   nvchperdiv[SCOPEMAXCHANNELS];  // number of voltage channels per div
   int   ntchperdiv[SCOPEMAXCHANNELS];  // number of time channels per div

   int   channel[SCOPEMAXCHANNELS];
   int   eventNumber;

} ScopeData;


#endif // NOT __SCOPEDATA_H__
