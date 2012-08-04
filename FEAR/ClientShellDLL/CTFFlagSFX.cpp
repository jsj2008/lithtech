// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlagSFX.cpp
//
// PURPOSE : Client side representation on CTFFlag
//
// CREATED : 05/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "CTFFlagSFX.h"
#include "CTFFlagBaseSFX.h"
#include "GameModeMgr.h"
#include "StateMachine.h"
#include "InterfaceMgr.h"
#include "ClientInfoMgr.h"
#include "HUDMessageQueue.h"
#include "HUDTransmission.h"
#include "HUDCTFFlag.h"
#include "HUDCTFBase.h"
#include "CMoveMgr.h"
#include "TeamClientFXSFX.h"

// Statemachine to handle ctfflagsfx states.
class FlagSFXStateMachine : public MacroStateMachine
{
public:

	FlagSFXStateMachine( )
	{
		m_pFlagSFX = NULL;
		m_delegatePlayerChangedTeamsEvent.Attach( this, g_pInterfaceMgr->GetClientInfoMgr( ), g_pInterfaceMgr->GetClientInfoMgr( )->PlayerChangedTeamsEvent );
	}

	~FlagSFXStateMachine( )
	{
		Term( );
	}

	// Initialize to a flagbase.
	bool Init( CTFFlagSFX& flag )
	{
		m_pFlagSFX = &flag;
		return true;
	}

	void Term( )
	{
		m_delegatePlayerChangedTeamsEvent.Detach();
		CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();
		pClientFXMgr->ShutdownClientFX( &m_ClientFxLink );
		m_ClientFxLink.ClearLink();
	}

protected:

	// Statemachine event handlers.
	bool InBase_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool InBase_OnPlayerChanged( MacroStateMachine::EventParams& eventParams );
	bool InBase_OnExit( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnPlayerChanged( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnExit( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnPlayerChanged( MacroStateMachine::EventParams& eventParams );

	enum FlagEvent
	{
		eFlagEvent_PlayerChanged = EVENT_User,
	};

	// State table.
	MSM_BeginTable( FlagSFXStateMachine )
		MSM_BeginState( kCTFFlagState_InBase )
			MSM_OnEnter( InBase_OnEnter )
			MSM_OnEvent( eFlagEvent_PlayerChanged, InBase_OnPlayerChanged )
			MSM_OnExit( InBase_OnExit )
		MSM_EndState( )
		MSM_BeginState( kCTFFlagState_Carried )
			MSM_OnEnter( Carried_OnEnter )
			MSM_OnEvent( eFlagEvent_PlayerChanged, Carried_OnPlayerChanged )
			MSM_OnExit( Carried_OnExit )
		MSM_EndState( )
		MSM_BeginState( kCTFFlagState_Loose )
			MSM_OnEnter( Loose_OnEnter )
			MSM_OnEvent( eFlagEvent_PlayerChanged, Loose_OnPlayerChanged )
		MSM_EndState( )
	MSM_EndTable( )

	// Declare delegate to listen for player kill events.
	static void OnPlayerChangedTeamsEvent( FlagSFXStateMachine* pFlagSFXStateMachine, CClientInfoMgr* pClientInfoMgr, EventCaster::NotifyParams& notifyParams )
	{
		g_pHUDMgr->QueueUpdate(kHUDScores);
		pFlagSFXStateMachine->DoUserEvent( eFlagEvent_PlayerChanged, EventParams( ));
	}
	Delegate< FlagSFXStateMachine, CClientInfoMgr, OnPlayerChangedTeamsEvent > m_delegatePlayerChangedTeamsEvent;

	// Creates a team clientfx.
	void CreateTeamClientFX( HRECORD hTeamClientFxRec );

private:

	// The flag that owns us.
	CTFFlagSFX* m_pFlagSFX;

	CClientFXLink m_ClientFxLink;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::InBase_OnEnter
//
//	PURPOSE:	Handle a enter InBase state.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::InBase_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	g_pHUDMgr->QueueUpdate(kHUDScores);

	HRECORD hFlagBaseRec = m_pFlagSFX->m_pCTFFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, FlagTeamClientFxInBase );
	CreateTeamClientFX( hTeamClientFxRec );

	// Tell base it has the flag.
	m_pFlagSFX->m_pCTFFlagBaseSFX->SetHasFlag( true );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::InBase_OnPlayerChanged
//
//	PURPOSE:	Handle a enter InBase state.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::InBase_OnPlayerChanged( MacroStateMachine::EventParams& eventParams )
{
	HRECORD hFlagBaseRec = m_pFlagSFX->m_pCTFFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, FlagTeamClientFxInBase );
	CreateTeamClientFX( hTeamClientFxRec );
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::InBase_OnExit
//
//	PURPOSE:	Handle a Exit InBase state.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::InBase_OnExit( MacroStateMachine::EventParams& eventParams )
{
	g_pHUDMgr->QueueUpdate(kHUDScores);

	// Tell base it lost the flag.
	m_pFlagSFX->m_pCTFFlagBaseSFX->SetHasFlag( false );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::Carried_OnEnter
//
//	PURPOSE:	Handle a enter carried state.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::Carried_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Don't show flag in first person.
	g_pCommonLT->SetObjectFlags( m_pFlagSFX->GetServerObj( ), OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3 );

	HRECORD hFlagBaseRec = m_pFlagSFX->m_pCTFFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, FlagTeamClientFxCarried );
	CreateTeamClientFX( hTeamClientFxRec );

	// Check if we're the flag carrier.
	if( g_pMoveMgr->GetServerObject() == m_pFlagSFX->m_hFlagCarrier )
	{
		// Show the flag on our HUD.
		g_pHUDCTFFlag->SetHasFlag( true );

		// Hide the clientfx, since we're holding it.
		m_ClientFxLink.GetInstance()->Hide( );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::Carried_OnPlayerChanged
//
//	PURPOSE:	Handle when player changes teams.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::Carried_OnPlayerChanged( MacroStateMachine::EventParams& eventParams )
{
	HRECORD hFlagBaseRec = m_pFlagSFX->m_pCTFFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, FlagTeamClientFxCarried );
	CreateTeamClientFX( hTeamClientFxRec );
	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::Carried_OnExit
//
//	PURPOSE:	Handle a exit carried state.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::Carried_OnExit( MacroStateMachine::EventParams& eventParams )
{
	// Handle issue where playermgr clears visibility flag on this object due to the USRFLG_ATTACH_HIDE1SHOW3 flag.
	g_pCommonLT->SetObjectFlags( m_pFlagSFX->GetServerObj(), OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	g_pCommonLT->SetObjectFlags( m_pFlagSFX->GetServerObj( ), OFT_User, 0, USRFLG_ATTACH_HIDE1SHOW3 );
	g_pLTClient->SetObjectShadowLOD( m_pFlagSFX->GetServerObj(), eEngineLOD_Low);

	// Make sure we don't think we have the flag.
	if( g_pMoveMgr->GetServerObject() == m_pFlagSFX->m_hFlagCarrier )
	{
		g_pHUDCTFFlag->SetHasFlag( false );
	}

	m_pFlagSFX->m_hFlagCarrier = NULL;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::Loose_OnEnter
//
//	PURPOSE:	Handle a enter loose state.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::Loose_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	HRECORD hFlagBaseRec = m_pFlagSFX->m_pCTFFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, FlagTeamClientFxLoose );
	CreateTeamClientFX( hTeamClientFxRec );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagSFXStateMachine::Loose_OnPlayerChanged
//
//	PURPOSE:	Handle a enter Loose state.
//
// ----------------------------------------------------------------------- //
bool FlagSFXStateMachine::Loose_OnPlayerChanged( MacroStateMachine::EventParams& eventParams )
{
	HRECORD hFlagBaseRec = m_pFlagSFX->m_pCTFFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, FlagTeamClientFxLoose );
	CreateTeamClientFX( hTeamClientFxRec );
	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::CreateClientFX
//
//  PURPOSE:	Creates the clientfx for the team.
//
// ----------------------------------------------------------------------- //
void FlagSFXStateMachine::CreateTeamClientFX( HRECORD hTeamClientFxRec )
{
	uint8 nTeamId = m_pFlagSFX->m_pCTFFlagBaseSFX->GetCS().m_nTeamId;
	TeamClientFXSFX::CreateTeamClientFX( m_pFlagSFX->GetServerObj( ), hTeamClientFxRec, nTeamId, m_ClientFxLink );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::CTFFlagSFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTFFlagSFX::CTFFlagSFX( )
{
	m_pFlagSFXStateMachine = NULL;
	m_pCTFFlagBaseSFX = NULL;
	m_hFlagCarrier = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::~CTFFlagSFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTFFlagSFX::~CTFFlagSFX( )
{
	if( m_pFlagSFXStateMachine )
	{
		delete m_pFlagSFXStateMachine;
		m_pFlagSFXStateMachine = NULL;
	}

	if( m_csCTFFlag.m_hFlagBase && m_pCTFFlagBaseSFX )
	{
		m_pCTFFlagBaseSFX->SetFlag( NULL, NULL );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::Init
//
//  PURPOSE:	Initialize the client side turret class...
//
// ----------------------------------------------------------------------- //

bool CTFFlagSFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg ))
		return false;

	m_csCTFFlag.Read( pMsg );

	// Make sure flagbase knows about us.  Because the order of the objects arriving on the client is
	// not easily managed, each interdependent object will set itself on the other.
	m_pCTFFlagBaseSFX = reinterpret_cast< CTFFlagBaseSFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_CTFFLAGBASE_ID, m_csCTFFlag.m_hFlagBase ));

	// Register with the pickup detector so we have the same priority.
	if( !m_iObjectDetectorLink.IsRegistered( ))
	{
		g_pPlayerMgr->GetPickupObjectDetector( ).RegisterObject( m_iObjectDetectorLink, m_hServerObject, this );
	}

	// Create our statemachine object.
	LT_MEM_TRACK_ALLOC(m_pFlagSFXStateMachine = new FlagSFXStateMachine, LT_MEM_TYPE_GAMECODE);
	m_pFlagSFXStateMachine->Init( *this );
	if( m_pCTFFlagBaseSFX )
	{
		m_pFlagSFXStateMachine->SetState( m_csCTFFlag.m_eCTFFlagState );
		m_pCTFFlagBaseSFX->SetFlag( GetServerObj( ), this );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::SetFlagBase
//
//  PURPOSE:	Sets the flag base object.
//
// ----------------------------------------------------------------------- //
void CTFFlagSFX::SetFlagBase( HOBJECT hObject, CTFFlagBaseSFX* pCTFFlagBaseSFX ) 
{ 
	m_csCTFFlag.m_hFlagBase = hObject; 
	m_pCTFFlagBaseSFX = pCTFFlagBaseSFX;
	m_pFlagSFXStateMachine->SetState( m_csCTFFlag.m_eCTFFlagState );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::Update
//
//  PURPOSE:	Update the weapon associated with the turret...
//
// ----------------------------------------------------------------------- //

bool CTFFlagSFX::Update( )
{
	if( !CSpecialFX::Update( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DoPlayerFlagEvent
//
//  PURPOSE:	Common function used to post messages due to a player event received by OnServerMessage.
//
// ----------------------------------------------------------------------- //

inline bool DoPlayerFlagEvent( ILTMessage_Read* pMsg, char const* pszGameMsgFormat, eChatMsgType eType, char const* pszTransmissionStringId )
{
	// Read in the player doing the event.
	uint8 nClientId = pMsg->Readuint8( );
	CLIENT_INFO *pClient = g_pInterfaceMgr->GetClientInfoMgr( )->GetClientByID( nClientId );
	if( !pClient )
		return false;

	// Write out the event to the message log.
	wchar_t wszTmp[128] = L"";
	FormatString( pszGameMsgFormat, wszTmp, LTARRAYSIZE( wszTmp ), pClient->sName.c_str( ));
	g_pGameMsgs->AddMessage( wszTmp, eType );

	// Do the transmission.
	const char* pszFilename = g_pClientSoundMgr->GetSoundFilenameFromId( "Dialogue", pszTransmissionStringId );
	if( !LTStrEmpty( pszFilename ))
	{
		g_pClientSoundMgr->PlaySoundLocal(pszFilename,SOUNDPRIORITY_PLAYER_HIGH);
	}

	g_pTransmission->Show( LoadString( pszTransmissionStringId ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::OnServerMessage
//
//  PURPOSE:	Handle a message recieved from the server side object...
//
// ----------------------------------------------------------------------- //

bool CTFFlagSFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	m_csCTFFlag.m_eCTFFlagState = ( CTFFlagState )pMsg->ReadBits( FNumBitsExclusive<kCTFFlagState_NumStates>::k_nValue );

	// See if there is any event message info.
	if( pMsg->Readbool( ))
	{
		// Check if this flag is for our team.
		bool bLocalFlag = ( g_pInterfaceMgr->GetClientInfoMgr( )->IsLocalTeam( m_pCTFFlagBaseSFX->GetCS().m_nTeamId ));

		// Handle the event.
		CTFFlagFXMsgId eMsgId = ( CTFFlagFXMsgId )pMsg->ReadBits( FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
		switch( eMsgId )
		{
		case kCTFFlagFXMsg_Steal:
			{
				// Read in the flag carrier.
				m_hFlagCarrier = pMsg->ReadObject();

				char const* pszTransmissionId = bLocalFlag ? 
					DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), TheyStoleFlagMessage ) : 
				DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), WeStoleFlagMessage );
				DoPlayerFlagEvent( pMsg, bLocalFlag ? "CTF_PlayerStoleOurFlag" : "CTF_PlayerStoleTheirFlag", 
					bLocalFlag ? kMsgOtherTeam : kMsgTeam, pszTransmissionId );
			}
			break;
		case kCTFFlagFXMsg_PickedUp:
			{
				// Read in the flag carrier.
				m_hFlagCarrier = pMsg->ReadObject();

				char const* pszTransmissionId = bLocalFlag ? 
					DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), TheyPickedUpFlagMessage ) : 
				DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), WePickedUpFlagMessage );
				DoPlayerFlagEvent( pMsg, bLocalFlag ? "CTF_PlayerPickedUpOurFlag" : "CTF_PlayerPickedUpTheirFlag", 
					bLocalFlag ? kMsgOtherTeam : kMsgTeam, pszTransmissionId );
			}
			break;
		case kCTFFlagFXMsg_Return:
			{
				char const* pszTransmissionId = bLocalFlag ? 
					DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), WeReturnedFlagMessage ) : 
				DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), TheyReturnedFlagMessage );
				DoPlayerFlagEvent( pMsg, bLocalFlag ? "CTF_PlayerReturnedOurFlag" : "CTF_PlayerReturnedTheirFlag", 
					bLocalFlag ? kMsgTeam : kMsgOtherTeam, pszTransmissionId );
			}
			break;
		case kCTFFlagFXMsg_AutoReturn:
			{
				char const* pszTransmissionId = bLocalFlag ? 
					DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), AutoReturnedOurFlagMessage ) : 
				DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), AutoReturnedTheirFlagMessage );

				// Do the transmission.
				const char* pszFilename = g_pClientSoundMgr->GetSoundFilenameFromId( "Dialogue", pszTransmissionId );
				if( !LTStrEmpty( pszFilename ))
				{
					g_pClientSoundMgr->PlaySoundLocal(pszFilename,SOUNDPRIORITY_PLAYER_HIGH);
				}

				wchar_t const* pwszTransmission = LoadString( pszTransmissionId );
				g_pGameMsgs->AddMessage( pwszTransmission, bLocalFlag ? kMsgTeam : kMsgOtherTeam );
				g_pTransmission->Show( pwszTransmission );
			}
			break;
		case kCTFFlagFXMsg_Dropped:
			{
				char const* pszTransmissionId = bLocalFlag ? 
					DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), TheyDropppedFlagMessage ) : 
				DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), WeDropppedFlagMessage );
				DoPlayerFlagEvent( pMsg, bLocalFlag ? "CTF_PlayerDroppedOurFlag" : "CTF_PlayerDroppedTheirFlag", 
					bLocalFlag ? kMsgOtherTeam : kMsgTeam, pszTransmissionId );
			}
			break;
		case kCTFFlagFXMsg_Capture:
			{
				char const* pszTransmissionId = bLocalFlag ? 
					DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), TheyCapturedFlagMessage ) : 
				DATABASE_CATEGORY( CTFRules ).GETRECORDATTRIB( GameModeMgr::Instance().GetCTFRulesRecord(), WeCapturedFlagMessage );
				DoPlayerFlagEvent( pMsg, bLocalFlag ? "CTF_PlayerCapturedOurFlag" : "CTF_PlayerCapturedTheirFlag", 
					bLocalFlag ? kMsgOtherTeam : kMsgTeam, pszTransmissionId );
			}
			break;
		case kCTFFlagFXMsg_FlagDefend:
			{
				uint8 nKillerClientId = pMsg->Readuint8( );
				CLIENT_INFO *pKillerClient = g_pInterfaceMgr->GetClientInfoMgr( )->GetClientByID( nKillerClientId );
				if( !pKillerClient )
					return false;

				// Write out the event to the message log.
				wchar_t wszTmp[128] = L"";
				FormatString( bLocalFlag ? "CTF_PlayerDefendedOurFlagBase" : "CTF_PlayerDefendedTheirFlagBase", 
					wszTmp, LTARRAYSIZE( wszTmp ), pKillerClient->sName.c_str( ));
				g_pGameMsgs->AddMessage( wszTmp, bLocalFlag ? kMsgTeam : kMsgOtherTeam );
			}
			break;
		case kCTFFlagFXMsg_FlagCarrierDefend:
			{
				uint8 nKillerClientId = pMsg->Readuint8( );
				CLIENT_INFO *pKillerClient = g_pInterfaceMgr->GetClientInfoMgr( )->GetClientByID( nKillerClientId );
				if( !pKillerClient )
					return false;

				// Write out the event to the message log.
				wchar_t wszTmp[128] = L"";
				FormatString( bLocalFlag ? "CTF_PlayerDefendedTheirFlagCarrier" : "CTF_PlayerDefendedOurFlagCarrier", 
					wszTmp, LTARRAYSIZE( wszTmp ), pKillerClient->sName.c_str( ));
				g_pGameMsgs->AddMessage( wszTmp, bLocalFlag ? kMsgOtherTeam : kMsgTeam );
			}
			break;
		case kCTFFlagFXMsg_FlagCarrierKill:
			{
				uint8 nKillerClientId = pMsg->Readuint8( );
				CLIENT_INFO *pKillerClient = g_pInterfaceMgr->GetClientInfoMgr( )->GetClientByID( nKillerClientId );
				if( !pKillerClient )
					return false;

				// Write out the event to the message log.
				wchar_t wszTmp[128] = L"";
				FormatString( bLocalFlag ? "CTF_PlayerKilledTheirFlagCarrier" : "CTF_PlayerKilledOurFlagCarrier", 
					wszTmp, LTARRAYSIZE( wszTmp ), pKillerClient->sName.c_str( ));
				g_pGameMsgs->AddMessage( wszTmp, bLocalFlag ? kMsgTeam : kMsgOtherTeam );
			}
			break;
		case kCTFFlagFXMsg_CaptureAssist:
			{
				uint8 nClientId = pMsg->Readuint8( );
				CLIENT_INFO *pClient = g_pInterfaceMgr->GetClientInfoMgr( )->GetClientByID( nClientId );
				if( !pClient )
					return false;

				// Write out the event to the message log.
				wchar_t wszTmp[128] = L"";
				FormatString( bLocalFlag ? "CTF_PlayerAssistedOurCapture" : "CTF_PlayerAssistedTheirCapture", 
					wszTmp, LTARRAYSIZE( wszTmp ), pClient->sName.c_str( ));
				g_pGameMsgs->AddMessage( wszTmp, bLocalFlag ? kMsgTeam : kMsgOtherTeam  );
			}
			break;
		}
	}

	if( !m_pFlagSFXStateMachine->SetState( m_csCTFFlag.m_eCTFFlagState ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::CanGrabFlag
//
//  PURPOSE:	Checks if flag can be taken by local player.
//
// ----------------------------------------------------------------------- //
bool CTFFlagSFX::CanGrabFlag( ) const
{
	switch( m_csCTFFlag.m_eCTFFlagState )
	{
	case kCTFFlagState_InBase:
		{
			// Get the base we're associated with.
			CTFFlagBaseSFX* pCTFFlagBaseSFX = reinterpret_cast< CTFFlagBaseSFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_CTFFLAGBASE_ID, m_csCTFFlag.m_hFlagBase ));
			if( !pCTFFlagBaseSFX )
				return false;

			// Check if base allows steals.
			if( !pCTFFlagBaseSFX->GetCS().m_bAllowSteals )
				return false;

			// No teams, always allow grabs.
			if( !GameModeMgr::Instance( ).m_grbUseTeams )
				return true;

			// Only allow steals if on opposite teams.
			CClientInfoMgr *pCI = g_pInterfaceMgr->GetClientInfoMgr();
			CLIENT_INFO* pLocal = pCI->GetLocalClient();
			return ( pLocal->nTeamID != pCTFFlagBaseSFX->GetCS().m_nTeamId );
		}
		break;
	case kCTFFlagState_Carried:
		return false;
		break;
	case kCTFFlagState_Loose:
		return true;
		break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagSFX::IsFlagInBase
//
//  PURPOSE:	Checks if flag is in the base.
//
// ----------------------------------------------------------------------- //
bool CTFFlagSFX::IsFlagInBase( ) const
{
	return ( m_pFlagSFXStateMachine->GetState( ) == kCTFFlagState_InBase );
}