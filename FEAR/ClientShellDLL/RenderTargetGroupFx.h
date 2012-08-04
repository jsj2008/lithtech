// ----------------------------------------------------------------------- //
//
// MODULE  : RenderTargetGroupFX.h
//
// PURPOSE : Client side implementation of the RenderTargetGroup object which is
//			 used to provide a surface to the other render target objects
//
// CREATED : 5/11/04
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __RENDERTARGETGROUPFX_H__
#define __RENDERTARGETGROUPFX_H__

#ifndef __SPECIAL_FX_H__
#	include "SpecialFX.h"
#endif

#ifndef __ILTRENDERER_H__
#	include "iltrenderer.h"
#endif

//the maximum number of render targets that can be created within a level
#define MAX_RENDERTARGETGROUP_OBJECTS	4

//------------------------------------------------------------------------------
//the creation structure that is read from the server side object and used for
//the creation of the client side object
struct RENDERTARGETGROUPCREATESTRUCT : 
	public SFXCREATESTRUCT
{
	RENDERTARGETGROUPCREATESTRUCT()
	{
		for(uint32 nCurrDim = 0; nCurrDim < LTARRAYSIZE(m_nDimensions); nCurrDim++)
			m_nDimensions[nCurrDim].Init(128, 128);

		m_nUniqueGroupID	= 0;
		m_bCubeMap			= false;
		m_bMipMap			= true;
		m_bFogVolumes		= false;
		m_bLastFrameEffects = false;
		m_bCurrFrameEffects = false;
	}

	//the number of frames to wait before rendering the render target again
	uint32		m_nUniqueGroupID;

	//render target dimensions, for each LOD setting
	LTVector2n	m_nDimensions[3];

	//flag indicating whether or not this is a cubic render target
	bool		m_bCubeMap;

	//flags indicating which rendering features are supported
	bool		m_bMipMap;
	bool		m_bFogVolumes;
	bool		m_bLastFrameEffects;
	bool		m_bCurrFrameEffects;
};

//------------------------------------------------------------------------------
// The actual game side object that handles the render target and updating it
class CRenderTargetGroupFX : 
	public CSpecialFX
{
public:

	CRenderTargetGroupFX();
	~CRenderTargetGroupFX();

	virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
	virtual bool Update();
	virtual bool CreateObject(ILTClient* pClientDE);

	//get the ID of this special effect object, used for identifying object types dynamically
	virtual uint32	GetSFXID()					{ return SFX_RENDERTARGETGROUP_ID; }

	//called to access the unique group ID of this render target group
	uint32			GetUniqueGroupID() const	{ return m_nUniqueGroupID; }

	//called to access properties of this render target
	bool			IsCubeMap() const			{ return m_bCubeMap; }
	HRENDERTARGET	GetRenderTarget()			{ return m_hRenderTarget; }

	//called to get the current render target LOD. This is zero for low detail, one for medium, and
	//two for high
	uint32			GetRenderTargetLOD() const;

	//called to determine if this render target is enabled at this LOD
	bool			IsEnabledAtLOD(uint32 nLOD);

private:

	//this handles creation of the render target
	bool		CreateRenderTarget(uint32 nLOD);

	//this will release the associated render target
	bool		ReleaseRenderTarget();

	//the LOD setting that we have created this render target at
	uint32		m_nCreatedLOD;

	//our render target that we will be displaying
	HRENDERTARGET	m_hRenderTarget;

	//the unique ID of this group
	uint32		m_nUniqueGroupID;

	//render target dimensions
	LTVector2n	m_nDimensions[3];

	//flag indicating whether or not this is a cubic render target
	bool		m_bCubeMap;

	//flags indicating which rendering features are supported
	bool		m_bMipMap;
	bool		m_bFogVolumes;
	bool		m_bLastFrameEffects;
	bool		m_bCurrFrameEffects;
};

#endif // __RENDERTARGETGROUPFX_H__
