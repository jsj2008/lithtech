// ----------------------------------------------------------------------- //
//
// MODULE  : CreateFX.cpp
//
// PURPOSE : The ActiveWorldModel object
//
// CREATED : 7/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "CreateFX.h"
#include "iperformancemonitor.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXCreateFX("ClientFX_CreateFX", "ClientFX");

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateProps::CCreateProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CCreateProps::CCreateProps()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CCreateProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	return CBaseCreateProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateFX::Init
//
//  PURPOSE:	Initialises class CCreateFX
//
//	NOTE:		Fill the FX_BASEDATA struct out with the properties for 
//				creating a whole new fx in the ClientFXMgr and return false
//				so this fx will get deleted and the new one will get created.
//
// ----------------------------------------------------------------------- //

bool CCreateFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps )
{
	if( !CBaseCreateFX::Init(pData, pProps ) )
		return false;

	//success
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateFX::Update
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

bool CCreateFX::Update( float tmFrameTime )
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXCreateFX);

	//update our base object
	BaseUpdate(tmFrameTime);	

	//we only want to create the effect as we become active
	if(IsInitialFrame())
	{
		CLIENTFX_CREATESTRUCT CreateStruct;
		CreateStruct.m_dwFlags			= GetProps()->m_nFXFlags;
		CreateStruct.m_hParentObject	= GetParentObject();
		CreateStruct.m_hParentRigidBody = GetParentRigidBody();
		CreateStruct.m_hSocket			= GetSocketAttach();
		CreateStruct.m_hNode			= GetNodeAttach();
		CreateStruct.m_tTransform		= GetParentOffset() * GetAdditionalTransform();

		for(uint32 nCurrEffect = 0; nCurrEffect < GetProps()->m_nNumToCreate; nCurrEffect++)
		{
			CBaseCreateFX::CreateEffect(CreateStruct);
		}
	}

	//we always return false because we always want to be placed into a shutting down state since
	//we just emit and then do nothing
	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetCreateProps
//
//  PURPOSE:	Returns a list of properties for this FX
//
// ----------------------------------------------------------------------- //

void fxGetCreateProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	fxGetBaseCreateProps(pList);
}