//------------------------------------------------------------------
//
//   MODULE  : NULLFX.CPP
//
//   PURPOSE : Implements class CNullFX
//
//   CREATED : On 12/3/98 At 6:34:44 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "NullFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CNullProps::CNullProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CNullProps::CNullProps() : 
	m_fGravity			( 0.0f ),
	m_vMinVel			( 0.0f, 0.0f, 0.0f ),
	m_vMaxVel			( 0.0f, 0.0f, 0.0f ),
	m_bBounce			( LTFALSE )
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CNullProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CNullProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "GravityAcceleration" ) ) 
		{
			m_fGravity = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinVelocity" ) )
		{
			m_vMinVel = fxProp.GetVector();	
		}
		else if( !_stricmp( fxProp.m_sName, "MaxVelocity" ) )
		{
			m_vMaxVel = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, "Bounce" ) )
		{
			m_bBounce = (LTBOOL)fxProp.GetComboVal();
		}
	}

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CNullFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CNullFX::CNullFX()
:	CBaseFX				( CBaseFX::eNullFX ),
	m_vVelocity			( 0.0f, 0.0f, 0.0f ),
	m_vPosition			( 0.0f, 0.0f, 0.0f )
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CNullFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CNullFX::~CNullFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CNullFX
//
//------------------------------------------------------------------

bool CNullFX::Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pData, pProps)) 
		return false;

	//setup our velocity
	m_vVelocity.x = GetRandom( GetProps()->m_vMinVel.x, GetProps()->m_vMaxVel.x );
	m_vVelocity.y = GetRandom( GetProps()->m_vMinVel.y, GetProps()->m_vMaxVel.y );
	m_vVelocity.z = GetRandom( GetProps()->m_vMinVel.z, GetProps()->m_vMaxVel.z );

	ObjectCreateStruct ocs;

	ocs.m_ObjectType		= OT_NORMAL;
	ocs.m_Flags				= FLAG_NOLIGHT;
	ocs.m_Pos				= m_vPosition = m_vCreatePos;
	ocs.m_Rotation			= m_rCreateRot;
	
	// Develop the Right and Up vectors based off the Forward...
	LTMatrix mInitRot;
	mInitRot.Identity();

	if( pData->m_vTargetNorm.LengthSquared() > MATH_EPSILON )
	{
		LTVector vR, vU;

		pData->m_vTargetNorm.Normalize();

		if( (1.0f == pData->m_vTargetNorm.y) || (-1.0f == pData->m_vTargetNorm.y) )
		{
			vR = LTVector( 1.0f, 0.0f, 0.0f ).Cross( pData->m_vTargetNorm );
		}
		else
		{
			vR = LTVector( 0.0f, 1.0f, 0.0f ).Cross( pData->m_vTargetNorm );
		}

		vU = pData->m_vTargetNorm.Cross( vR );
		ocs.m_Rotation = LTRotation( pData->m_vTargetNorm, vU );
		ocs.m_Rotation.ConvertToMatrix( mInitRot );
	}

	m_hObject = m_pLTClient->CreateObject( &ocs );
	if( !m_hObject )
		return LTFALSE;

	m_bUpdateColour		= LTFALSE;
	m_bUpdateScale		= LTFALSE;
	
	// Compute our Velocity based on Initial Rotation...

	m_vVelocity = mInitRot * m_vVelocity;
		
	// Success !!

	return LTTRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CNullFX
//
//------------------------------------------------------------------

void CNullFX::Term()
{
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	m_hObject = LTNULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CNullFX
//
//------------------------------------------------------------------

bool CNullFX::Update( LTFLOAT tmFrameTime)
{
	// Base class update first
	
	if( !CBaseFX::Update(tmFrameTime) )
		return false;

	// If we want a bouncy FX check for an intersection on our "next" position...
	
	if( GetProps()->m_bBounce )
	{
		ClientIntersectQuery	iQuery;
		ClientIntersectInfo		iInfo;

		iQuery.m_From = m_vPosition;
		iQuery.m_To = iQuery.m_From + (m_vVelocity * tmFrameTime); 

		if( m_pLTClient->IntersectSegment( &iQuery, &iInfo ) )
		{
	
			LTVector	vNewVel = iInfo.m_Plane.m_Normal * 2;
			LTVector	vL = -m_vVelocity;
			LTFLOAT		Dot = iInfo.m_Plane.m_Normal.Dot( vL );
			
			// Develop the normalized reflected angle...

			vNewVel *= Dot;
			vNewVel -= vL;
			vNewVel.Norm();
			
			// Scale it out with a decreased velocity magnitude

			m_vVelocity = vNewVel * ( m_vVelocity.Mag() * 0.75f);
		}
	}
	
	m_vPosition += m_vVelocity * tmFrameTime;
	m_vVelocity.y += GetProps()->m_fGravity;

	m_pLTClient->SetObjectPos( m_hObject, &m_vPosition );

	// Success !!

	return LTTRUE;
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetNullProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetNullProps(CFastList<FX_PROP> *pList)
{
	FX_PROP	fxProp;
	float	fVec[3];
	fVec[0] = 0.0f;
	fVec[1] = 1.0f;
	fVec[2] = 0.0f;

	// Add the base props

	AddBaseProps(pList);

	fxProp.Float( "GravityAcceleration", -500.0f );
	pList->AddTail( fxProp );

	fxProp.Vector( "MinVelocity", fVec );
	pList->AddTail( fxProp );

	fxProp.Vector( "MaxVelocity", fVec );
	pList->AddTail( fxProp );

	fxProp.Combo( "Bounce", "0, No, Yes" );
	pList->AddTail( fxProp );
}