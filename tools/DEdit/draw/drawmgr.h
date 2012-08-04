//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// This defines the abstract editor drawing interface.
// It is implemented with the editor's (default) software renderer
// and with a d3d renderer.  There is one drawmgr for each window.

#ifndef __DRAWMGR_H__
#define __DRAWMGR_H__


	#include "edithelpers.h"
	#include "3d_ops.h"
	#include "optionsdisplay.h"
	
	class CRegionView;
	class CPreMainWorld;
	class CPrePoly;

	#define RM_HARDWARE	0
	#define RM_MMX		1
	#define RM_RAMP		2
	#define RM_RGB		3

	class DrawMgr
	{
	public:
		
		virtual			~DrawMgr() {}

		virtual void	Draw() {}
		virtual void	Resize(int x, int y) {}
		virtual void	Term() {}		
	};

	

	// deviceNum enumerates the available devices.
	// renderMode is one of the defines above.
	// If successful, deviceNum and renderMode are filled in.
	DrawMgr* dm_CreateDirect3dDrawMgr(CRegionView *pView, int *deviceNum, int *renderMode);

	// Gets DirectDraw device names..  Returns number of names filled in.
	int dm_GetDirectDrawDeviceNames(char **pFillIn, int nMaxNames);

	// Tells the Direct3d manager to unbind from the given binding.
	// (implemented in d3dtexture.h).
	void d3d_UnbindFromTexture(void *pBinding);

	//this function will, given the desired size for a texture, find the appropriate
	//size that should be used
	void d3d_FindTextureSize(uint32 nDesiredX, uint32 nDesiredY, uint32& nOutX, uint32& nOutY);


#endif  // __DRAWMGR_H__



