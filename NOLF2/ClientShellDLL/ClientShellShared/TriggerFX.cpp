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
	#include "TriggerTypeMgr.h"
	#include "TriggerFX.h"
	#include "ClientResShared.h"


#define	TRIGFX_DIMS_MODEL	"Models\\1x1_square.ltb"
#define TRIGFX_DIMS_SKIN	"Models\\1x1_square.dtx"

static bool TriggerDimsFilterFn( HOBJECT hTest, void *pUserData )
{
	if( hTest == (HOBJECT)pUserData )
		return true;

	if( IsMainWorld( hTest ) || (GetObjectType( hTest ) == OT_WORLDMODEL) )
		return true;

	return false;
}

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
	m_hDimsObject				( LTNULL ),
	m_hIcon						( LTNULL )
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
	m_hDimsObject = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the trigger fx
//
// ----------------------------------------------------------------------- //

LTBOOL CTriggerFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !hServObj ) return LTFALSE;
	if( !CSpecialFX::Init( hServObj, pMsg )) return LTFALSE;

	m_cs.hServerObj = hServObj;
	m_cs.Read( pMsg );

	ObjectCreateStruct ocs;
	
	LTVector vPos;
	g_pLTClient->GetObjectPos( m_hServerObject, &vPos );

	ocs.m_Pos = vPos;
	
	SAFE_STRCPY( ocs.m_Filename, TRIGFX_DIMS_MODEL );
	SAFE_STRCPY( ocs.m_SkinName, TRIGFX_DIMS_SKIN );

	ocs.m_ObjectType = OT_MODEL;
	ocs.m_Flags = FLAG_NOLIGHT | FLAG_GOTHRUWORLD;
	
	m_hDimsObject = g_pLTClient->CreateObject( &ocs );
	if( !m_hDimsObject )
		return LTFALSE;

	LTVector vDims = m_cs.vDims;
	g_pPhysicsLT->SetObjectDims( m_hDimsObject, &vDims, 0 );

	if( m_cs.nTriggerTypeId != TTMGR_INVALID_ID )
	{
		TRIGGERTYPE *pType = g_pTriggerTypeMgr->GetTriggerType( m_cs.nTriggerTypeId );
		if( pType )
		{
			m_hIcon = g_pInterfaceResMgr->GetTexture( pType->szIcon );
		}
	}
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CTriggerFX::Update()
{
	if( !m_hServerObject || m_bWantRemove || !m_hDimsObject ) return LTFALSE;

	CalcLocalClientDistance();

	CheckPlayersWithinTrigger();
	
	return LTTRUE;	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CTriggerFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return LTFALSE;

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
	 
	return LTTRUE;
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
	g_pLTClient->SetObjectPos( m_hDimsObject, &vTrigPos );

	HLOCALOBJ	hPlayerObj = g_pLTClient->GetClientObject();
	LTVector	vPlayerPos, vPlayerDims;

	g_pLTClient->GetObjectPos( hPlayerObj, &vPlayerPos );

	// Make sure we are within the display radius...
	
	float	fMaxRadius = Max( m_cs.fHUDAlwaysOnDist, m_cs.fHUDLookAtDist );
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
			LTRotation rRot;
			g_pLTClient->GetObjectRotation( g_pPlayerMgr->GetCamera(), &rRot );

			vDir = rRot.Forward();
		}

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From		= vPlayerPos + (vDir * fMinDist);
		IQuery.m_To			= IQuery.m_From + (vDir * fMaxDist);
		IQuery.m_Flags		= INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
				
		// We need to recieve rayhits for this intersect call...

		g_pCommonLT->SetObjectFlags( m_hDimsObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT );

		if( g_pLTClient->IntersectSegment( &IQuery, &IInfo ))
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
	
	m_lstPlayersNotInTrigger.clear();
	m_lstNewPlayersInTrigger.clear();

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

			if( pChar->m_cs.bIsPlayer )
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

	if( (m_lstNewPlayersInTrigger.size() > 0) && (nNumPlayersFound > 1) )
	{
		CClientInfoMgr *pInfoMgr = g_pInterfaceMgr->GetClientInfoMgr();
		if( !pInfoMgr )
			return;

		CString sTransmission, sMessage;

		if( bLocalPlayerIn )
		{
			// Display a general transmission and messages for each player you are waiting for...

			int nPlayersNotInTrig = m_lstPlayersNotInTrigger.size();

			if( m_cs.nPlayerInsideID != (uint32)-1 )
			{
				g_pTransmission->Show( m_cs.nPlayerInsideID );
			}
			else if( nPlayersNotInTrig > 1 )
			{
				//sTransmission.Format( "You are waiting for %i players.", nPlayersNotInTrig );
				g_pTransmission->Show( FormatTempString(IDS_EXIT_PLAYER_WAITING,nPlayersNotInTrig) );
			}
			else
			{
				//sTransmission.Format( "You are waiting for 1 player." );
				g_pTransmission->Show( FormatTempString(IDS_EXIT_PLAYER_WAITING_1) );
			}		
			
			
			CharFXList::iterator iter;
			for( iter = m_lstPlayersNotInTrigger.begin(); iter != m_lstPlayersNotInTrigger.end(); ++iter )
			{
				//sMessage.Format( "You are waiting for %s.", pInfoMgr->GetPlayerName( (*iter)->m_cs.nClientID ));
				g_pChatMsgs->AddMessage( FormatTempString(IDS_EXIT_PLAYER_WAITING_NAME,pInfoMgr->GetPlayerName( (*iter)->m_cs.nClientID)) );
			}
		}
		else
		{
			// Display a general transmission and messages for each player waiting for you...

			int nPlayersInTrig = m_lstCurPlayersInTrigger.size();
			
			if( m_cs.nPlayerOutsideID != (uint32)-1 )
			{
				g_pTransmission->Show( m_cs.nPlayerOutsideID );
			}
			else if( nPlayersInTrig > 1 )
			{
//				sTransmission.Format( "%i players are waiting for you",nPlayersInTrig  );
				g_pTransmission->Show( FormatTempString(IDS_EXIT_WAITING,nPlayersInTrig) );
			}
			else
			{
//				sTransmission.Format( "1 player is waiting for you." );
				g_pTransmission->Show( FormatTempString(IDS_EXIT_WAITING_1) );
			}
			

			CharFXList::iterator iter;
			for( iter = m_lstCurPlayersInTrigger.begin(); iter != m_lstCurPlayersInTrigger.end(); ++iter )
			{
				g_pChatMsgs->AddMessage( FormatTempString(IDS_EXIT_WAITING_NAME,pInfoMgr->GetPlayerName( (*iter)->m_cs.nClientID))  );	
			}
		}
	}
}