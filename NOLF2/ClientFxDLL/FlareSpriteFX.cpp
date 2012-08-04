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

void fxGetSpriteProps(CFastList<FX_PROP> *pList);
extern HOBJECT	g_hPlayer;


//Function to handle filtering of the intersect segment calls needed by the flare sprite
inline bool FlareSpriteListFilterFn(HOBJECT hTest, void *pUserData)
{
	// Check for the object type. We only want to be blocked by world models since
	//otherwise models will cause us to flicker and we can get hit by lots of other items
	uint32 nObjType;
	if(g_pLTClient->Common()->GetObjectType(hTest, &nObjType) != LT_OK)
		return false;

	if(nObjType != OT_WORLDMODEL)
		return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteProps::CFlareSpriteProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CFlareSpriteProps::CFlareSpriteProps() :
	m_fMinAngle			( 0.0f ),
	m_bUseCameraAngle	( LTTRUE ),
	m_fMinAlpha			( 0.0f ),
	m_fMaxAlpha			( 0.0f ),
	m_fMinScale			( 0.0f ),
	m_fMaxScale			( 10.0f ),
	m_fScaleRange		( 0.0f ),
	m_fBlindSprAngle	( 0.0f ),
	m_fBlindCamAngle	( 0.0f ),
	m_bBlindingFlare	( LTFALSE ),
	m_fBlindMaxScale	( 10.0f )
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CFlareSpriteProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CSpriteProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "MinAngle" ))
		{
			m_fMinAngle = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "ObjectAngle" ))
		{
			m_bUseCameraAngle = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinAlpha" ))
		{
			m_fMinAlpha = LTCLAMP( fxProp.GetFloatVal(), 0.0f, 1.0f );
		}
		else if( !_stricmp( fxProp.m_sName, "MaxAlpha" ))
		{
			m_fMaxAlpha = LTCLAMP( fxProp.GetFloatVal(), 0.0f, 1.0f );
		}
		else if( !_stricmp( fxProp.m_sName, "MinScale" ))
		{
			m_fMinScale = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxScale" ))
		{
			m_fMaxScale = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "BlindObjectAngle" ))
		{	
			m_bUseCamBlindAngle = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "BlindSpriteAngle" ))
		{
			m_fBlindSprAngle = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "BlindCameraAngle" ))
		{
			m_fBlindCamAngle = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "BlindMaxScale" ))
		{
			m_fBlindMaxScale = fxProp.GetFloatVal();
		}
	}

	if( m_fMinAlpha > m_fMaxAlpha )
	{
		m_fMinAlpha = 0.0f;
		m_fMaxAlpha = 1.0f;
	}

	if( m_fMinScale > m_fMaxScale )
	{
		m_fMinScale = m_fMaxScale;
	}

	m_fScaleRange = m_fMaxScale - m_fMinScale;

	if( (m_fBlindSprAngle > MATH_EPSILON) || (m_fBlindCamAngle > MATH_EPSILON) )
		m_bBlindingFlare = LTTRUE;

	if( m_fBlindMaxScale < m_fMaxScale )
		m_fBlindMaxScale = m_fMaxScale;

	//force rotation on this sprite
	m_bRotate = TRUE;

	//the parent sprite should NEVER be doing a ray cast though since we already
	//will be, so make sure that is false
	m_bCastVisibleRay = false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFlareSpriteFX::CFlareSpriteFX
//
//  PURPOSE:	Standard constuctor
//
// ----------------------------------------------------------------------- //

CFlareSpriteFX::CFlareSpriteFX()
:	CSpriteFX			( CBaseFX::eFlareSpriteFX )
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

bool CFlareSpriteFX::Init( ILTClient *pClientDE, FX_BASEDATA *pBaseData, CBaseFXProps *pProps)
{
	// Let the CSpriteFX base class create the actuall object

	if( !CSpriteFX::Init( pClientDE, pBaseData, pProps ) )
		return LTFALSE;

	// We always want to rotate so we know what the correct forward is...

	m_pLTClient->Common()->SetObjectFlags( m_hObject, OFT_Flags, FLAG_ROTATEABLESPRITE, FLAG_ROTATEABLESPRITE );

	m_bUpdateScale = LTFALSE;

	return LTTRUE;
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
	// Let our base class handel the normal sprite update

	if( !CSpriteFX::Update( tmCur ) )
		return LTFALSE;

	if( !m_hCamera )
		return LTFALSE;

	// Get the Forwards of the Sprite and the Camera and calculate the direction...

	LTVector vObjPos, vCamPos;
	m_pLTClient->GetObjectPos( m_hObject, &vObjPos );
	m_pLTClient->GetObjectPos( m_hCamera, &vCamPos );

	LTRotation	rObjRot, rCamRot;
	LTFLOAT		fCamFace = (GetProps()->m_nFacing == FACE_CAMERAFACING) ? -1.0f : 1.0f;

	m_pLTClient->GetObjectRotation( m_hObject, &rObjRot );
	m_pLTClient->GetObjectRotation( m_hCamera, &rCamRot );

	LTVector vObjF, vCamF;
	vObjF = rObjRot.Forward();
	vCamF = rCamRot.Forward();

	LTVector vDir = vObjPos - vCamPos;
	vDir.Normalize();

	// Find the angles...
	
	LTFLOAT fCameraAngle = vDir.Dot( vCamF );
	fCameraAngle = fCameraAngle < 0.0f ? 0.0f : fCameraAngle;
	fCameraAngle *= 90.0f;

	LTFLOAT fObjectAngle = -vDir.Dot( vObjF ) * fCamFace;
	fObjectAngle = fObjectAngle < 0.0f ? 0.0f : fObjectAngle;
	fObjectAngle *= 90.0f;

	// Do we want to use the Camera or Parent angle?

	LTFLOAT	fAngle = GetProps()->m_bUseCameraAngle ? fCameraAngle : fObjectAngle;

	// If we are within the threshold of looking at the flare update the color and scale...

	if( fAngle < ( 90.0f - GetProps()->m_fMinAngle ) )
	{
		// ...otherwise make it invisible.

		m_pLTClient->Common()->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_VISIBLE );
	}
	else
	{
		if( GetProps()->m_bBlindingFlare )
		{
			m_pLTClient->Common()->SetObjectFlags( m_hObject, OFT_Flags, FLAG_SPRITEBIAS | FLAG_VISIBLE, FLAG_SPRITE_NOZ | FLAG_SPRITEBIAS | FLAG_VISIBLE);
		}
		else
		{
			m_pLTClient->Common()->SetObjectFlags( m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
		}

		// Calculate the multiplier we want to use based on the angle 

		LTFLOAT fMultiplier = ( fAngle + GetProps()->m_fMinAngle - 90.0f ) / GetProps()->m_fMinAngle;
		
		// Update the color and alpha based on off from direct center of the sprite we are looking

		LTFLOAT	a = LTCLAMP( fMultiplier * m_alpha, GetProps()->m_fMinAlpha, GetProps()->m_fMaxAlpha );
		m_pLTClient->SetObjectColor( m_hObject, 
									 fMultiplier * m_red, 
									 fMultiplier * m_green,
									 fMultiplier * m_blue,
									 a ); // make sure alpha doesn't go outside our range

		// Update the scale...

		LTFLOAT	fScale = GetProps()->m_fMinScale + ( fMultiplier * GetProps()->m_fScaleRange );

		m_pLTClient->SetObjectScale( m_hObject,	&LTVector( fScale, fScale, fScale ));

		// No since in updating anymore if the sprite is completly see through 

		if( a < 0.001 )
			return LTTRUE;

		if( GetProps()->m_bBlindingFlare )
		{
			if( (fObjectAngle > ( 90.0f - GetProps()->m_fBlindSprAngle )) && (fCameraAngle > ( 90.0f - GetProps()->m_fBlindCamAngle )) )
			{
				// See if anything is blocking our path to the camera

				ClientIntersectQuery	iQuery;
				ClientIntersectInfo		iInfo;

				iQuery.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
				iQuery.m_FilterFn	= FlareSpriteListFilterFn;
				iQuery.m_pUserData	= NULL;
				iQuery.m_From		= vObjPos;
				iQuery.m_To			= vCamPos;

				if( !m_pLTClient->IntersectSegment( &iQuery, &iInfo ) )
				{
					LTFLOAT	fBlindingAngle = GetProps()->m_bUseCamBlindAngle ? GetProps()->m_fBlindCamAngle : GetProps()->m_fBlindSprAngle;
					fAngle = GetProps()->m_bUseCamBlindAngle ? fCameraAngle : fObjectAngle;

					fMultiplier = ( fAngle + fBlindingAngle - 90.0f ) / fBlindingAngle;

					LTFLOAT	fBlindScaleRange = GetProps()->m_fBlindMaxScale - fScale;
					fScale = fScale + ( fMultiplier * fBlindScaleRange );
					m_pLTClient->SetObjectScale( m_hObject,	&LTVector( fScale, fScale, fScale ));
					
					m_pLTClient->Common()->SetObjectFlags( m_hObject, OFT_Flags, FLAG_SPRITE_NOZ, FLAG_SPRITEBIAS | FLAG_SPRITE_NOZ);
				}

			}
		}

	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetFlareSpriteProps
//
//  PURPOSE:	Returns a list of properties for this FX
//
// ----------------------------------------------------------------------- //

void fxGetFlareSpriteProps(CFastList<FX_PROP> *pList)
{
	fxGetSpriteProps( pList );

	FX_PROP fxProp;

	fxProp.Float( "MinAngle", 45.0f );
	pList->AddTail( fxProp );

	fxProp.Combo( "ObjectAngle", "1,Sprite,Camera" );
	pList->AddTail( fxProp );

	fxProp.Float( "MinAlpha", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxAlpha", 1.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MinScale", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxScale", 10.0f );
	pList->AddTail( fxProp );

	fxProp.Combo( "BlindObjectAngle", "1,Sprite,Camera" );
	pList->AddTail( fxProp );

	fxProp.Float( "BlindSpriteAngle", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "BlindCameraAngle", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "BlindMaxScale", 10.0f );
	pList->AddTail( fxProp );
}