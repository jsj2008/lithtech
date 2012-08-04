//------------------------------------------------------------------
//
//   MODULE  : CAMWOBBLEFX.CPP
//
//   PURPOSE : Implements class CCamWobbleFX
//
//   CREATED : On 12/30/98 At 3:28:28 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "clientfx.h"
#include "CamWobbleFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCamWobbleProps::CCamWobbleProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CCamWobbleProps::CCamWobbleProps() :
	m_fPeriod			( 1.0f ),
	m_xMultiplier		( 0.05f ),
	m_yMultiplier		( 0.05f ),
	m_fInnerDistSqrd	( 250000.0f ),
	m_fOuterDistSqrd	( 360000.0f )
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCamWobbleProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CCamWobbleProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "xMultiplier"))
		{
			m_xMultiplier = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "yMultiplier"))
		{
			m_yMultiplier = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Reps"))
		{
			m_fPeriod = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "InnerRadius" ))
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
//   FUNCTION : CCamWobbleFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CCamWobbleFX::CCamWobbleFX()
:	CBaseFX				( CBaseFX::eCamWobbleFX ),
	m_xFovAnchor		( 0.0f ),
	m_yFovAnchor		( 0.0f )
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CCamWobbleFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CCamWobbleFX::~CCamWobbleFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CCamWobbleFX
//
//------------------------------------------------------------------

bool CCamWobbleFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps))
		return false;

	m_bUpdateScale  = false;

	// Save the initial camera field of view

	m_pLTClient->GetCameraFOV(m_hCamera, &m_xFovAnchor, &m_yFovAnchor);

	// Success !!

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CCamWobbleFX
//
//------------------------------------------------------------------

void CCamWobbleFX::Term()
{
	// Reset the FOV

	if (m_hCamera) m_pLTClient->SetCameraFOV(m_hCamera, m_xFovAnchor, m_yFovAnchor);

	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CCamWobbleFX
//
//------------------------------------------------------------------

bool CCamWobbleFX::Update(float tmFrameTime)
{
	// Base class update first
	
	if (!CBaseFX::Update(tmFrameTime)) 
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

	// Compute the FOV offsets

	float fLen = GetProps()->m_tmLifespan / GetProps()->m_fPeriod;

	float fVal = fmodf(m_tmElapsed, fLen);

	float fRadVal = (MATH_CIRCLE / fLen) * fVal;
	
	float xOff = m_xFovAnchor + ((float)sin(fRadVal) * GetProps()->m_xMultiplier * fFallOff);
	float yOff = m_yFovAnchor + ((float)cos(fRadVal) * GetProps()->m_yMultiplier * fFallOff);

	m_pLTClient->SetCameraFOV(m_hCamera, xOff, yOff);

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetCamWobbleFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetCamWobbleProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	// Add the base props

	AddBaseProps(pList);

	fxProp.Float("xMultiplier", 0.05f);
	pList->AddTail(fxProp);

	fxProp.Float("yMultiplier", 0.05f);
	pList->AddTail(fxProp);

	fxProp.Float("Reps", 5.0f);
	pList->AddTail(fxProp);

	fxProp.Float( "InnerRadius", 500.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "OuterRadius", 600.0f );
	pList->AddTail( fxProp );
}
