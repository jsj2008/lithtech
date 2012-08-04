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

#include "stdafx.h"
#include "RenderTargetGroupFX.h"
#include "iltrenderer.h"
#include "PlayerCamera.h"

//the global level of detail that should be used for render targets. 0 = low, 1 = medium, 2 = high
VarTrack g_vtRenderTargetLOD;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetGroupFX::~CRenderTargetGroupFX
//
//	PURPOSE:	Standard constructor
//
// ----------------------------------------------------------------------- //

CRenderTargetGroupFX::CRenderTargetGroupFX() :
	CSpecialFX()
{
	m_nCreatedLOD		= 0;
	m_nUniqueGroupID	= 0;
	m_hRenderTarget		= NULL;
	m_bCubeMap			= false;
	m_bMipMap			= true;
	m_bFogVolumes		= false;
	m_bLastFrameEffects	= false;
	m_bCurrFrameEffects	= false;

	for(uint32 nCurrDims = 0; nCurrDims < LTARRAYSIZE(m_nDimensions); nCurrDims++)
		m_nDimensions[nCurrDims].Init(128, 128);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetGroupFX::~CRenderTargetGroupFX
//
//	PURPOSE:	Standard destructor
//
// ----------------------------------------------------------------------- //

CRenderTargetGroupFX::~CRenderTargetGroupFX()
{
	//release our render target
	ReleaseRenderTarget();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetGroupFX::Init
//
//	PURPOSE:	Create the render target
//
// ----------------------------------------------------------------------- //

bool CRenderTargetGroupFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	//validate the parameters
    if (!psfxCreateStruct) 
		return false;

	//initialize the base object
	if(!CSpecialFX::Init(psfxCreateStruct))
		return false;

	//convert this to our own creation structure
	RENDERTARGETGROUPCREATESTRUCT* cs = (RENDERTARGETGROUPCREATESTRUCT*)psfxCreateStruct;

	//extract all of the relevant data from the creation structure provided
	m_nUniqueGroupID	= cs->m_nUniqueGroupID;
	m_bCubeMap			= cs->m_bCubeMap;
	m_bMipMap			= cs->m_bMipMap;
	m_bFogVolumes		= cs->m_bFogVolumes;
	m_bLastFrameEffects	= cs->m_bLastFrameEffects;
	m_bCurrFrameEffects	= cs->m_bCurrFrameEffects;

	for(uint32 nCurrDim = 0; nCurrDim < LTARRAYSIZE(m_nDimensions); nCurrDim++)
		m_nDimensions[nCurrDim] = cs->m_nDimensions[nCurrDim];

	//and now create our render target
	if(!CreateRenderTarget(GetRenderTargetLOD()))
		return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetGroupFX::CreateObject
//
//	PURPOSE:	Create object associated the poly grid
//
// ----------------------------------------------------------------------- //

bool CRenderTargetGroupFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE))
		return false;

	//we don't actually need to create any sort of object since we just use the server
	//object's position and orientation

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetRenderTargetLOD::CreateObject
//
//	PURPOSE:	called to get the current render target LOD. This is zero for 
//				low detail, one for medium, and two for high
//
// ----------------------------------------------------------------------- //
uint32 CRenderTargetGroupFX::GetRenderTargetLOD() const
{
	//make sure that our variable is initialized
	if(!g_vtRenderTargetLOD.IsInitted())
		g_vtRenderTargetLOD.Init(g_pLTBase, "RenderTargetLOD", NULL, 2.0f);

	return LTCLAMP((uint32)(g_vtRenderTargetLOD.GetFloat() + 0.5f), 0, 2);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsEnabledAtLOD::CreateObject
//
//	PURPOSE:	called to determine if this render target is enabled at this LOD
//
// ----------------------------------------------------------------------- //
bool CRenderTargetGroupFX::IsEnabledAtLOD(uint32 nLOD)
{
	LTASSERT(nLOD < LTARRAYSIZE(m_nDimensions), "Error: Invalid access into the dimension list");
	return (m_nDimensions[nLOD].x > 0) && (m_nDimensions[nLOD].y > 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetGroupFX::Update
//
//	PURPOSE:	Update the grid
//
// ----------------------------------------------------------------------- //

bool CRenderTargetGroupFX::Update()
{
    if(m_bWantRemove)
       return false;

	//see if our LOD has changed
	uint32 nCurrLOD = GetRenderTargetLOD();
	if(m_nCreatedLOD != nCurrLOD)
		CreateRenderTarget(nCurrLOD);
		
    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetGroupFX::CreateRenderTarget
//
//	PURPOSE:	this handles creation of the render target
//
// ----------------------------------------------------------------------- //

bool CRenderTargetGroupFX::CreateRenderTarget(uint32 nLOD)
{
	//sanity check on the LOD
	LTASSERT(nLOD < LTARRAYSIZE(m_nDimensions), "Error: Invalid LOD specified");

	//first off clear up any existing render target
	ReleaseRenderTarget();

	//store this LOD setting, even if we fail so that way we don't keep trying to recreate at this
	//resolution
	m_nCreatedLOD = nLOD;

	//we don't need to do anything if we are disabled at this LOD
	if(!IsEnabledAtLOD(nLOD))
		return true;

	//now we need to try and create a new render target using the options provided. Note
	//that we assume that the server has already validated these flags
	uint32 nRTFlags = eRTO_DepthBuffer;

	if(m_bCubeMap)
		nRTFlags |= eRTO_CubeMap;
	if(m_bMipMap)
		nRTFlags |= eRTO_AutoGenMipMaps;
	if(m_bFogVolumes)
		nRTFlags |= eRTO_FogVolume;
	if(m_bLastFrameEffects)
		nRTFlags |= eRTO_PreviousFrameEffect;
	if(m_bCurrFrameEffects)
		nRTFlags |= eRTO_CurrentFrameEffect;

	//now that we have our options, attempt to create the render target
	if(g_pLTClient->GetRenderer()->CreateRenderTarget(m_nDimensions[nLOD].x, m_nDimensions[nLOD].y, nRTFlags, m_hRenderTarget) != LT_OK)
	{
		return false;
	}

	//success
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetGroupFX::ReleaseRenderTarget
//
//	PURPOSE:	this will release the associated render target
//
// ----------------------------------------------------------------------- //

bool CRenderTargetGroupFX::ReleaseRenderTarget()
{
	//if we don't have one just bail
	if(!m_hRenderTarget)
		return true;

	//and now release our reference to it
	g_pLTClient->GetRenderer()->ReleaseRenderTarget(m_hRenderTarget);

	//and clear our reference
	m_hRenderTarget = NULL;

	//success
	return true;
}
