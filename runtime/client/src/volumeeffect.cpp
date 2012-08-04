// ----------------------------------------------------------------------- //
//
// MODULE  : volumeeffect.cpp
//
// PURPOSE : Volume Effect support functions - Implementation
//
// CREATED : 1/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "bdefs.h"
#include "clientmgr.h"
#include "volumeeffect.h"


static bool InitDynamicParticles( LTVolumeEffect* pVE, VolumeEffectInfo* pInfo )
{
	pVE->m_DPUpdateFn = pInfo->m_DPUpdateFn;
	pVE->m_DPPrimitive = pInfo->m_DPPrimitive;
	pVE->m_DPUserData = pInfo->m_DPUserData;
	pVE->m_DPLighting = pInfo->m_DPLighting;
	pVE->m_DPLightConstant = pInfo->m_DPLightConstant;
	pVE->m_DPSaturate = pInfo->m_DPSaturate;
	pVE->m_DPTexture = NULL;

	if( pInfo->m_DPTextureName )
	{
		FileRef ref;

		ref.m_FileType = FILE_CLIENTFILE;
		ref.m_pFilename = pInfo->m_DPTextureName;
		
		if( !(pVE->m_DPTexture = g_pClientMgr->AddSharedTexture( &ref )) )
			return false;
	}

	return true;
}


bool ve_Init( LTVolumeEffect* pVE, VolumeEffectInfo* pInfo )
{
	ve_Term( pVE );

	pVE->m_EffectType = pInfo->m_EffectType;

	pVE->SetDims( pInfo->m_Dims );

	switch( pInfo->m_EffectType )
	{
	case VolumeEffectInfo::kDynamicParticles:
		return InitDynamicParticles( pVE, pInfo );
	default:
		ASSERT(0);
		return false;
	}
}


void ve_Term( LTVolumeEffect* pVE )
{
}
