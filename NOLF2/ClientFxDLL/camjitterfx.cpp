// ----------------------------------------------------------------------- //
//
// MODULE  : CamJitterFX.cpp
//
// PURPOSE : The CamJitterFX object
//
// CREATED : 12/30/98
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// Includes....

#include "stdafx.h"
#include "clientfx.h"
#include "CamJitterFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCamJitterProps::CCamJitterProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CCamJitterProps::CCamJitterProps() : 
	m_fInnerDistSqrd	( 250000.0f ),
	m_fOuterDistSqrd	( 360000.0f )
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCamJitterProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CCamJitterProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "InnerRadius" ))
		{
			LTFLOAT	fRad = fxProp.GetFloatVal();
			m_fInnerDistSqrd = fRad * fRad;
		}
		else if( !_stricmp( fxProp.m_sName, "OuterRadius" ))
		{
			LTFLOAT fRad = fxProp.GetFloatVal();
			m_fOuterDistSqrd = fRad * fRad;
		}
	}

	if( m_fInnerDistSqrd >= m_fOuterDistSqrd )
		m_fOuterDistSqrd = m_fInnerDistSqrd + 1.0f;

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CCamJitterFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CCamJitterFX::CCamJitterFX()
:	CBaseFX				( CBaseFX::eCamJitterFX )
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CCamJitterFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CCamJitterFX::~CCamJitterFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CCamJitterFX
//
//------------------------------------------------------------------

bool CCamJitterFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

	m_bUpdateScale  = true;

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CCamJitterFX
//
//------------------------------------------------------------------

void CCamJitterFX::Term()
{
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CCamJitterFX
//
//------------------------------------------------------------------

bool CCamJitterFX::Update(float tmCur)
{
	// Base class update first
	
	if (!CBaseFX::Update(tmCur)) 
		return false;

	// Return out if we have shutdown

	if (IsShuttingDown()) 
		return true;

	if (!g_bAppFocus)
	{
		return true;
	}

	LTVector	vCurCamPos;
	LTVector	vObjPos;
	
	// Retrieve the current position of the camera and get the distance to it

	m_pLTClient->GetObjectPos(m_hCamera, &vCurCamPos);

	if( m_hParent )
	{
		m_pLTClient->GetObjectPos( m_hParent, &vObjPos );
	}
	else
	{
		vObjPos = m_vCreatePos;
	}

	LTFLOAT	fDistSqrd = vObjPos.DistSqr( vCurCamPos );
	LTFLOAT	fFallOff;

	// Figure out the fall off of the shaking based on the inner and outer radii...

	if( fDistSqrd > GetProps()->m_fOuterDistSqrd )
	{
		return LTTRUE;
	}
	else if( fDistSqrd <= GetProps()->m_fInnerDistSqrd )
	{
		fFallOff = 1.0f;
	}
	else
	{
		fFallOff = 1 - ((fDistSqrd - GetProps()->m_fInnerDistSqrd) / (GetProps()->m_fOuterDistSqrd - GetProps()->m_fInnerDistSqrd));
	}

	// Randomize the position offset mased on the scale of hte FX and the fall off

	LTVector vRand;
	vRand.x = GetRandom( -1.0f, 1.0f ) * m_scale * fFallOff;
	vRand.y = GetRandom( -1.0f, 1.0f ) * m_scale * fFallOff;
	vRand.z = GetRandom( -1.0f, 1.0f ) * m_scale * fFallOff;

	vRand += vCurCamPos;

	m_pLTClient->SetObjectPos(m_hCamera, &vRand);

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetCamJitterFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetCamJitterProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	// Add the base props

	AddBaseProps(pList);

	fxProp.Float( "InnerRadius", 500.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "OuterRadius", 600.0f );
	pList->AddTail( fxProp );
}