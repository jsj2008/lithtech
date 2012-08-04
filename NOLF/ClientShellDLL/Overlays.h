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
	OVM_SUNGLASS = 0,
	OVM_SCOPE,
	OVM_SCUBA,
	OVM_SPACE,

	//non-exclusive overlays
	OVM_NON_EXCLUSIVE,
	OVM_STATIC = OVM_NON_EXCLUSIVE,
	OVM_CAMERA,
	OVM_ZOOM_IN,
	OVM_ZOOM_OUT,

	NUM_OVERLAY_MASKS
};

enum eSunglassMode
{
	SUN_NONE	= -1,
	SUN_CAMERA	= 0,
	SUN_MINES,
	SUN_IR,
	SUN_INVIS_INK,
	NUM_SUN_MODES
};


#endif  // __OVERLAYS_H__