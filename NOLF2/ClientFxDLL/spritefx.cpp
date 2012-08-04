//------------------------------------------------------------------
//
//   MODULE  : SPRITEFX.CPP
//
//   PURPOSE : Implements class CSpriteFX
//
//   CREATED : On 11/23/98 At 6:21:37 PM
//
//------------------------------------------------------------------

//
// Includes....
//

#include "stdafx.h"
#include "ClientFX.h"
#include "SpriteFX.h"
#include "iltspritecontrol.h"

//Function to handle filtering of the intersect segment calls needed by the flare sprite
inline bool SpriteListFilterFn(HOBJECT hTest, void *pUserData)
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
//  ROUTINE:	CSpriteProps::CSpriteProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CSpriteProps::CSpriteProps() : 
	m_nFacing			( 0 ),
	m_bCastVisibleRay	( LTFALSE ),
	m_dwObjectFlags2	( 0 ),
	m_dwObjectFlags		( 0 ),
	m_vNorm				( 0.0f, 0.0f ,1.0f ),
	m_bRotate			( LTFALSE )
{
	m_sSpriteName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CSpriteProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "Sprite" ))
		{
			fxProp.GetPath( m_sSpriteName );
		}
		else if( !_stricmp( fxProp.m_sName, "Facing" ))
		{
			m_nFacing = fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Normal"))
		{
			m_vNorm = fxProp.GetVector();

			// Dont norm a zero length vector!

			if( m_vNorm.LengthSquared() > MATH_EPSILON )
			{
				m_vNorm.Normalize();
			}
			else
			{
				// Just kludge the forward down the Z

				m_vNorm.Init( 0.0f, 0.0f, 1.0f );
			}
		}
		else if( !_stricmp( fxProp.m_sName, "BlendMode" ))
		{
			int	nBlendMode = fxProp.GetComboVal();

			if( nBlendMode == 1 )
			{
				m_dwObjectFlags  |= FLAG_NOLIGHT;
				m_dwObjectFlags2 |= FLAG2_ADDITIVE;
			}
			else if( nBlendMode == 2 )
			{
				m_dwObjectFlags  |= FLAG_NOLIGHT;
				m_dwObjectFlags2 |= FLAG2_MULTIPLY;
			}
		}
		else if( !_stricmp( fxProp.m_sName, "DisableZ" ))
		{
			LTBOOL	bNoZ = (LTBOOL)fxProp.GetComboVal();
			
			m_dwObjectFlags |= ( bNoZ ? FLAG_SPRITE_NOZ : 0 );
		}
		else if( !_stricmp( fxProp.m_sName, "DisableLight" ))
		{
			LTBOOL bNoLight = (LTBOOL)fxProp.GetComboVal();

			m_dwObjectFlags |= ( bNoLight ? FLAG_NOLIGHT : 0 );
		}
		else if( !_stricmp( fxProp.m_sName, "CastVisibleRay" ))
		{
			m_bCastVisibleRay = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "ZBias" ))
		{
			if((LTBOOL)fxProp.GetComboVal())
			{
				m_dwObjectFlags |= FLAG_SPRITEBIAS;
			}
		}
	}

	m_bRotate = ( (m_vRotAdd.x != 0.0f) || (m_vRotAdd.y != 0.0f) || (m_vRotAdd.z != 0.0f) ) ? LTTRUE : LTFALSE ;

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CSpriteFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CSpriteFX::CSpriteFX( CBaseFX::FXType nType )
:	CBaseFX				( nType )
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

bool CSpriteFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{

	// Perform base class initialisation

	if( !CBaseFX::Init( pClientDE, pBaseData, pProps))
		return false;
	
	// Use the "target" Normal instead, if one was specified...
	LTVector vNorm = GetProps()->m_vNorm;

	if( pBaseData->m_vTargetNorm.LengthSquared() > MATH_EPSILON )
	{
		vNorm = pBaseData->m_vTargetNorm;
		vNorm.Normalize();
	}

	// Develop the Right and Up vectors based off the Forward...

	LTVector vR, vU;
	if( (1.0f == vNorm.y) || (-1.0f == vNorm.y) )
	{
		vR = LTVector( 1.0f, 0.0f, 0.0f ).Cross( vNorm );
	}
	else
	{
		vR = LTVector( 0.0f, 1.0f, 0.0f ).Cross( vNorm );
	}

	vU = vNorm.Cross( vR );
	m_rNormalRot = LTRotation( vNorm, vU );

	
	// Combine the direction we would like to face with our parents rotation...

	ObjectCreateStruct ocs;
	if( m_hParent )
	{
		m_pLTClient->GetObjectRotation( m_hParent, &ocs.m_Rotation );
	}
	else
	{
		ocs.m_Rotation = m_rCreateRot;
	}

	ocs.m_Rotation = ocs.m_Rotation * m_rNormalRot;	

	ocs.m_ObjectType		= OT_SPRITE;
	ocs.m_Flags				|= GetProps()->m_dwObjectFlags | pBaseData->m_dwObjectFlags | (GetProps()->m_nFacing || GetProps()->m_bRotate ? FLAG_ROTATABLESPRITE : 0);
	ocs.m_Flags2			|= pBaseData->m_dwObjectFlags2 | GetProps()->m_dwObjectFlags2;
	
	// Calculate the position with the offset in 'local' coordinate space...

	LTMatrix mMat;
	ocs.m_Rotation.ConvertToMatrix( mMat );

	m_vPos = ocs.m_Pos = m_vCreatePos + (mMat * GetProps()->m_vOffset);

	// When camera facing we want the Normal rotation at the identity...

	if( GetProps()->m_nFacing == FACE_CAMERAFACING )
		m_rNormalRot.Init();
	
	strcpy( ocs.m_Filename, GetProps()->m_sSpriteName );

	m_hObject = m_pLTClient->CreateObject( &ocs );
	if( !m_hObject )
		return LTFALSE;

	// Success !!

	return LTTRUE;
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
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = NULL;

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
	// Shutdown....

	if( IsShuttingDown() )
	{
		m_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAGMASK_ALL);
	}
	else if( IsInitialFrame() )
	{
		//we are on the initial frame of another play, so we need to reset
		//our sprite's current position in the animation to the beginning
		//so it doesn't get out of sync, which happens quite a bit.
		ILTSpriteControl *pControl;
		m_pLTClient->GetSpriteControl(m_hObject, pControl);

		pControl->SetCurPos(0, 0);
	}

	// Base class update first
	
	if( !CBaseFX::Update(tmFrameTime) )
		return true;

	if(GetProps()->m_bCastVisibleRay)
	{
		// See if anything is blocking our path to the camera

		ClientIntersectQuery	iQuery;
		ClientIntersectInfo		iInfo;

		LTVector vObjPos, vCamPos;
		m_pLTClient->GetObjectPos( m_hObject, &vObjPos );
		m_pLTClient->GetObjectPos( m_hCamera, &vCamPos );

		iQuery.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iQuery.m_FilterFn	= SpriteListFilterFn;
		iQuery.m_pUserData	= NULL;
		iQuery.m_From		= vObjPos;
		iQuery.m_To			= vCamPos;

		if( m_pLTClient->IntersectSegment( &iQuery, &iInfo ) )
		{
			m_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);

			//don't bother updating anything else since we aren't visible
			return true;
		}
		else
		{
			m_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		}
	}

	// Align if neccessary, to the rotation of our parent

	if( (m_hParent) && (GetProps()->m_nFacing == FACE_PARENTALIGN) )
	{
		LTRotation rParentRot;
		m_pLTClient->GetObjectRotation(m_hParent, &rParentRot);
		rParentRot = ( GetProps()->m_bRotate ? rParentRot : rParentRot * m_rNormalRot );
		m_pLTClient->SetObjectRotation(m_hObject, &rParentRot);
	}

	// If we want to add a rotation, make sure we are facing the correct way...
	
	if( GetProps()->m_bRotate )
	{
		LTFLOAT		tmFrame = tmFrameTime;
		LTVector	vR( m_rRot.Right() );
		LTVector	vU( m_rRot.Up() );
		LTVector	vF( m_rRot.Forward() );

		LTRotation	rRotation;

		if( m_hCamera && (GetProps()->m_nFacing == FACE_CAMERAFACING))
		{
			//grab the flags for this object
			uint32 nFlags;
			m_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags, nFlags);
	
			if(nFlags & FLAG_REALLYCLOSE)
			{
				rRotation.Identity();
			}
			else
			{
				m_pLTClient->GetObjectRotation( m_hCamera, &rRotation );
			}
		}
		else
		{
			m_pLTClient->GetObjectRotation( m_hObject, &rRotation );
		}

		m_rRot.Rotate( vR, MATH_DEGREES_TO_RADIANS(GetProps()->m_vRotAdd.x * tmFrame) );
		m_rRot.Rotate( vU, MATH_DEGREES_TO_RADIANS(GetProps()->m_vRotAdd.y * tmFrame) );
		m_rRot.Rotate( vF, MATH_DEGREES_TO_RADIANS(GetProps()->m_vRotAdd.z * tmFrame) );
		
		rRotation = rRotation * m_rRot;
		
		m_pLTClient->SetObjectRotation( m_hObject, &(rRotation * m_rNormalRot) );
	}

	// Success !!

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetSpriteFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetSpriteProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;
	float vTmp[3];
	vTmp[0] = 0.0f;
	vTmp[1] = 0.0f;
	vTmp[2] = 1.0f;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Path( "Sprite", "spr|..." );
	pList->AddTail(fxProp);

	fxProp.Vector( "Normal", vTmp );
	pList->AddTail(fxProp);

	fxProp.Combo( "Facing", "0,CameraFacing,AlongNormal,ParentAlign" );
	pList->AddTail(fxProp);

	fxProp.Combo( "BlendMode", "0, None, Additive, Multiply" );
	pList->AddTail( fxProp );

	fxProp.Combo( "DisableZ", "0,No,Yes" );
	pList->AddTail( fxProp );

	fxProp.Combo( "DisableLight", "1,No,Yes" );
	pList->AddTail( fxProp );

	fxProp.Combo( "CastVisibleRay", "0,No,Yes" );
	pList->AddTail( fxProp );

	fxProp.Combo( "ZBias", "1,No,Yes" );
	pList->AddTail( fxProp );
}