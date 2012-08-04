//------------------------------------------------------------------
//
//   MODULE  : SPRITEFX.CPP
//
//   PURPOSE : Implements class CSpriteFX
//
//   CREATED : On 11/23/98 At 6:21:37 PM
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "ClientFX.h"
#include "SpriteFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteProps::CSpriteProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CSpriteProps::CSpriteProps() : 
	m_bCastVisibleRay( false ),
	m_fMinScale(1.0f),
	m_fMaxScale(1.0f)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CSpriteProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if( LTStrIEquals( pszName, "CastVisibleRay" ))
	{
		m_bCastVisibleRay = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Color" ))
	{
		m_cfcColor.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "Scale" ))
	{
		m_ffcScale.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MinOverallScale" ))
	{
		m_fMinScale = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "MaxOverallScale" ))
	{
		m_fMaxScale = CFxProp_Float::Load(pStream);
	}
	else
	{
		return CBaseSpriteProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

bool CSpriteProps::PostLoadProperties()
{
	if( m_fMinScale > m_fMaxScale )
	{
		std::swap(m_fMinScale, m_fMaxScale);
	}

	return CBaseSpriteProps::PostLoadProperties();
}

//------------------------------------------------------------------
//
//   FUNCTION : CSpriteFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CSpriteFX::CSpriteFX( CBaseFX::FXType nType ) :	
	CBaseSpriteFX( nType )
{	
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CSpriteFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CSpriteFX::~CSpriteFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CSpriteFX
//
//------------------------------------------------------------------

bool CSpriteFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{

	// Perform base class initialisation

	if( !CBaseSpriteFX::Init(pBaseData, pProps))
		return false;

	//install our visible callback if we need to cast a visible ray
	if(GetProps()->m_bCastVisibleRay)
	{
		g_pLTClient->GetCustomRender()->SetVisibleCallback(m_hObject, CustomRenderVisibleCallback);
	}

	//determine a random overall scale
	m_fScale = GetRandom(GetProps()->m_fMinScale, GetProps()->m_fMaxScale);

	//also determine the largest scale possible so we don't have to update this every frame
	float fMaxScale = GetProps()->m_ffcScale.GetFirstValue();
	for(uint32 nKey = 1; nKey < GetProps()->m_ffcScale.GetNumKeys(); nKey++)
		fMaxScale = LTMAX(fMaxScale, GetProps()->m_ffcScale.GetKey(nKey));

	//and use this as our visible scale
	SetVisScale(fMaxScale * m_fScale);
	
	// Success !!
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CSpriteFX
//
//------------------------------------------------------------------

void CSpriteFX::Term()
{
	CBaseSpriteFX::Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CSpriteFX
//
//------------------------------------------------------------------

bool CSpriteFX::Update(float tmFrameTime)
{
	// Base class update first
	
	if( !CBaseSpriteFX::Update(tmFrameTime) )
		return true;

	//determine the unit lifetime
	float fUnitLifetime = GetUnitLifetime();

	//update the color of the sprite
	m_vColor = GetProps()->m_cfcColor.GetValue(fUnitLifetime);
	
	//and now determine the overall scale
	SetScale(m_fScale * GetProps()->m_ffcScale.GetValue(fUnitLifetime));

	return true;
}

//hook for the custom render object, this will just call into the render function
bool CSpriteFX::CustomRenderVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera, void* pUser)
{
	return ((CSpriteFX*)pUser)->HandleVisibleCallback(tCamera, tSkyCamera);
}

//function that handles the visible callback
bool CSpriteFX::HandleVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera)
{
	// See if anything is blocking our path to the camera
	LTVector vObjPos;
	g_pLTClient->GetObjectPos(m_hObject, &vObjPos);
		
	if(!IsPointVisible(tCamera.m_vPos, vObjPos, IsInSky(GetProps()->m_eInSky)))
	{
		return false;
	}

	//visible
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetSpriteFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetSpriteProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props
	fxGetBaseSpriteProps(pList);

	// Add all the props to the list
	fxProp.SetupEnumBool( "CastVisibleRay", false, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupColor( "Color", 0xFFFFFFFF, eCurve_Linear, "Color of the sprite over time");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "Scale", 1.0f, 0.0f, eCurve_Linear, "Scale of the sprite over time");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "MinOverallScale", 1.0f, 0.0f, eCurve_None, "Minimum overall scale that the sprite can be. This allows for an additional randomized scale to be applied on the sprite");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "MaxOverallScale", 1.0f, 0.0f, eCurve_None, "See MinOverallScale");
	pList->AddTail( fxProp );
}