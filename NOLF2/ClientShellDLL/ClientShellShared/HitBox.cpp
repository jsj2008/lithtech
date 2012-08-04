// ----------------------------------------------------------------------- //
//
// MODULE  : HitBox.cpp
//
// PURPOSE : Client side reresentation of the CCharacterHitBox object
//
// CREATED : 8/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "HitBox.h"

//
// Defines...
//

	#define	HB_COLOR_R	0.5f
	#define HB_COLOR_G	0.5f
	#define HB_COLOR_B	0.5f
	#define HB_COLOR_A	1.0f

static VarTrack	s_ShowClientHitBox;


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
	m_vOffset		( 0.0f, 0.0f, 0.0f ),
	m_hBoundingBox	( INVALID_HOBJECT )
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

	if( m_hBoundingBox != INVALID_HOBJECT )
	{
		g_pLTClient->RemoveObject( m_hBoundingBox );
	}
	m_hBoundingBox = INVALID_HOBJECT;
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

	if( !s_ShowClientHitBox.IsInitted() )
	{
		s_ShowClientHitBox.Init( g_pLTClient, "ShowClientHitBox", LTNULL, 0.0f );
	}

	m_hModel	= hModel;
	m_vDims		= vDims;
	m_vOffset	= vOffset;

	// Create the hitbox at the propper offset from the models position...

	LTVector vModelPos;
	g_pLTClient->GetObjectPos( m_hModel, &vModelPos );

	LTRotation rModelRot;
	g_pLTClient->GetObjectRotation( m_hModel, &rModelRot );

	LTMatrix mMat;
	rModelRot.ConvertToMatrix( mMat );

	// Get rid of our object if it already exists...

	if( m_hObject )
	{
		g_pLTClient->RemoveObject( m_hObject );
		m_hObject = LTNULL;
	}

	ObjectCreateStruct ocs;
	
	ocs.m_ObjectType	= OT_NORMAL;
	ocs.m_Flags			= FLAG_RAYHIT;
	ocs.m_Pos			= vModelPos + (mMat * m_vOffset);
	
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

void CHitBox::Update()
{
	if( (m_hObject == INVALID_HOBJECT) ||
		(m_hModel == INVALID_HOBJECT) )
	{
		return;
	}

	// Offset the HitBox from the position of the associated model...
	
	LTVector vPos;
	g_pLTClient->GetObjectPos( m_hModel, &vPos );

	LTRotation rRot;
	g_pLTClient->GetObjectRotation( m_hModel, &rRot );

	LTMatrix mMat;
	rRot.ConvertToMatrix( mMat );

	vPos += (mMat * m_vOffset);

	g_pLTClient->SetObjectPos( m_hObject, &vPos );

	// If the model object is not visible we should not be able to interact with the hitbox

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hModel, OFT_Flags, dwFlags );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, ((dwFlags & FLAG_VISIBLE) ? FLAG_RAYHIT : 0), FLAG_RAYHIT );
	

#ifndef _FINAL
	// Update the visual model...
	UpdateBoundingBox();
#endif // _FINAL

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
	if( m_hObject == INVALID_HOBJECT )
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
	if( m_hObject == INVALID_HOBJECT )
		return;

	// Just set the offset, Update() will take care of setting the position...

	m_vOffset = vOffset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::CreateBoundingBox()
//
//	PURPOSE:	Create a model for some visual feedback of the hitbox...
//
// ----------------------------------------------------------------------- //

void CHitBox::CreateBoundingBox()
{
	if( (m_hObject == INVALID_HOBJECT) ||
		(m_hBoundingBox != INVALID_HOBJECT ))
		return;

	ObjectCreateStruct ocs;

	g_pLTClient->GetObjectPos( m_hObject, &ocs.m_Pos );
	
	ocs.m_ObjectType	= OT_MODEL;
	ocs.m_Flags			= FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_GOTHRUWORLD;
	ocs.m_Flags2		= FLAG2_FORCETRANSLUCENT;
	ocs.m_Scale			= m_vDims * 2.0f;

	LTStrCpy( ocs.m_Filename, "Models\\1x1_square.ltb", ARRAY_LEN( ocs.m_Filename ));
	LTStrCpy( ocs.m_SkinName, "Models\\1x1_square.dtx", ARRAY_LEN( ocs.m_SkinName ));
	
	m_hBoundingBox = g_pLTClient->CreateObject( &ocs );
	if( m_hBoundingBox == INVALID_HOBJECT )
		return;

	g_pLTClient->SetObjectColor( m_hBoundingBox, HB_COLOR_R, HB_COLOR_G, HB_COLOR_B, HB_COLOR_A);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::UpdateBoundingBox()
//
//	PURPOSE:	Update the model for some visual feedback of the hitbox...
//
// ----------------------------------------------------------------------- //

void CHitBox::UpdateBoundingBox()
{
	if( m_hObject == INVALID_HOBJECT )
		return;

	if( s_ShowClientHitBox.GetFloat( 0.0f ) > 0.0f )
	{
		CreateBoundingBox();

		if( m_hBoundingBox == INVALID_HOBJECT )
			return;

		LTVector vPos;

		g_pLTClient->GetObjectPos( m_hObject, &vPos );
		g_pLTClient->SetObjectPos( m_hBoundingBox, &vPos );

		LTVector vScale = m_vDims * 2.0f;
		g_pLTClient->SetObjectScale( m_hBoundingBox, &vScale );
		
		g_pLTClient->SetObjectColor( m_hBoundingBox, HB_COLOR_R, HB_COLOR_G, HB_COLOR_B, HB_COLOR_A );	
	}
	else
	{
		if( m_hBoundingBox != INVALID_HOBJECT )
		{
			g_pLTClient->RemoveObject( m_hBoundingBox );
		}
		m_hBoundingBox = INVALID_HOBJECT;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHitBox::SetCanBeSearched()
//
//	PURPOSE:	Set the hitbox to be searchable...
//
// ----------------------------------------------------------------------- //

void CHitBox::SetCanBeSearched( bool bCanBeSearched )
{
	if( m_hObject == INVALID_HOBJECT )
		return;

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, (bCanBeSearched ? USRFLG_CAN_SEARCH : 0), USRFLG_CAN_SEARCH );
}
