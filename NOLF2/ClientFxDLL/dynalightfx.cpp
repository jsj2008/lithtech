//------------------------------------------------------------------
//
//   MODULE  : DYNALIGHTFX.CPP
//
//   PURPOSE : Implements class CDynaLightFX
//
//   CREATED : On 12/14/98 At 5:43:43 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "DynaLightFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynaLightProps::CDynaLightProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CDynaLightProps::CDynaLightProps() : 
	m_bFlicker(false),
	m_bForceLightWorld(false)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDynaLightProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CDynaLightProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "Flicker"))
		{
			m_bFlicker = fxProp.GetIntegerVal() ? true : false;
		}
		else if( !_stricmp( fxProp.m_sName, "ForceLightWorld"))
		{
			m_bForceLightWorld = fxProp.GetComboVal() ? true : false;
		}
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CDynaLightFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CDynaLightFX::CDynaLightFX()
:	CBaseFX		( CBaseFX::eDynaLightFX )
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CDynaLightFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CDynaLightFX::~CDynaLightFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CDynaLightFX
//
//------------------------------------------------------------------

bool CDynaLightFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

	LTVector vScale;
	vScale.Init(100.0f, 100.0f, 100.0f);

	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);

	ocs.m_ObjectType		= OT_LIGHT;
	ocs.m_Flags				= pBaseData->m_dwObjectFlags;
	ocs.m_Flags2			|= pBaseData->m_dwObjectFlags2;
	ocs.m_Pos				= m_vCreatePos;
	ocs.m_Rotation			= m_rCreateRot;
	ocs.m_Scale				= vScale;

	if(GetProps()->m_bForceLightWorld)
	{
		ocs.m_Flags2 |= FLAG2_FORCEDYNAMICLIGHTWORLD;
	}

	// Lights can't be really close

	ocs.m_Flags	&= ~FLAG_REALLYCLOSE;

	// Create the light

	m_hObject = m_pLTClient->CreateObject(&ocs);

	// We want the colour updated thankyou

	m_bUpdateColour = true;
	m_bUpdateScale  = true;

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CDynaLightFX
//
//------------------------------------------------------------------

void CDynaLightFX::Term()
{
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CDynaLightFX
//
//------------------------------------------------------------------

bool CDynaLightFX::Update(float tmFrameTime)
{
	// Base class update first
	
	if (!CBaseFX::Update(tmFrameTime)) 
		return false;

	if (IsShuttingDown())
	{
		m_pLTClient->SetLightRadius(m_hObject, 0);
		
		return true;
	}

	// If we're flickering, change some of the attributes slightly

	if (GetProps()->m_bFlicker)
	{
		float fRand = 0.3f + GetRandom(0.0f, 0.19f);
		
		m_red   *= fRand;
		m_green *= fRand;
		m_blue  *= fRand;
	}
	
	// Try to add some sort of intensity based off the alpha...

	m_red	= LTCLAMP( m_red * m_alpha, 0.0f, 1.0f );
	m_green	= LTCLAMP( m_green * m_alpha, 0.0f, 1.0f );
	m_blue	= LTCLAMP( m_blue * m_alpha, 0.0f, 1.0f );

	// Set the new light colour

	m_pLTClient->SetLightColor(m_hObject, m_red, m_green, m_blue);

	// Set the new light scale

	m_pLTClient->SetLightRadius(m_hObject, m_scale);

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetDynaLightFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetDynaLightProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	// Add the base props

	AddBaseProps(pList);

	fxProp.Int("Flicker", 0);
	pList->AddTail(fxProp);

	fxProp.Combo( "ForceLightWorld", "0, No, Yes" );
	pList->AddTail( fxProp );
}