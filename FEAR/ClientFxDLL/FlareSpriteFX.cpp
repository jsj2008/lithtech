// ----------------------------------------------------------------------- //
//
// MODULE  : FlareSpriteFX.cpp
//
// PURPOSE : This FX is used as a blinding flare
//
// CREATED : 8/01/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "ClientFX.h"
#include "FlareSpriteFX.h"
#include "ClientServerShared.h"
#include "iperformancemonitor.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXFlareSprite("ClientFX_FlareSprite", "ClientFX");

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteProps::CFlareSpriteProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CFlareSpriteProps::CFlareSpriteProps() :
	m_fCosMinAngle		( 0.0f ),
	m_bUseCameraAngle	( true ),
	m_fMinAlpha			( 0.0f ),
	m_fMaxAlpha			( 0.0f ),
	m_fMinScale			( 0.0f ),
	m_fMaxScale			( 10.0f ),
	m_fScaleRange		( 0.0f ),
	m_fCosBlindSprAngle	( 0.0f ),
	m_fCosBlindCamAngle	( 0.0f ),
	m_bBlindingFlare	( false ),
	m_fBlindMaxScale	( 10.0f )
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CFlareSpriteProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if( LTStrIEquals( pszName, "MinAngle" ))
	{
		m_fCosMinAngle = LTCos(MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream)));
	}
	else if( LTStrIEquals( pszName, "ObjectAngle" ))
	{
		m_bUseCameraAngle = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "MinAlpha" ))
	{
		m_fMinAlpha = LTCLAMP( CFxProp_Float::Load(pStream), 0.0f, 1.0f );
	}
	else if( LTStrIEquals( pszName, "MaxAlpha" ))
	{
		m_fMaxAlpha = LTCLAMP( CFxProp_Float::Load(pStream), 0.0f, 1.0f );
	}
	else if( LTStrIEquals( pszName, "MinScale" ))
	{
		m_fMinScale = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "MaxScale" ))
	{
		m_fMaxScale = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "BlindObjectAngle" ))
	{	
		m_bUseCamBlindAngle = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "BlindSpriteAngle" ))
	{
		m_fCosBlindSprAngle = LTCos(MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream)));
	}
	else if( LTStrIEquals( pszName, "BlindCameraAngle" ))
	{
		m_fCosBlindCamAngle = LTCos(MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream)));
	}
	else if( LTStrIEquals( pszName, "BlindMaxScale" ))
	{
		m_fBlindMaxScale = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Color" ))
	{
		m_cfcColor.Load(pStream, pCurveData);
	}
	else
	{
		return CBaseSpriteProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}
	return true;
}


bool CFlareSpriteProps::PostLoadProperties()
{
	if( m_fMinAlpha > m_fMaxAlpha )
	{
		std::swap(m_fMinAlpha, m_fMaxAlpha);
	}

	if( m_fMinScale > m_fMaxScale )
	{
		std::swap(m_fMinScale, m_fMaxScale);
	}

	m_fScaleRange = m_fMaxScale - m_fMinScale;

	if( !LTNearlyEquals(m_fCosBlindSprAngle, 1.0f, 0.001f) || 
		!LTNearlyEquals(m_fCosBlindCamAngle, 1.0f, 0.001f))
	{
		m_bBlindingFlare = true;
	}

	if( m_fBlindMaxScale < m_fMaxScale )
		m_fBlindMaxScale = m_fMaxScale;

	return CBaseSpriteProps::PostLoadProperties();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteFX::CFlareSpriteFX
//
//  PURPOSE:	Standard constuctor
//
// ----------------------------------------------------------------------- //

CFlareSpriteFX::CFlareSpriteFX() :	
	CBaseSpriteFX( CBaseFX::eFlareSpriteFX )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteFX::~CFlareSpriteFX
//
//  PURPOSE:	Standard destructor
//
// ----------------------------------------------------------------------- //

CFlareSpriteFX::~CFlareSpriteFX()
{	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteFX::Init
//
//  PURPOSE:	Initialises class CFlareSpriteFX
//
// ----------------------------------------------------------------------- //

bool CFlareSpriteFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Let the CSpriteFX base class create the actuall object
	if( !CBaseSpriteFX::Init(pBaseData, pProps ) )
		return false;

	//setup our visibility callback
	g_pLTClient->GetCustomRender()->SetVisibleCallback(m_hObject, CustomRenderVisibleCallback);

	//setup our visible radius
	float fVisScale = GetProps()->m_fMinScale + GetProps()->m_fScaleRange;
	if(GetProps()->m_bBlindingFlare)
	{
		LTMAX(fVisScale, GetProps()->m_fBlindMaxScale);
	}

	SetVisScale(fVisScale);

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteFX::Update
//
//  PURPOSE:	Updates class CFlareSpriteFX
//
// ----------------------------------------------------------------------- //

bool CFlareSpriteFX::Update( float tmCur )
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXFlareSprite);

	// Let our base class handel the normal sprite update
	if( !CBaseSpriteFX::Update( tmCur ) )
		return false;

	return true;
}

//hook for the custom render object, this will just call into the render function
bool CFlareSpriteFX::CustomRenderVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera, void* pUser)
{
	return ((CFlareSpriteFX*)pUser)->HandleVisibleCallback(tCamera, tSkyCamera);
}

//function that handles the visible callback
bool CFlareSpriteFX::HandleVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera)
{
	LTRigidTransform tObject;
	g_pLTClient->GetObjectTransform( m_hObject, &tObject );

	// Get the Forwards of the Sprite and the Camera and calculate the direction...
	LTVector vObjForward = tObject.m_rRot.Forward();

	//determine if we are in the sky
	bool bInSky = IsInSky(GetProps()->m_eInSky);

	//determine our camera forward (which is either the normal camera object forward, or rotated if
	//we are in the sky
	LTVector vVisCamForward, vVisCamPos;
	if(bInSky)
	{
		vVisCamForward = tSkyCamera.m_rRot.Forward();
		vVisCamPos = tSkyCamera.m_vPos;
	}
	else
	{
		vVisCamForward = tCamera.m_rRot.Forward();
		vVisCamPos = tCamera.m_vPos;
	}

	LTVector vDir = tObject.m_vPos - vVisCamPos;
	vDir.Normalize();

	// Find the angles...
	float fCameraAngle = vDir.Dot( vVisCamForward );
	fCameraAngle = LTMAX(0.0f, fCameraAngle);

	float fObjectAngle = vDir.Dot( vObjForward );
	fObjectAngle = LTMAX(0.0f, fObjectAngle);

	// Do we want to use the Camera or Parent angle?
	float	fAngle = GetProps()->m_bUseCameraAngle ? fCameraAngle : fObjectAngle;

	// If we are within the threshold of looking at the flare update the color and scale...
	if(fAngle < GetProps()->m_fCosMinAngle)
		return false;
	
	// Calculate the multiplier we want to use based on the angle 
	float fMultiplier = (fAngle - GetProps()->m_fCosMinAngle) / (1.0f - GetProps()->m_fCosMinAngle);

	// Update the color and alpha based on off from direct center of the sprite we are looking

	//determine the color
	m_vColor	= GetProps()->m_cfcColor.GetValue(GetUnitLifetime()) * fMultiplier;
	m_vColor.w	= LTCLAMP( fMultiplier * m_vColor.w, GetProps()->m_fMinAlpha, GetProps()->m_fMaxAlpha );

	// Update the scale...
	float fScale = GetProps()->m_fMinScale + ( fMultiplier * GetProps()->m_fScaleRange );
	SetScale(fScale);

	// No sense in updating anymore if the sprite is completly see through 
	if( m_vColor.w < 0.001 )
		return false;

	if(GetProps()->m_bBlindingFlare)
	{
		if((fObjectAngle > GetProps()->m_fCosBlindSprAngle) && (fCameraAngle > GetProps()->m_fCosBlindCamAngle))
		{
			// See if anything is blocking our path to the camera
			if(!IsPointVisible(tCamera.m_vPos, tObject.m_vPos, bInSky))
				return false;

			float fCosBlindingAngle = GetProps()->m_bUseCamBlindAngle ? GetProps()->m_fCosBlindCamAngle : GetProps()->m_fCosBlindSprAngle;
			fAngle = GetProps()->m_bUseCamBlindAngle ? fCameraAngle : fObjectAngle;

			fMultiplier = (fAngle - fCosBlindingAngle) / (1.0f - fCosBlindingAngle);

			float fBlindScaleRange = GetProps()->m_fBlindMaxScale - fScale;
			fScale = fScale + ( fMultiplier * fBlindScaleRange );
			SetScale(fScale);
		}
	}

	//visible
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetFlareSpriteProps
//
//  PURPOSE:	Returns a list of properties for this FX
//
// ----------------------------------------------------------------------- //

void fxGetFlareSpriteProps(CFastList<CEffectPropertyDesc> *pList)
{
	fxGetBaseSpriteProps( pList );

	CEffectPropertyDesc fxProp;

	fxProp.SetupFloat( "MinAngle", 45.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "ObjectAngle", "Camera", "Sprite,Camera", eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "MinAlpha", 0.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "MaxAlpha", 1.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "MinScale", 0.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "MaxScale", 10.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "BlindObjectAngle", "Camera", "Sprite,Camera", eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "BlindSpriteAngle", 0.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "BlindCameraAngle", 0.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "BlindMaxScale", 10.0f, eCurve_None, "" );
	pList->AddTail( fxProp );

	fxProp.SetupColor( "Color", 0xFFFFFFFF, eCurve_Linear, "Color of the flare sprite");
	pList->AddTail( fxProp );
}
