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
#include "fxflags.h"
#include "BaseCreateFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBaseCreateProps::CBaseCreateProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CBaseCreateProps::CBaseCreateProps() : 
	m_nFXFlags(0),
	m_nNumEffects(0),
	m_nNumToCreate(1)
{
	//initalize all the names to be empty
	for(uint32 nCurrEffect = 0; nCurrEffect < knNumEffects; nCurrEffect++)
	{
		m_pszEffects[nCurrEffect] = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBaseCreateProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CBaseCreateProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//check for one of the effect names
	char pszPropName[32];
	for(uint32 nCurrEffect = 0; nCurrEffect < knNumEffects; nCurrEffect++)
	{
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Effect%d", nCurrEffect);
		if( LTStrIEquals(pszPropName, pszName) )
		{
			//we found a match, record the effect name
			const char* pszEffectName = CFxProp_String::Load(pStream, pszStringTable);

			if(pszEffectName && !LTStrEmpty(pszEffectName))
			{
				m_pszEffects[m_nNumEffects] = pszEffectName;
				m_nNumEffects++;
			}
			return true;
		}
	}

	//it wasn't an effect name, so check other properties
	if( LTStrIEquals( pszName, "Loop" ))
	{
		if( CFxProp_EnumBool::Load(pStream) )
			m_nFXFlags |= FXFLAG_LOOP;
	}
	else if( LTStrIEquals( pszName, "NumToCreate" ))
	{
		m_nNumToCreate = CFxProp_Int::Load(pStream);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CBaseCreateProps::CollectResources(IFXResourceCollector& Collector)
{
	//run through each effect we could possible create and collect their resources
	for(uint32 nCurrEffect = 0; nCurrEffect < m_nNumEffects; nCurrEffect++)
	{
		Collector.CollectEffect(m_pszEffects[nCurrEffect]);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBaseCreateFX::Init
//
//  PURPOSE:	Initialises class CCreateFX
//
//	NOTE:		Fill the FX_BASEDATA struct out with the properties for 
//				creating a whole new fx in the ClientFXMgr and return false
//				so this fx will get deleted and the new one will get created.
//
// ----------------------------------------------------------------------- //

bool CBaseCreateFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps )
{
	if( !CBaseFX::Init(pData, pProps ) )
		return false;

	//success
	return true;
}

//this function should be called from all base classes that pass in the name of the effect that
//should be created, along with the position and orientation. The effect will then be created
//at the specified position and orientation. 
bool CBaseCreateFX::CreateEffect(const CLIENTFX_CREATESTRUCT& CreateStruct)
{
	//fail if there is no effect specified
	if(GetProps()->m_nNumEffects == 0)
		return false;

	//find a random effect within our collection of effects
	uint32 nRandom = GetRandom(0, GetProps()->m_nNumEffects - 1);
	
	//and now create it at the specified rotation and position
	CreateNewFX(m_pFxMgr, GetProps()->m_pszEffects[nRandom], CreateStruct, true);

	//success
	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetCreateProps
//
//  PURPOSE:	Returns a list of properties for this FX
//
// ----------------------------------------------------------------------- //

void fxGetBaseCreateProps(CFastList<CEffectPropertyDesc> *pList)
{
	//add the standard properties
	AddBaseProps( pList );

	CEffectPropertyDesc fxProp;

	//add all of our effects
	char pszPropName[32];
	for(uint32 nCurrEffect = 0; nCurrEffect < CBaseCreateProps::knNumEffects; nCurrEffect++)
	{
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Effect%d", nCurrEffect);
		fxProp.SetupClientFXRef(pszPropName, "", eCurve_None, "Specifies one of the potential effects that can be created. These should be listed without any blank entries between effects");
		pList->AddTail(fxProp);		
	}

	fxProp.SetupIntMin( "NumToCreate", 1, 0, eCurve_None, "Indicates the number of effects that should be randomly chosen and created");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "Loop", false, eCurve_None, "Indicates if the created effect should loop or only play once");
	pList->AddTail(fxProp);
}