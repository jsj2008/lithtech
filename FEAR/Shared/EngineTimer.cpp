// ----------------------------------------------------------------------- //
//
// MODULE  : EngineTimer.cpp
//
// PURPOSE : Definition/Implementation of the engine timer classes.
//
// CREATED : 4/19/04
//
// (c) 1996-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "EngineTimer.h"

// Saving of objecttimers only supported on server, since the engine
// only saves the timers on the server.
#ifdef _SERVERBUILD

// Accessor to the global ist of serializable engine timers.
// Need to put this into a accessor function to guarantee order of
// operations with construction of this object and global SerializeableEngineTimers.
// Just using a global static would not guarantee that the list was
// constructed before it was modified.
SerializeableEngineTimer::SerializeableEngineTimerList& SerializeableEngineTimer::GetSerializeableEngineTimerList()
{
	static SerializeableEngineTimerList lstSerializeableEngineTimers;
	return lstSerializeableEngineTimers;
}

// Called when the server does a load of a save game.  It is necessary to run through 
// all persistant timers and clear them out since the engine server creates
// new timer objects internally.  All timers must be save/loaded manually to have them
// persist across save/load.
// This only needs to be done on the server since the engine only recreates
// the timers on the server for a load.
void SerializeableEngineTimer::OnLoad( bool bKeepAliveLoad )
{
	// Get the handles to the old global timers so we can find them
	// in the list of serializeable timers.  Add references, since
	// the global timers themselves are part of the list.  Adding ref
	// will ensure we're never pointing at deleted memory.
	HENGINETIMER hOldSimulationTimerHandle = SimulationTimer::Instance( ).GetHandle();
	g_pLTBase->Timer()->AddTimerRef( hOldSimulationTimerHandle );
	HENGINETIMER hOldRealTimeTimerHandle = RealTimeTimer::Instance( ).GetHandle();
	g_pLTBase->Timer()->AddTimerRef( hOldRealTimeTimerHandle );
	HENGINETIMER hOldGameTimeTimerHandle = GameTimeTimer::Instance( ).GetHandle();
	g_pLTBase->Timer()->AddTimerRef( hOldGameTimeTimerHandle );

	// Get the new global timer handles.
	HENGINETIMER hNewSimulationTimerHandle = g_pLTBase->Timer( )->GetSimulationTimer( );
	HENGINETIMER hNewRealTimeTimerHandle = g_pLTBase->Timer( )->CreateTimer( );
	g_pLTBase->Timer()->AddSystemChildTimer( hNewRealTimeTimerHandle );
	HENGINETIMER hNewGameTimeTimerHandle = g_pLTBase->Timer( )->CreateTimer( );
	g_pLTBase->Timer()->AddSystemChildTimer( hNewGameTimeTimerHandle );

	// Go through all the serializable timers and reset their handles to the new ones.
	SerializeableEngineTimerList::iterator iter = GetSerializeableEngineTimerList( ).begin();
	for( ; iter != GetSerializeableEngineTimerList( ).end( ); iter++ )
	{
		SerializeableEngineTimer* pTimer = *iter;
		if( pTimer->GetHandle() == hOldSimulationTimerHandle )
			pTimer->SetHandle( hNewSimulationTimerHandle );
		else if( pTimer->GetHandle() == hOldRealTimeTimerHandle )
			pTimer->SetHandle( hNewRealTimeTimerHandle );
		else if( pTimer->GetHandle() == hOldGameTimeTimerHandle )
			pTimer->SetHandle( hNewGameTimeTimerHandle );
	}

	g_pLTBase->Timer( )->ReleaseTimer( hOldSimulationTimerHandle );
	g_pLTBase->Timer( )->ReleaseTimer( hOldRealTimeTimerHandle );
	g_pLTBase->Timer( )->ReleaseTimer( hOldGameTimeTimerHandle );
	g_pLTBase->Timer( )->ReleaseTimer( hNewSimulationTimerHandle );
	if( bKeepAliveLoad )
	{
		g_pLTBase->Timer( )->ReleaseTimer( hNewRealTimeTimerHandle );
		g_pLTBase->Timer( )->ReleaseTimer( hNewGameTimeTimerHandle );
	}
}

// Serialize to a message.
void SerializeableEngineTimer::Save(ILTMessage_Write& msg) const
{
	// If the timer is the simulationtimer or interfacetimer, 
	// then we need to hook the timer back up to the new simuationtimer
	// or interfacetimer on load.  Otherwise, we can get the timer
	// handle from the serialization itself.  We can't
	// for the serializaationtimer or interfacetimer because
	// they can exist beyond a level and the timers are reset
	// on a level switch.
	if( GetHandle( ) == SimulationTimer::Instance( ).GetHandle( ))
	{
		msg.Writeuint8(( uint8 )eEngineTimerType_SimulationTimer );
	}
	else if( GetHandle( ) == RealTimeTimer::Instance( ).GetHandle( ))
	{
		msg.Writeuint8(( uint8 )eEngineTimerType_InterfaceTimer );
	}
	else if( GetHandle( ) == GameTimeTimer::Instance( ).GetHandle( ))
	{
		msg.Writeuint8(( uint8 )eEngineTimerType_GameTimer );
	}
	else if( IsValid( ))
	{
		msg.Writeuint8(( uint8 )eEngineTimerType_ObjectTimer );
		msg.WriteTimer( GetHandle( ));
	}
	else
	{
		msg.Writeuint8(( uint8 )eEngineTimerType_Invalid );
	}
}

// Deserialize from a message.
void SerializeableEngineTimer::Load(ILTMessage_Read& msg)
{
	// If the timer is the simulationtimer or interfacetimer type,
	// then hook it up to the existing global timers.  Otherwise
	// read it from the msg.
	EngineTimerType eEngineTimerType = ( EngineTimerType )msg.Readuint8( );
	switch( eEngineTimerType )
	{
		case eEngineTimerType_SimulationTimer:
			SetHandle( SimulationTimer::Instance().GetHandle());
			break;
		case eEngineTimerType_GameTimer:
			SetHandle( GameTimeTimer::Instance().GetHandle());
			break;
		case eEngineTimerType_InterfaceTimer:
			SetHandle( RealTimeTimer::Instance().GetHandle());
			break;
		case eEngineTimerType_ObjectTimer:
			SetHandle( msg.ReadTimer( ));
			break;
		case eEngineTimerType_Invalid:
			break;
		default:
			LTERROR( "Invalid engine timer type." );
			break;
	}
}

#endif // _SERVERBUILD
