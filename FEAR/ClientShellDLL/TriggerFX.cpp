// ----------------------------------------------------------------------- //
//
// MODULE  : TriggerFX.h
//
// PURPOSE : Trigger special fx class - Definition
//
// CREATED : 5/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "CharacterFX.h"
	#include "TriggerTypeDB.h"
	#include "TriggerFX.h"
	#include "resourceextensions.h"
	#include "HUDTransmission.h"
	#include "HUDMessageQueue.h"
	#include "PlayerCamera.h"

#define	TRIGFX_DIMS_MODEL	"Models\\1x1_square." RESEXT_MODEL_PACKED
#define TRIGFX_DIMS_MATERIAL	"Models\\1x1_square." RESEXT_MATERIAL

/*
static bool TriggerDimsFilterFn( HOBJECT hTest, void* pUserData )
{
	if( hTest == (HOBJECT)pUserData )
		return true;

	if( IsMainWorld( hTest ) || (GetObjectType( hTest ) == OT_WORLDMODEL) )
		return true;

	return false;
}
*/

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTriggerFX::CTriggerFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTriggerFX::CTriggerFX( )
:	CSpecialFX					(),
	m_bWithinIndicatorRadius	( false ),
	m_fDistPercent				( -1.0f ),
	m_hDimsObject				( NULL )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTriggerFX::~CTriggerFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTriggerFX::~CTriggerFX()
{
	if( m_hDimsObject )
		g_pLTClient->RemoveObject( m_hDimsObject );
	m_hDimsObject = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the trigger fx
//
// ----------------------------------------------------------------------- //

bool CTriggerFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !hServObj ) return false;
	if( !CSpecialFX::Init( hServObj, pMsg )) return false;

	m_cs.hServerObj = hServObj;
	m_cs.Read( pMsg );

	ObjectCreateStruct ocs;
	
	LTVector vPos;
	g_pLTClient->GetObjectPos( m_hServerObject, &vPos );

	ocs.m_Pos = vPos;
	
	ocs.SetFileName(TRIGFX_DIMS_MODEL );
	ocs.SetMaterial(0, TRIGFX_DIMS_MATERIAL );

	ocs.m_ObjectType = OT_MODEL;
	ocs.m_Flags = FLAG_NOLIGHT | FLAG_GOTHRUWORLD;
	
	m_hDimsObject = g_pLTClient->CreateObject( &ocs );
	if( !m_hDimsObject )
		return false;

	LTVector vDims = m_cs.vDims;
	g_pPhysicsLT->SetObjectDims( m_hDimsObject, &vDims, 0 );

	if( NULL != m_cs.hTriggerTypeRecord )
	{
		m_hIcon.Load( TriggerTypeDB::Instance().GetIconTexture(m_cs.hTriggerTypeRecord) );
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

bool CTriggerFX::Update()
{
	if( !m_hServerObject || m_bWantRemove || !m_hDimsObject ) return false;

	CalcLocalClientDistance();

	CheckPlayersWithinTrigger();
	
	return true;	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

bool CTriggerFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return false;

	uint8 nMsgId = pMsg->Readuint8();

	switch( nMsgId )
	{
		case TRIGFX_ALLFX_MSG :
		{
			// Re-init our data...

			m_cs.Read( pMsg );
		}
		break;

		case TRIGFX_LOCKED_MSG :
		{
			m_cs.bLocked = !!(pMsg->Readuint8());
		}
		break;
	}
	 
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerFX::CalcLocalClientDistance
//
//	PURPOSE:	Calculates the distance to from the local player to the trigger...
//
// ----------------------------------------------------------------------- //

void CTriggerFX::CalcLocalClientDistance()
{
	m_fDistPercent = -1.0f;

	// Don't do anything if the trigger is locked or our distances are too small..

	if( m_cs.bLocked || (m_cs.fHUDAlwaysOnDist <= 0.0f && m_cs.fHUDLookAtDist <= 0.0f) )
		return;

	// See if the player is within the trigger...

	LTVector vTrigPos;
	g_pLTClient->GetObjectPos( m_hServerObject, &vTrigPos );
	g_pLTClient->SetObjectPos( m_hDimsObject, vTrigPos );

	HLOCALOBJ	hPlayerObj = g_pLTClient->GetClientObject();
	LTVector	vPlayerPos, vPlayerDims;

	g_pLTClient->GetObjectPos( hPlayerObj, &vPlayerPos );

	// Make sure we are within the display radius...
	
	float	fMaxRadius = LTMAX( m_cs.fHUDAlwaysOnDist, m_cs.fHUDLookAtDist );
	float	fDistSqr = vTrigPos.DistSqr( vPlayerPos );
	bool	bWithinLookAtDist = (fDistSqr < m_cs.fHUDLookAtDist * m_cs.fHUDLookAtDist);
	bool	bWithinAlwaysOnDist = (fDistSqr < m_cs.fHUDAlwaysOnDist * m_cs.fHUDAlwaysOnDist);

	if( !bWithinLookAtDist && !bWithinAlwaysOnDist )
	{
		// We are not close enough...
		
		m_bWithinIndicatorRadius = false;
		return;
	}

	m_bWithinIndicatorRadius = true;

	g_pPhysicsLT->GetObjectDims( hPlayerObj, &vPlayerDims );

	LTVector vTrigMin = vTrigPos - m_cs.vDims;
	LTVector vTrigMax = vTrigPos + m_cs.vDims;
	LTVector vPlayerMin = vPlayerPos - vPlayerDims;
	LTVector vPlayerMax = vPlayerPos + vPlayerDims;

	// Check if we are within the height of the trigger...

	bool bWithinHeight =false;
	if( vPlayerMax.y > vTrigMin.y && vPlayerMin.y < vTrigMax.y )
		bWithinHeight = true;
	
	// See if we are inside the trigger at all...

	if( bWithinHeight && (BoxesIntersect( vTrigMin, vTrigMax, vPlayerMin, vPlayerMax ) || bWithinAlwaysOnDist))
	{
		m_fDistPercent = 1.0f;
	}
	else
	{
		// We are within the height of the trigger, show how far from it we are...

		float fMinDist = (vPlayerDims.x + vPlayerDims.z) * 0.5f;
		float fMaxDist = 100000.0f;

		LTVector vDir;

		if( bWithinAlwaysOnDist )
		{
			vDir = vTrigPos - vPlayerPos;
			vDir.Normalize();
		}
		else
		{
			LTRotation const& rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );
			vDir = rRot.Forward();
		}

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From		= vPlayerPos + (vDir * fMinDist);
		IQuery.m_To			= IQuery.m_From + (vDir * fMaxDist);
		IQuery.m_Flags		= INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
				
		// We need to recieve rayhits for this intersect call...

		g_pCommonLT->SetObjectFlags( m_hDimsObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT );

		if( g_pLTClient->IntersectSegment( IQuery, &IInfo ))
		{
			if( IInfo.m_hObject == m_hDimsObject )
			{
				IInfo.m_Point.y = vPlayerPos.y;
				float fDist = vPlayerPos.Dist( IInfo.m_Point );
				m_fDistPercent = 1.0f - (fDist / fMaxRadius);
			}
		}

		// No more rayhits...
		
		g_pCommonLT->SetObjectFlags( m_hDimsObject, OFT_Flags, 0, FLAG_RAYHIT );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerFX::CheckPlayersWithinTrigger
//
//	PURPOSE:	Checks to see which players are inside the trigger
//				and sends transmissions accordingly...
//
// ----------------------------------------------------------------------- //

void CTriggerFX::CheckPlayersWithinTrigger()
{
	if( m_cs.bLocked )
		return;

	// Get a list of all the characters...

	CSpecialFXList *pList = g_pGameClientShell->GetSFXMgr()->GetFXList( SFX_CHARACTER_ID );
	if( !pList )
		return;

	int nListSize = pList->GetSize();
	int nNumChars = pList->GetNumItems();
	int nNumFoundChars = 0;
	int nNumPlayersFound = 0;
	uint32 dwLocalId = 0;

	g_pLTClient->GetLocalClientID( &dwLocalId );

	LTVector vTrigPos, vPlayerPos, vPlayerDims, vPlayerMin, vPlayerMax;
	g_pLTClient->GetObjectPos( m_hServerObject, &vTrigPos );

	// Setup the triggers box...
	
	LTVector vTrigMin = vTrigPos - m_cs.vDims;
	LTVector vTrigMax = vTrigPos + m_cs.vDims;

	bool bLocalPlayerIn = false;
	
	// Initialize our containers to zero.  Don't call clear, since we'll be using
	// these vectors every frame and most likely they will have the same
	// number of elements across multiple frames.
	m_lstPlayersNotInTrigger.resize( 0 );
	m_lstNewPlayersInTrigger.resize( 0 );

	for( int i = 0; i < nListSize; ++i )
	{
		// Try not to go through the entire list...

		if( nNumFoundChars == nNumChars )
			break;

		if( (*pList)[i] )
		{
			CCharacterFX *pChar = (CCharacterFX*)(*pList)[i];
			if( !pChar )
				continue;

			// Found another char..
			++nNumFoundChars;

			if( pChar->m_cs.bIsPlayer && pChar->m_cs.nClientID != ( uint8 )-1 )
			{
				++nNumPlayersFound;
				
				HOBJECT hPlayer = pChar->GetServerObj();

				g_pLTClient->GetObjectPos( hPlayer, &vPlayerPos );
				g_pPhysicsLT->GetObjectDims( hPlayer, &vPlayerDims );

				vPlayerMin = vPlayerPos - vPlayerDims;
				vPlayerMax = vPlayerPos + vPlayerDims;

				// Check the current list of players in the trigger for this player...
					
				CharFXList::iterator iter;
				for( iter = m_lstCurPlayersInTrigger.begin(); iter != m_lstCurPlayersInTrigger.end(); ++iter )
				{
					if( pChar == (*iter) )
						break;
				}

				// Check if we are within the height of the trigger...

				bool bWithinHeight = false;
				if( vPlayerMax.y > vTrigMin.y && vPlayerMin.y < vTrigMax.y )
					bWithinHeight = true;

				if( bWithinHeight && BoxesIntersect( vTrigMin, vTrigMax, vPlayerMin, vPlayerMax ) && !pChar->IsPlayerDead())
				{
					if( dwLocalId == pChar->m_cs.nClientID )
						bLocalPlayerIn = true;

					// If it wasn't in the list add it...

					if( iter == m_lstCurPlayersInTrigger.end() )
					{
						m_lstCurPlayersInTrigger.push_back( pChar );
						m_lstNewPlayersInTrigger.push_back( pChar );
					}

				}
				else
				{
					if( iter != m_lstCurPlayersInTrigger.end() )
						m_lstCurPlayersInTrigger.erase( iter );

					m_lstPlayersNotInTrigger.push_back( pChar );
				}
			}
		}
	}

	wchar_t wszBuffer[256];

	if( (m_lstNewPlayersInTrigger.size() > 0) && (nNumPlayersFound > 1) )
	{
		CClientInfoMgr *pInfoMgr = g_pInterfaceMgr->GetClientInfoMgr();
		if( !pInfoMgr )
			return;

		if( bLocalPlayerIn )
		{
			// Display a general transmission and messages for each player you are waiting for...

			int nPlayersNotInTrig = m_lstPlayersNotInTrigger.size();

			if( m_cs.nPlayerInsideID != (uint32)-1 )
			{
				g_pTransmission->Show( StringIDFromIndex(m_cs.nPlayerInsideID) );
			}
			else if( nPlayersNotInTrig > 1 )
			{
				//sTransmission.Format( "You are waiting for %i players.", nPlayersNotInTrig );
				FormatString( "IDS_EXIT_PLAYER_WAITING", wszBuffer, LTARRAYSIZE(wszBuffer), nPlayersNotInTrig );
				g_pTransmission->Show( wszBuffer );
			}
			else
			{
				//sTransmission.Format( "You are waiting for 1 player." );
				FormatString( "IDS_EXIT_PLAYER_WAITING_1", wszBuffer, LTARRAYSIZE(wszBuffer) );
				g_pTransmission->Show( wszBuffer );
			}		
			
			
			CharFXList::iterator iter;
			for( iter = m_lstPlayersNotInTrigger.begin(); iter != m_lstPlayersNotInTrigger.end(); ++iter )
			{
				//sMessage.Format( "You are waiting for %s.", pInfoMgr->GetPlayerName( (*iter)->m_cs.nClientID ));
				FormatString( "IDS_EXIT_PLAYER_WAITING_NAME", wszBuffer, LTARRAYSIZE(wszBuffer), pInfoMgr->GetPlayerName( (*iter)->m_cs.nClientID) );
				g_pGameMsgs->AddMessage( wszBuffer );
			}
		}
		else
		{
			// Display a general transmission and messages for each player waiting for you...

			int nPlayersInTrig = m_lstCurPlayersInTrigger.size();
			
			if( m_cs.nPlayerOutsideID != (uint32)-1 )
			{
				g_pTransmission->Show( LoadString(m_cs.nPlayerOutsideID) );
			}
			else if( nPlayersInTrig > 1 )
			{
//				sTransmission.Format( "%i players are waiting for you",nPlayersInTrig  );
				FormatString( "IDS_EXIT_WAITING", wszBuffer, LTARRAYSIZE(wszBuffer), nPlayersInTrig );
				g_pTransmission->Show( wszBuffer );
			}
			else
			{
//				sTransmission.Format( "1 player is waiting for you." );
				FormatString( "IDS_EXIT_WAITING_1", wszBuffer, LTARRAYSIZE(wszBuffer) );
				g_pTransmission->Show( wszBuffer );
			}
			

			CharFXList::iterator iter;
			for( iter = m_lstCurPlayersInTrigger.begin(); iter != m_lstCurPlayersInTrigger.end(); ++iter )
			{
				FormatString( "IDS_EXIT_WAITING_NAME", wszBuffer, LTARRAYSIZE(wszBuffer), pInfoMgr->GetPlayerName( (*iter)->m_cs.nClientID) );
				g_pGameMsgs->AddMessage( wszBuffer );	
			}
		}
	}
}