// ----------------------------------------------------------------------- //
//
// MODULE  : HitBox.cpp
//
// PURPOSE : Client side reresentation of the CCharacterHitBox object
//
// CREATED : 8/26/02
//
// (c) 2002-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "HitBox.h"
#include "CharacterFX.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::CHitBox()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CHitBox::CHitBox()
:	m_hObject		( INVALID_HOBJECT ),
	m_hModel		( INVALID_HOBJECT ),
	m_vDims			( 0.0f, 0.0f, 0.0f ),
	m_vOffset		( 0.0f, 0.0f, 0.0f )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::~CHitBox()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CHitBox::~CHitBox()
{
	if( m_hObject != INVALID_HOBJECT )
	{
		g_pLTClient->RemoveObject( m_hObject );
	}
	m_hObject = INVALID_HOBJECT;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::Init()
//
//	PURPOSE:	Create our object
//
// ----------------------------------------------------------------------- //

bool CHitBox::Init( HOBJECT hModel, const LTVector &vDims, const LTVector &vOffset )
{
	if( hModel == INVALID_HOBJECT )
		return false;

	m_hModel	= hModel;
	m_vDims		= vDims;
	m_vOffset	= vOffset;

	// Create the hitbox at the propper offset from the models position...

	LTRigidTransform tModelTrans;
	g_pLTClient->GetObjectTransform( m_hModel, &tModelTrans );

	// Get rid of our object if it already exists...

	if( m_hObject )
	{
		g_pLTClient->RemoveObject( m_hObject );
		m_hObject = NULL;
	}

	ObjectCreateStruct ocs;
	
	ocs.m_ObjectType	= OT_NORMAL;
	ocs.m_Flags			= FLAG_RAYHIT;
	ocs.m_Pos			= tModelTrans.m_vPos + (tModelTrans.m_rRot * m_vOffset);
	
	m_hObject = g_pLTClient->CreateObject( &ocs );
	if( m_hObject == INVALID_HOBJECT )
	{
		return false;
	}

	g_pPhysicsLT->SetObjectDims( m_hObject, &m_vDims, 0 );
	
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_HITBOX | USRFLG_CHARACTER, USRFLG_HITBOX | USRFLG_CHARACTER );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::Update()
//
//	PURPOSE:	Update the HitBox...
//
// ----------------------------------------------------------------------- //

bool CHitBox::Update()
{
	if( !m_hModel )
	{
		return true;
	}

	// Offset the HitBox from the position of the associated model...
	
	LTVector vPos;
	g_pLTClient->GetObjectPos( m_hModel, &vPos );

	LTRotation rRot;
	g_pLTClient->GetObjectRotation( m_hModel, &rRot );

	vPos += (rRot * m_vOffset);

	g_pLTClient->SetObjectPos( m_hObject, vPos );

	// If the model object is not visible we should not be able to interact with the hitbox

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hModel, OFT_Flags, dwFlags );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, ((dwFlags & FLAG_VISIBLE) ? FLAG_RAYHIT : 0), FLAG_RAYHIT );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::SetDims()
//
//	PURPOSE:	Set the dimensions of the hitbox...
//
// ----------------------------------------------------------------------- //

void CHitBox::SetDims( const LTVector &vDims )
{
	if( !m_hObject )
		return;

	m_vDims = vDims;
	g_pPhysicsLT->SetObjectDims( m_hObject, &m_vDims, 0 );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::SetOffset()
//
//	PURPOSE:	Set the offset from the models position...
//
// ----------------------------------------------------------------------- //

void CHitBox::SetOffset( const LTVector &vOffset )
{
	if( !m_hObject )
		return;

	// Just set the offset, Update() will take care of setting the position...
	m_vOffset = vOffset;
}

