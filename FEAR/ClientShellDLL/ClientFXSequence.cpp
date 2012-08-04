// ----------------------------------------------------------------------- //
//
// MODULE  : ClientFXSequence.cpp
//
// PURPOSE : Creates ClientFX, one after the other, in sequence...
//
// CREATED : 4/20/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "stdafx.h"
	#include "ClientFXSequence.h"
	#include "ClientDB.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::Save( )
//
//	PURPOSE:	Save the state of the sequence...
//
// ----------------------------------------------------------------------- //

void CClientFXSequence::Save( ILTMessage_Write &rMsg )
{
	rMsg.WriteDatabaseRecord( g_pLTDatabase, m_hFXSequenceRecord );
	rMsg.WriteObject( m_hParentObject );
	rMsg.Writeuint32( GetState( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::Load( )
//
//	PURPOSE:	Load the state of the sequence...
//
// ----------------------------------------------------------------------- //

void CClientFXSequence::Load( ILTMessage_Read &rMsg )
{
	m_hFXSequenceRecord = rMsg.ReadDatabaseRecord( g_pLTDatabase, ClientDB::Instance( ).GetClientFXSequenceCategory( ));
	m_hParentObject = rMsg.ReadObject( );

	SetState( rMsg.Readuint32( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::Begin_OnEnter( )
//
//	PURPOSE:	Begin the sequence by creating the beginning ClientFX...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::Begin_OnEnter( MacroStateMachine::EventParams &rEventParams )
{
	if( m_ClientFXLink.IsValid( ))
		g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_ClientFXLink );

	if( m_hFXSequenceRecord )
	{
		// Immediately play and show the Enter FX...
		char const* pszEnterFX = ClientDB::Instance( ).GetString( m_hFXSequenceRecord, CDB_FXSEQ_EnterFX );
		if( !LTStrEmpty( pszEnterFX ))
		{
			CLIENTFX_CREATESTRUCT fxInit( pszEnterFX, 0, m_hParentObject );
			g_pGameClientShell->GetRealTimeClientFXMgr().CreateClientFX( &m_ClientFXLink, fxInit, true );
			if( m_ClientFXLink.IsValid( ))
			{
				m_ClientFXLink.GetInstance()->Show( );
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::Begin_OnExit( )
//
//	PURPOSE:	Kill the ClientFX when leaving the beginning state of the sequence...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::Begin_OnExit( MacroStateMachine::EventParams &rEventParams )
{
	// Kill the ClientFX...
	if( m_ClientFXLink.IsValid( ))
		g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_ClientFXLink );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::Begin_OnUpdate( )
//
//	PURPOSE:	When the ClientFX finishes move on the the looping state...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::Begin_OnUpdate( MacroStateMachine::EventParams &rEventParams )
{
	bool bContinueUpdating = false;
	if( m_ClientFXLink.IsValid( ) )
	{
		float fDuration = m_ClientFXLink.GetInstance( )->m_fDuration;
		float fElapsed = m_ClientFXLink.GetInstance( )->m_tmElapsed;

		uint32 dwNum = 0;
		uint32 dwDenom = 1;
		SimulationTimer::Instance( ).GetTimerTimeScale( dwNum, dwDenom );
		float fTimeScale = (float)dwNum / (float)dwDenom;

		// Continue updating if the FX is not currently done or will not end on this frame...
		if( (fDuration - fElapsed) < (g_pLTClient->GetFrameTime( ) / fTimeScale) )
		{
			bContinueUpdating = false;
		}
		else if( !m_ClientFXLink.GetInstance( )->IsDone( ))
		{
			bContinueUpdating = true;
		}
	}

	if( !bContinueUpdating )
	{
		// The beginning FX has finished, enter the loop state... 
		SetState( eState_Loop );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::Loop_OnEnter( )
//
//	PURPOSE:	Create the looping ClientFX.  Leaving this state is controlled
//				by the client of the system...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::Loop_OnEnter( MacroStateMachine::EventParams &rEventParams )
{
	// Make sure any previously playing FX is killed...
	if( m_ClientFXLink.IsValid( ))
		g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_ClientFXLink );

	if( m_hFXSequenceRecord )
	{
		// Play and show the looping FX...
		char const* pszFX = ClientDB::Instance( ).GetString( m_hFXSequenceRecord, CDB_FXSEQ_LoopFX );
		if( !LTStrEmpty( pszFX ))
		{
			CLIENTFX_CREATESTRUCT fxInit( pszFX, FXFLAG_LOOP, m_hParentObject );
			g_pGameClientShell->GetRealTimeClientFXMgr().CreateClientFX( &m_ClientFXLink, fxInit, true );
			if( m_ClientFXLink.IsValid( ))
			{
				m_ClientFXLink.GetInstance()->Show( );
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::Loop_OnExit( )
//
//	PURPOSE:	Kill the ClientFX when leaving the looping state of the sequence...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::Loop_OnExit( MacroStateMachine::EventParams &rEventParams )
{
	// Kill the ClientFX...
	if( m_ClientFXLink.IsValid( ))
		g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_ClientFXLink );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::End_OnEnter( )
//
//	PURPOSE:	End the sequence by creating the finishing ClientFX...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::End_OnEnter( MacroStateMachine::EventParams &rEventParams )
{
	// Make sure any previously playing FX is killed...
	if( m_ClientFXLink.IsValid( ))
		g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_ClientFXLink );

	if( m_hFXSequenceRecord )
	{
		// Create and show the ending FX...
		char const* pszFX = ClientDB::Instance( ).GetString( m_hFXSequenceRecord, CDB_FXSEQ_ExitFX );
		if( !LTStrEmpty( pszFX ))
		{
			CLIENTFX_CREATESTRUCT fxInit( pszFX, 0, m_hParentObject );
			g_pGameClientShell->GetRealTimeClientFXMgr().CreateClientFX( &m_ClientFXLink, fxInit, true );
			if( m_ClientFXLink.IsValid( ))
			{
				m_ClientFXLink.GetInstance()->Show( );
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequence::End_OnExit( )
//
//	PURPOSE:	Kill the ClientFX when leaving the ending state of the sequence...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::End_OnExit( MacroStateMachine::EventParams &rEventParams )
{
	// Kill the ClientFX...
	if( m_ClientFXLink.IsValid( ))
		g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_ClientFXLink );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientFXSequenceStateEnd::Update( )
//
//	PURPOSE:	Kill the ClientFX when leaving the ending state of the sequence...
//
// ----------------------------------------------------------------------- //

bool CClientFXSequence::End_OnUpdate( MacroStateMachine::EventParams &rEventParams )
{
	bool bContinueUpdating = false;
	if( m_ClientFXLink.IsValid( ) )
	{
		float fDuration = m_ClientFXLink.GetInstance( )->m_fDuration;
		float fElapsed = m_ClientFXLink.GetInstance( )->m_tmElapsed;

		uint32 dwNum = 0;
		uint32 dwDenom = 1;
		SimulationTimer::Instance( ).GetTimerTimeScale( dwNum, dwDenom );
		float fTimeScale = (float)dwNum / (float)dwDenom;

		// Continue updating if the FX is not currently done or will not end on this frame...
		if( (fDuration - fElapsed) < (g_pLTClient->GetFrameTime( ) / fTimeScale) )
		{
			bContinueUpdating = false;
		}
		else if( !m_ClientFXLink.GetInstance( )->IsDone( ))
		{
			bContinueUpdating = true;
		}
	}

	if( !bContinueUpdating )
	{
		// The entire sequence has finished, set a null state...
		SetState( eInvalid_State );
	}

	return true;
}

// EOF
