 // ----------------------------------------------------------------------- //
//
// MODULE  : VolumetricLightFX.h
//
// PURPOSE : Client side implementation of the volumetric lighting effect
//			 associated with LightSpot objects
//
// CREATED : 9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUMETRICLIGHTFX_H__
#define __VOLUMETRICLIGHTFX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"
#include "ILTCustomRenderCallback.h"

// The maximum number of volumetric lights that can be created within a level
#define MAX_VOLUMETRICLIGHTFX_OBJECTS	256

class ILTCustomRenderCallback;

class CVolumetricLightFX : 
	public CSpecialFX
{
public :

	CVolumetricLightFX();
	~CVolumetricLightFX();

    virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
    virtual bool Update();
    virtual bool CreateObject(ILTClient* pClientDE);

	virtual uint32 GetSFXID() { return SFX_VOLUMETRICLIGHT_ID; }

private :
	// Callback for rendering slices
	static void RenderSlicesCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
	{
		((CVolumetricLightFX*)pUser)->RenderSlices(pInterface, tCamera);
	}
	void RenderSlices(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	// Callback for rendering the shell
	static void RenderShellCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
	{
		((CVolumetricLightFX*)pUser)->RenderShell(pInterface, tCamera);
	}
	void RenderShell(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	// Get the world-relative vertices representing the corners of the light's frustum
	void GetLightFrustumVertices(LTVector* pVertices, uint32 nStride);
	
	// Let go of our render targets
	void ReleaseRenderTargets();
	
	// Update our cached transforms and other information relating to the position
	// and other properties of the light
	void UpdateLightProperties();
	
	// Let go of our material references
	void ReleaseMaterials();
	
	// Update the materials to point to the most accurate material representation
	void UpdateMaterials();
	
private:
	// Our CreateStruct that was provided at object creation
	VOLUMETRICLIGHTCREATESTRUCT m_CS;
	LTObjRef	m_hSliceObject, m_hShellObject;
	
	// The perspective transform associated with the light's FOV and clipping range
	LTMatrix	m_mLightPerspectiveTransform;
	
	// The basic slice material that the effect should use (which is used for determining
	// when the material has changed due to a console variable change, indicating that
	// a new clone must be created
	HMATERIAL	m_hBaseSliceMaterial;
	
	// The materials (cloned) used for slice and shell rendering
	HMATERIAL	m_hSliceMaterial, m_hShellMaterial;
	
	// Rendering status flags for tracking when things are drawn
	bool		m_bShellWasDrawn, m_bSliceWasDrawn, m_bShellWasDrawnLastFrame;
	
	// Maintenance for indicating when we are associated with the resource manager and the render targets
	bool		m_bMgrRef, m_bRenderTargetRef;
	
	// Caching variables for the current light transform, FOV, and clipping parameters
	LTRigidTransform	m_tCurTransform;
	LTVector2			m_vCurFOV, m_vCurClip;
	bool				m_bDirectional;
	
};

#endif // __VOLUMETRICLIGHTFX_H__
