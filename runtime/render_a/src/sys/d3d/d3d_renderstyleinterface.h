//-------------------------------------------------------------------
//
//   MODULE    : D3D_RENDERSTYLEINTERFACE.H
//
//   PURPOSE   : The D3D Impl of the ILTRenderStyles inteface.
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------

#ifndef __D3D_RENDERSTYLEINTERFACE_H_
#define __D3D_RENDERSTYLEINTERFACE_H_

#include "iltrenderstyles.h"

class D3DRenderStyles : public ILTRenderStyles
{
public:
	declare_interface(D3DRenderStyles);

	D3DRenderStyles()								{ };
	virtual ~D3DRenderStyles()						{ };

	//******** Core Member Functions *********//
	virtual CRenderStyle*			DuplicateRenderStyle(CRenderStyle* pRendStyle);				// Duplicate a render style (maybe you want to change it just for a certain object - you better dup it first, then change it, and set it).
	virtual CRenderStyle*			CreateRenderStyle(bool bSetToDefault = true);				// Create a render style. You can then set all it's internals yourself.
	virtual CRenderStyle*			LoadRenderStyle(const char* szFilename);					// Load a render style (ltb file) and create a render object for it. 
	virtual void					FreeRenderStyle(CRenderStyle* pRendStyle);					// Free render style - you no longer need it (may or may not be internally freed - depending on the ref count).

	//******** Functions called by CD3DRenderStyle
	virtual void					OnDelete(CD3DRenderStyle* pRendStyle);						// Notification from the render style that it is being deleted, so the interface should clean itself up
};

#endif // __D3D_RENDERSTYLEINTERFACE_H_

