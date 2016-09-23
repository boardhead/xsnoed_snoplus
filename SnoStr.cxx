//
// File:		SnoStr.cxx - SNO String definitions and utilities
//
// Created:		01/17/00 - Phil Harvey
//				09/05/01 - PH Changed run type bit "DCR Activity" to "UC"
//
#include <string.h>
#include "SnoStr.h"


// XSNOED MTC word trigger bit strings
// - word defined as: (mtc_word[3] & 0xff000000UL) | (mtc_word[4] & 0x0007ffffUL)
char *SnoStr::sXsnoedTrig[32] = {
#ifdef SNOPLUS
	"OWLEL", 			"OWLEH", 			"PULGT", 			"PRESCL", 
	"PED", 				"PONG", 			"SYNC",				"EXTA",
	"HYDRO",			"EXT3",				"EXT4",				"EXT5",
	"EXT6",				"EXT7",			    "EXT8",				"SRAW",
	"NCDMUX",			"SOFGT",			"MISS",				"ORPHAN",
	"NONE",				"CAEN",		        "u22",			    "u23",
	"100L",				"100M",				"100H",				"20",
	"20LB",				"ESUML",			"ESUMH",			"OWLN"
#else
	"OWLEL", 			"OWLEH", 			"PULGT", 			"PRESCL", 
	"PED", 				"PONG", 			"SYNC",				"EXTA",
	"HYDRO",			"EXT3",				"EXT4",				"EXT5",
	"EXT6",				"EXT7", 			"EXT8",				"SRAW",
	"NCDMUX",			"SOFGT",			"MISS",				"ORPHAN",
	"NONE",				"CAEN",		        "u22",			    "u23",
	"100L",				"100M",				"100H",				"20",
	"20LB",				"ESUML",			"ESUMH",			"OWLN"
#endif
};
							
// TRIG bank 'TriggerMask' strings
char *SnoStr::sTrigMask[32] = { 
#ifdef SNOPLUS
	"100L",				"100M",				"100H",				"20",
	"20LB",				"ESUML",			"ESUMH",			"OWLN",
 	"OWLEL",			"OWLEH",			"PULGT",			"PRESCL",
 	"PED",				"PONG",				"SYNC",				"EXTA",
 	"HYDRO",			"EXT3",				"EXT4",				"EXT5",
 	"EXT6",				"EXT7",			    "EXT8",				"SRAW",
 	"NCD",  			"SOFGT",			"<26>",				"<27>",
 	"<28>",				"<29>",				"<30>",				"<31>"
#else
	"100L",				"100M",				"100H",				"20",
	"20LB",				"ESUML",			"ESUMH",			"OWLN",
 	"OWLEL",			"OWLEH",			"PULGT",			"PRESCL",
 	"PED",				"PONG",				"SYNC",				"EXTA",
 	"HYDRO",			"EXT3",				"EXT4",				"EXT5",
 	"EXT6",				"NCDSHAP",			"EXT8",				"SRAW",
 	"NCD",			    "SOFGT",			"<26>",				"<27>",
 	"<28>",				"<29>",				"<30>",				"<31>"
#endif
};

// RHDR bank 'RunMask' strings
char *SnoStr::sRunType[32] = {
#ifdef SNOPLUS
	"Mainenance",		"Transition",		"Physics",			"Int Source",
	"Ext Source",		"ECA",			    "Diagnostic",		"Experimental",
	"Supernova",		"<09>",				"<10>",				"TELLIE",
	"SMELLIE",		    "AMELLIE",			"PCA",				"ECA PED",
	"ECA TSlope",		"<17>",			    "<18>",				"<19>",
	"<20>",		    	"DCR Activity",		"Coils Off",		"PMT Off",
	"Bubbler",			"Recirc",			"SL Assay",			"Unusual Activity",
 	"<28>",				"<29>",				"<30>",				"<31>"
#else
	"Neutrino",			"Source",			"Calib",			"NCD",
	"Salt",				"Poison",			"Partial",			"Air",
	"D2O",				"H2O",				"UC",				"Trans",
	"Src Moving",		"Coils",			"ECA",				"Diag",
	"Supernova",		"Maint",			"PCA",				"Expt",
	"D2O Circ",			"Bubbler",			"SL Assay",			"NCD Maint",
	"PMT Off",			"NCD Off",			"NCD ECA",			"<27>",
 	"<28>",				"<29>",				"<30>",				"<31>"
#endif
};

// RHDR bank 'SourceMask' strings
char *SnoStr::sSourceMask[32] = {
	"Rotating",			"Laser",			"Sonoball",			"N16",
	"N17",				"NaI",				"Li8",				"P-T",
	"CF High",			"CF Low",			"U",				"TH",
	"P-Li7",			"Water Sampler",	"Prop. Counter",	"Single NCD",
	"Self Calib.",		"Spare1",   		"Cherenkov Low",	"Radon",
	"<20>",				"<21>",				"AmBe",				"<23>",
	"<24>",				"<25>",				"<26>",				"<27>",
 	"<28>",				"<29>",				"<30>",				"<31>"
};

// CAST bank 'status' strings
char *SnoStr::sManipStatus[kNumManipStatus] = {
	"<unknown>",		"Stopped",			"Low Tension",		"High Tension",	
	"At Endpoint",		"Stuck",			"Net Force",		"Axis Error",		
	"Moving Direct",	"Moving Idle",		"Aborting Move",	"Moving Umbil",
	"Moving Tension",	"Calc Error",		"Axis Flag",		"Axis Alarm"
};

// CAST bank source orientations
char *SnoStr::sSourceOrientation[kNumSourceOrientations] = {
    "<unknown>",        "North",            "East",             "South",
    "West"
};

// EPED bank 'CalibrationType' strings
char *SnoStr::sEPEDType[kNumEPEDTypes] = { "<unknown>", "QSLOPE", "TSLOPE", "PED" };

// EPED bank 'Flag' strings
char *SnoStr::sEPEDFlag[kNumEPEDFlags] = { "<unknown>", "STRT", "CHNG", "STOP", "END" };

// lookup table for SNOMAN particle codes
// (maximum of 32 particle types since they are mapped into the bits of an unsigned long)
LookupStruct SnoStr::sParticleLookup[] = {
				{ 0,	"<unknown>" },
				{ 1, 	"Photon" },
				{ 2, 	"Gamma" },
				{ 20,	"Electron" },
				{ 21,	"Positron" },
				{ 22,	"Mu-" },
				{ 23,	"Mu+" },
				{ 24,	"Tau-" },
				{ 25,	"Tau+" },
				{ 30,	"Nu-e" },
				{ 31,	"Nu-e-bar" },
				{ 32,	"Nu-mu" },
				{ 33,	"Nu-mu-bar" },
				{ 34,	"Nu-tau" },
				{ 35,	"Nu-tau-bar" },
				{ 40,	"Pi0" },
				{ 41,	"Pi+" },
				{ 42,	"Pi-" },
				{ 50,	"K0" },
				{ 51,	"K0-bar" },
				{ 52,	"K+" },
				{ 53,	"K-" },
				{ 80,	"Proton" },
				{ 81,	"Neutron" }
};

// lookup table for SNOMAN interaction codes
LookupStruct SnoStr::sInteractionLookup[] = {
				{ 100, "Start" },
				{ 101, "Beta decay" },
				{ 103, "Beta gamma decay" },
				{ 104, "Alpha decay" },
				{ 110, "Electron scattering" },
				{ 111, "Neutral current" },
				{ 112, "Charged current" },
				{ 120, "Fission" },
				{ 198, "Pick particle list" },
				{ 199, "Photon bomb" },
				// photon interactions
				{ 200, "Null boundary" },
				{ 201, "Spectral reflection" },
				{ 202, "Diffuse reflection" },
				{ 203, "Refraction" },
				{ 204, "Diffuse transmission" },
				{ 301, "Created" },
				{ 302, "Created in PMT" },
				{ 303, "Rayleigh scatter" },
				{ 304, "Escape" },
				{ 305, "Bounce" },
				{ 310, "Photodisintigration" },
				// neutron interactions
				{ 320, "Neutron interaction" },
				{ 321, "N elastic scatter" },
				{ 322, "N inelastic (n,2n) scatter" },
				{ 323, "N inelastic (n,3n) scatter" },
				{ 324, "N inelastic (n,n alpha) scatter" },
				{ 325, "N inelastic (n,n) excited" },
				{ 326, "Generic neutron capture" },
				{ 327, "N absorbed - gamma emisson" },
				{ 328, "N absorbed - proton emisson" },
				{ 329, "N absorbed - deuteron emisson" },
				{ 330, "N absorbed - triton emisson" },
				{ 331, "N absorbed - 3He emission" },
				{ 332, "N absorbed - alpha emission" },
				{ 339, "Killed by debug option" },
				// muon interactions
				{ 340, "Muon physics" },
				// hadron interactions
				{ 360, "Hadron physics" },
				// EGS codes
				{ 380, "Standard step" },
				{ 381, "Discarded (ECUT/PCUT)" },
				{ 382, "Discarded (AE/AP)" },
				{ 383, "Discarded (geometry)" },
				{ 384, "Discarded (photoelectric)" },
				{ 385, "Bremstrahlung" },
				{ 386, "Moller scatter" },
				{ 387, "Bhabha scatter" },
				{ 388, "Annihilation in flight" },
				{ 389, "Annihilation at rest" },
				{ 390, "Pair production" },
				{ 391, "Compton scatter" },
				{ 392, "Photoelectric effect" },
				{ 393, "Rayleigh scatter" },
				{ 397, "Photon bundle" },
				{ 398, "Shower node" },
				{ 399, "EGS4 vertex" },
				{ 400, "Particle lost" },
				{ 401, "Discarded (CERFAC)" },
				{ 402, "Reached PMT" },
				{ 403, "Obsolete" },
				{ 404, "Absorbed" },
				// neutrino interactions
				{ 510, "Solar neutrino" },
				{ 530, "Atmospheric neutrino" },
				{ 550, "Supernova neutrino" }
};

//---------------------------------------------------------------------------------------------

// GetList - get a comma-delineated list of strings from bits set in 'mask'
// - returns length of string
int SnoStr::GetList(char *outbuf, char **strs, u_int32 mask)
{
	int len = 0;
	outbuf[0] = '\0';
	for (int i=0; i<32; ++i) {
		if (mask & (1UL << i)) {
			if (len) {
				strcpy(outbuf+len,",");
				++len;
			}
			strcpy(outbuf+len,strs[i]);
			len += strlen(strs[i]);
		}
	}
	return(len);
}

/* do a binary search in an ordered lookup table */
int SnoStr::GetIndex(int code, LookupStruct *lookup, int num)
{
	int n1 = 0;
	int n2 = num - 1;
	while (n2 > n1) {
		int j = (n1 + n2) / 2;
		if (code < lookup[j].code) {
			n2 = j - 1;		// code is lower
		} else if (code > lookup[j].code) {
			n1 = j + 1;		// code is higher
		} else {
			n1 = n2 = j;	// found it
			break;
		}
	}
	if (code == lookup[n1].code) {
		return(n1);
	} else {
		return(-1);
	}
}

/* return name of an item in a lookup table */
char * SnoStr::Lookup(int code, LookupStruct *lookup, int num)
{
	int	index = GetIndex(code, lookup, num);
	
	if (index >= 0) return(lookup[index].name);
	else			return("<unknown>");
}

