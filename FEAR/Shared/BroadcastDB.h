// ----------------------------------------------------------------------- //
//
// MODULE  : BroadcastDB.h
//
// PURPOSE : Database interface for player broadcast messages
//
// CREATED : 12/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BROADCASTDB_H__
#define __BROADCASTDB_H__

enum eNavMarkerPlacement
{
	kNavMarkerLocation,
	kNavMarkerAttached,
	kNavMarkerProjected
};

BEGIN_DATABASE_CATEGORY( Broadcast, "Sound/Broadcast/Broadcasts" )
	DEFINE_GETRECORDATTRIB( Label, char const* );
	DEFINE_GETRECORDATTRIB( Line, char const* );
	DEFINE_GETRECORDATTRIB( Priority, int32 );
	DEFINE_GETRECORDATTRIB( NavMarker, HRECORD );
	DEFINE_GETRECORDATTRIB( NavMarkerPlacement, char const* );

	uint32 GetRandomLineID(HRECORD hRec) const;
	eNavMarkerPlacement GetPlacement(HRECORD hRec) const;

END_DATABASE_CATEGORY( );

BEGIN_DATABASE_CATEGORY( BroadcastSet, "Sound/Broadcast/BroadcastSet" )
	DEFINE_GETRECORDATTRIB( Label, char const* );
	DEFINE_GETRECORDATTRIB( Broadcast, HRECORD );
	DEFINE_GETRECORDATTRIB( BroadcastSubSet, HRECORD );
END_DATABASE_CATEGORY( );

BEGIN_DATABASE_CATEGORY( BroadcastGlobal, "Sound/Broadcast/Global" )
	DEFINE_GETRECORDATTRIB( Icon, HRECORD );
	DEFINE_GETRECORDATTRIB( DamageTimeOut, float );
	DEFINE_GETRECORDATTRIB( SoundOuterRadius, float );
	DEFINE_GETRECORDATTRIB( SoundInnerRadius, float );
	DEFINE_GETRECORDATTRIB( MultiKillTimer, float );

	DEFINE_GETSTRUCTATTRIB( MultiKills, Threshold, uint32 );
	DEFINE_GETSTRUCTATTRIB( MultiKills, Broadcast, HRECORD );
	DEFINE_GETSTRUCTATTRIB( MultiKills, MessageID, const char * );

	DEFINE_GETSTRUCTATTRIB( SequentialKills, Threshold, uint32 );
	DEFINE_GETSTRUCTATTRIB( SequentialKills, Broadcast, HRECORD );
	DEFINE_GETSTRUCTATTRIB( SequentialKills, MessageID, const char * );

	HRECORD GetGlobalRecord( ) const { return g_pLTDatabase->GetRecord( GetCategory( ), "Global" ); }
END_DATABASE_CATEGORY( );



#endif  // __BROADCASTDB_H__
