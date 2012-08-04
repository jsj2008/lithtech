// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerViewAttachmentMgr.cpp
//
// PURPOSE : Manager of objects attached to player-view models...
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "CMoveMgr.h"
	#include "CharacterFX.h"
	#include "PlayerViewAttachmentMgr.h"

//
// Globals...
//

	CPlayerViewAttachmentMgr	*g_pPVAttachmentMgr = LTNULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerViewAttachmentMgr::CPlayerViewAttachmentMgr()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CPlayerViewAttachmentMgr::CPlayerViewAttachmentMgr()
:	m_hObject	( LTNULL )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerViewAttachmentMgr::CPlayerViewAttachmentMgr()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CPlayerViewAttachmentMgr::~CPlayerViewAttachmentMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerViewAttachmentMgr::Init()
//
//	PURPOSE:	Initialize the mgr...
//
// ----------------------------------------------------------------------- //

bool CPlayerViewAttachmentMgr::Init()
{
	ASSERT( !g_pPVAttachmentMgr );

	if( g_pPVAttachmentMgr )
	{
		return false;
	}

	g_pPVAttachmentMgr = this;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerViewAttachmentMgr::Term()
//
//	PURPOSE:	Clean up...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::Term()
{
	RemovePVAttachments();
	m_hObject = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::CreatePVAttachments()
//
//  PURPOSE:	Create any attachment models that we should have...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::CreatePVAttachments( HOBJECT hPVObject )
{
	ASSERT( g_pPlayerMgr->GetMoveMgr() );
	ASSERT( g_pModelButeMgr );

	// No need to recrete the attachments if it's just being added to the same model...

	if( (m_hObject == hPVObject) && !m_lstPVAttachments.empty() )
		return;

	// Start with an empty list...
	
	RemovePVAttachments();
	
	// Get the clients CharacterFX so we know which model we are using...

	CCharacterFX* pCharFX = g_pPlayerMgr->GetMoveMgr()->GetCharacterFX();
	if( !pCharFX )
		return;

	// Check how many player view attachments we need to create...

	ModelId eModelId = pCharFX->GetModelId();
	uint8 nNumPVAttachments = g_pModelButeMgr->GetNumPlayerViewAttachments( eModelId );
	if( nNumPVAttachments == 0 )
		return;

	uint32 dwFlags = 0;
	g_pCommonLT->GetObjectFlags( hPVObject, OFT_Flags, dwFlags );

	// It must be a player view model...

	if( !(dwFlags & FLAG_REALLYCLOSE) )
		return;

	// We need to create attachments so assign the model we are attaching to...

	m_hObject = hPVObject;

	const char *pszPVAttachmentPosition = LTNULL;
	const char *pszPVAttachment = LTNULL;

	for( uint8 i = 0; i < nNumPVAttachments; ++i )
	{
		g_pModelButeMgr->GetPlayerViewAttachment( eModelId, i, pszPVAttachmentPosition, pszPVAttachment );
		
		CreatePVAttachment( pszPVAttachmentPosition, pszPVAttachment );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::CreatePVAttachment()
//
//  PURPOSE:	Create an attachment at a given socket...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::CreatePVAttachment( const char *pszPVAttachmentPosition, const char *pszPVAttachment )
{
	ASSERT( g_pAttachButeMgr );

	if( !pszPVAttachmentPosition || !pszPVAttachment || !m_hObject )
		return;

	CPlayerViewAttachment pva;

	// Make sure the attachment position socket actually exists on the weapon model...

	if( LT_OK != g_pModelLT->GetSocket( m_hObject, pszPVAttachmentPosition, pva.m_hSocket ))
	{
		return;
	}
	
	// Make sure the attachment record actually exists...
	
	int nAttachmentId = g_pAttachButeMgr->GetPVAttachmentIDByName( pszPVAttachment );
	if( nAttachmentId < 0 )
		return;

	char szBuf[MAX_CS_FILENAME_LEN] = {0};

	// The socket is good, now create the model...

	ObjectCreateStruct ocs;
	
	g_pAttachButeMgr->GetPVAttachmentModel( nAttachmentId, ocs.m_Filename, ARRAY_LEN( ocs.m_Filename ));
	g_pAttachButeMgr->CopyPVAttachmentSkins( nAttachmentId, ocs.m_SkinNames[0], ARRAY_LEN( ocs.m_SkinNames[0] ));
	g_pAttachButeMgr->CopyPVAttachmentRenderStyles( nAttachmentId, ocs.m_RenderStyleNames[0], ARRAY_LEN( ocs.m_RenderStyleNames[0] ));
	
	ocs.m_ObjectType = OT_MODEL;
	ocs.m_Flags     |= FLAG_REALLYCLOSE;
	ocs.m_Flags2    |= FLAG2_DYNAMICDIRLIGHT;

	if( g_pAttachButeMgr->GetPVAttachmentTranslcuent( nAttachmentId ))
	{
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;
	}

	pva.m_hModel = g_pLTClient->CreateObject( &ocs );
	if( !pva.m_hModel )
		return;

	LTVector vScale = g_pAttachButeMgr->GetPVAttachmentScale( nAttachmentId );
	g_pLTClient->SetObjectScale( pva.m_hModel, &vScale );
	g_pLTClient->SetModelLooping( pva.m_hModel, false );

	// The attachment is set up so give it an Update and add it to our list...

	m_lstPVAttachments.push_back( pva );
	UpdatePVAttachment( m_lstPVAttachments.size() - 1);

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::UpdatePVAttachments()
//
//  PURPOSE:	Update all the player view attachments to the propper position and rotation...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::UpdatePVAttachments()
{
	for( uint8 i = 0; i < m_lstPVAttachments.size(); ++i )
	{
		UpdatePVAttachment( i );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::UpdatePVAttachment()
//
//  PURPOSE:	Update the player view attachment to the propper position and rotation...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::UpdatePVAttachment( uint8 nPVAttachIndex )
{
	if( nPVAttachIndex >= m_lstPVAttachments.size() )
		return;

	if( !m_hObject )
		return;
	
	CPlayerViewAttachment PlayerViewAttachment = m_lstPVAttachments[nPVAttachIndex];

	if( PlayerViewAttachment.m_hModel )
	{
		if( INVALID_MODEL_SOCKET != PlayerViewAttachment.m_hSocket )
		{
			LTransform transform;
			if ( LT_OK == g_pModelLT->GetSocketTransform( m_hObject,
			                                              PlayerViewAttachment.m_hSocket,
			                                              transform,
			                                              LTTRUE ) )
			{
				LTVector vPos = transform.m_Pos;
				LTRotation rRot = transform.m_Rot;
				
				g_pLTClient->SetObjectPos( PlayerViewAttachment.m_hModel, &vPos );
				g_pLTClient->SetObjectRotation( PlayerViewAttachment.m_hModel, &rRot );
				
				if (transform.m_Scale.x != 1.0f || transform.m_Scale.y != 1.0f || transform.m_Scale.z != 1.0f)
					g_pLTClient->SetObjectScale( PlayerViewAttachment.m_hModel, &transform.m_Scale );
			}
		}
		else
		{
			ShowPVAttachment( nPVAttachIndex, false );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::RemovePVAttachments()
//
//  PURPOSE:	Remove the player view attachment objects and empty our list...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::RemovePVAttachments()
{
	for( uint8 i = 0; i < m_lstPVAttachments.size(); ++i )
	{
		RemovePVAttachment( i );
	}

	m_lstPVAttachments.clear();
	m_hObject = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::RemovePVAttachment()
//
//  PURPOSE:	Remove the player view attachment object...
//				This does NOT remove it from our player-view attachment list.
//				You should erase it after the call to RemovePVAttachment()
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::RemovePVAttachment( uint8 nPVAttachIndex )
{
	if( nPVAttachIndex >= m_lstPVAttachments.size() )
		return;
	
	CPlayerViewAttachment PlayerViewAttachment = m_lstPVAttachments[nPVAttachIndex];

	if( PlayerViewAttachment.m_hModel )
	{
		g_pLTClient->RemoveObject( PlayerViewAttachment.m_hModel );
	}

	PlayerViewAttachment.m_hModel	= LTNULL;
	PlayerViewAttachment.m_hSocket	= INVALID_MODEL_SOCKET;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::ShowPVAttachments()
//
//  PURPOSE:	Hide/Show the player view attachment objects...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::ShowPVAttachments( bool bVisible /* = true  */ )
{
	for( uint8 i = 0; i < m_lstPVAttachments.size(); ++i )
	{
		// Save whether the attachment was hidden before this call..

		bool bHidden = m_lstPVAttachments[i].m_bHidden;
		
		if( !bHidden )
			ShowPVAttachment( i, bVisible );

		// Keep the hidden state 

		m_lstPVAttachments[i].m_bHidden = bHidden;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::ShowPVAttachments()
//
//  PURPOSE:	Hide/Show the player view attachment object...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::ShowPVAttachment( uint8 nPVAttachIndex, bool bVisible /* = true  */ )
{
	if( nPVAttachIndex >= m_lstPVAttachments.size() )
		return;
	
	HOBJECT hModel = m_lstPVAttachments[nPVAttachIndex].m_hModel;
	
	if( hModel )
	{
		g_pCommonLT->SetObjectFlags( hModel, OFT_Flags, (bVisible ? FLAG_VISIBLE : 0), FLAG_VISIBLE );
	}

	m_lstPVAttachments[nPVAttachIndex].m_bHidden = !bVisible;
}

