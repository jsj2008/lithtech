// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItemFX.cpp
//
// PURPOSE : PickupItem - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PickupItemFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"

extern CGameClientShell* g_pGameClientShell;

#define PICKUPITEM_ROTVEL	0.3333f * MATH_CIRCLE

CPickupItemFX::~CPickupItemFX()
{
	if( m_linkClientFX.IsValid() )
	{
		g_pClientFXMgr->ShutdownClientFX( &m_linkClientFX );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PICKUPITEMCREATESTRUCT* pPICS = (PICKUPITEMCREATESTRUCT*)psfxCreateStruct;

	m_bRotate = pPICS->bRotate;
	m_bBounce = pPICS->bBounce;
	m_nTeamId = pPICS->m_nTeamId;

	// Shutdown any currently playing FX...

	if( m_linkClientFX.IsValid() )
	{
		g_pClientFXMgr->ShutdownClientFX( &m_linkClientFX );
	}


	if( pPICS->sClientFX.length() )
	{
		CLIENTFX_CREATESTRUCT fxInit( pPICS->sClientFX.c_str(), FXFLAG_LOOP, m_hServerObject );
		g_pClientFXMgr->CreateClientFX( &m_linkClientFX, fxInit, true );
	}
	
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Update
//
//	PURPOSE:	Update the pickupitem
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::Update()
{
    if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

    LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();

	if (m_bRotate)
	{
        LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		rRot.Rotate(rRot.Up(), PICKUPITEM_ROTVEL * fDeltaTime);
		g_pLTClient->SetObjectRotation(m_hServerObject, &rRot);
	}

	if (m_bBounce)
	{

	}

	// If we have a ClientFX that is playing hide or show it based on the serverobject...
	
	if( m_linkClientFX.IsValid() )
	{
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags( m_hServerObject, OFT_Flags, dwFlags );

		if( dwFlags & FLAG_VISIBLE )
		{
			m_linkClientFX.GetInstance()->Show();
		}
		else
		{
			m_linkClientFX.GetInstance()->Hide();
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::OnServerMessage
//
//	PURPOSE:	Handle recieving a message from the server...
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return LTFALSE;
	
	uint8 nMsgId = pMsg->Readuint8();

	switch( nMsgId )
	{
		case PUFX_CLIENTFX:
		{
			char szClientFX[256] = {0};

			pMsg->ReadString( szClientFX, ARRAY_LEN( szClientFX ));

			// Shutdown any currently playing FX...

			if( m_linkClientFX.IsValid() )
			{
				g_pClientFXMgr->ShutdownClientFX( &m_linkClientFX );
			}


			if( szClientFX[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( szClientFX, FXFLAG_LOOP, m_hServerObject );
				g_pClientFXMgr->CreateClientFX( &m_linkClientFX, fxInit, true );
			}
		}
		break;

		case PUFX_TEAMID:
		{
			m_nTeamId = pMsg->Readuint8( );
		}
		break;

		default:
		break;
		
	}

	return LTTRUE;
}
