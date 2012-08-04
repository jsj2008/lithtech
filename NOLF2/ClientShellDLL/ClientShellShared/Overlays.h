// ----------------------------------------------------------------------- //
//
// MODULE  : Overlays.h
//
// PURPOSE : definitions for overlays
//
// CREATED : 1/28/00
//
// ----------------------------------------------------------------------- //

#ifndef __OVERLAYS_H__
#define __OVERLAYS_H__

// the order of the eOverlayMask enum determines drawing priority, only the first
// exclusive overlay is drawn, while all active non-exclusive overlays are drawn
enum eOverlayMask
{
	OVM_NONE  = -1,

	//exclusive overlays
	OVM_SCOPE = 0,
	OVM_BINOC,
	OVM_SS_MASK,

	//non-exclusive overlays
	OVM_NON_EXCLUSIVE,
	OVM_STATIC = OVM_NON_EXCLUSIVE,
	OVM_CAMERA,
	OVM_CAMERA_TARGET,
	OVM_ZOOM_IN,
	OVM_ZOOM_OUT,

	//cinematic overlays
	OVM_CINEMATIC_1,
	OVM_CINEMATIC_2,
	OVM_CINEMATIC_3,
	OVM_CINEMATIC_4,
	OVM_CINEMATIC_5,
	OVM_CINEMATIC_6,
	OVM_CINEMATIC_7,
	OVM_CINEMATIC_8,
	OVM_CINEMATIC_9,

	NUM_OVERLAY_MASKS
};


#endif  // __OVERLAYS_H__