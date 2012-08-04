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
	#include "AttachmentDB.h"

//
// Globals...
//

	CPlayerViewAttachmentMgr	*g_pPVAttachmentMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerViewAttachmentMgr::CPlayerViewAttachmentMgr()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CPlayerViewAttachmentMgr::CPlayerViewAttachmentMgr()
:	m_hObject	( NULL )
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

	// Null the global so other systems know it's no longer valid...
	g_pPVAttachmentMgr = NULL;
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
	LTASSERT( g_pPVAttachmentMgr == NULL, "Trying to init CPlayerViewAttachmentMgr after already being initialized!" );

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
	m_hObject = NULL;
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

	// No need to recrete the attachments if it's just being added to the same model...

	if( (m_hObject == hPVObject) && !m_lstPVAttachments.empty() )
		return;

	// Start with an empty list...
	
	RemovePVAttachments();
	
	// Get the clients CharacterFX so we know which model we are using...

	CCharacterFX* pCharFX = g_pGameClientShell->GetLocalCharacterFX();
	if( !pCharFX )
		return;

	// Check how many player view attachments we need to create...

	ModelsDB::HMODEL hModel = pCharFX->GetModel();
	uint32 nNumPVAttachments = g_pModelsDB->GetNumPlayerViewAttachments( hModel );
	if( nNumPVAttachments == 0 )
		return;

	uint32 dwFlags = 0;
	g_pCommonLT->GetObjectFlags( hPVObject, OFT_Flags, dwFlags );

	// We need to create attachments so assign the model we are attaching to...

	m_hObject = hPVObject;

	const char *pszPVAttachmentPosition = NULL;
	HRECORD hPVAttachment = NULL;

	for( uint32 i = 0; i < nNumPVAttachments; ++i )
	{
		g_pModelsDB->GetPlayerViewAttachment( hModel, i, pszPVAttachmentPosition, hPVAttachment );
		
		CreatePVAttachment( pszPVAttachmentPosition, hPVAttachment );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerViewAttachmentMgr::CreatePVAttachment()
//
//  PURPOSE:	Create an attachment at a given socket...
//
// ----------------------------------------------------------------------- //

void CPlayerViewAttachmentMgr::CreatePVAttachment( const char *pszPVAttachmentPosition, HRECORD hPVAttachmentRecord )
{
	if( !pszPVAttachmentPosition || !hPVAttachmentRecord || !m_hObject )
		return;

	CPlayerViewAttachment pva;

	// Make sure the attachment position socket actually exists on the weapon model...

	if( LT_OK != g_pModelLT->GetSocket( m_hObject, pszPVAttachmentPosition, pva.m_hSocket ))
	{
		return;
	}
	
	// The socket is good, now create the model...

	ObjectCreateStruct ocs;
	
	AttachmentDB::Instance().GetPVAttachmentModel( hPVAttachmentRecord, ocs.m_Filename, ARRAY_LEN( ocs.m_Filename ));
	int nMaterials = AttachmentDB::Instance().GetPVAttachmentMaterialCount(hPVAttachmentRecord);
	for ( int iEachMaterial = 0; iEachMaterial < nMaterials; ++iEachMaterial)
	{
		AttachmentDB::Instance().GetPVAttachmentMaterial(hPVAttachmentRecord, iEachMaterial, ocs.m_Materials[iEachMaterial], ARRAY_LEN( ocs.m_Materials[iEachMaterial] ));
	}
	
	ocs.m_ObjectType = OT_MODEL;

	if( AttachmentDB::Instance().GetPVAttachmentTranslcuent( hPVAttachmentRecord ))
	{
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;
	}

	pva.m_hModel = g_pLTClient->CreateObject( &ocs );
	if( !pva.m_hModel )
		return;

	float flScale = AttachmentDB::Instance().GetPVAttachmentScale( hPVAttachmentRecord );
	g_pLTClient->SetObjectScale( pva.m_hModel, flScale );

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
			LTTransform transform;
			if ( LT_OK == g_pModelLT->GetSocketTransform( m_hObject,
			                                              PlayerViewAttachment.m_hSocket,
			                                              transform,
			                                              true ) )
			{
				LTVector vPos = transform.m_vPos;
				LTRotation rRot = transform.m_rRot;
				
				g_pLTClient->SetObjectTransform( PlayerViewAttachment.m_hModel, LTRigidTransform(vPos, rRot) );
				
				if (transform.m_fScale != 1.0f)
					g_pLTClient->SetObjectScale( PlayerViewAttachment.m_hModel, transform.m_fScale );
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
	m_hObject = NULL;
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

	PlayerViewAttachment.m_hModel	= NULL;
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

