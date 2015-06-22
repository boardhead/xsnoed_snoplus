/*
** colours.h	- XSNOED colour definitions
**
** The basic XSNOED colours are defined in the enumerated type EColours below.
**
** The hit and vessel colour numbers are special, since they both use variable
** length colour tables generated at run time.  The hit colours begin with
** colour number NUM_COLOURS, followed by the vessel colours.
**
** When a new colour is added, a corresponding resource should be added to
** PResourceManager.cxx, and a new sColourName should be added to PColourPicker.cxx.
*/
#ifndef __COLOURS_H__
#define __COLOURS_H__

enum EColour {
	BKG_COL,
	TEXT_COL,
	HID_COL,
	FRAME_COL,
	VDARK_COL,
	VLIT_COL,
	AXES_COL,
	CURCON_COL,
	RCON_COL,
	SCALE_UNDER,
	SCALE_COL0,
	SCALE_COL1,
	SCALE_COL2,
	SCALE_COL3,
	SCALE_COL4,
	SCALE_OVER,
	DISCARDED_COL,
	GRID_COL,
	SUN_COL,
	WATER_COL,
	NCD_COL,
	SCOPE0_COL,
	SCOPE1_COL,
	MC_NEUTRINO_COL,
	MC_ELECTRON_COL,
	MC_NEUTRON_COL,
	MC_GAMMA_COL,
	MC_PHOTON_COL,
	MC_SPECIAL_COL,
	MC_OTHER_COL,
	NUM_COLOURS
};

#endif // __COLOURS_H__
