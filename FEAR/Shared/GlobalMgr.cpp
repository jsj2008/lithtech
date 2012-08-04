// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalMgr.cpp
//
// PURPOSE : Implementations of global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GlobalMgr.h"
#include "WeaponDB.h"
#include "MissionDB.h"
#include "NavMarkerTypeDB.h"
#include "AnimationContext.h"
#include "ShatterTypeDB.h"
#include "FXDB.h"
#include "SoundDB.h"
#include "SoundFilterDB.h"
#include "TriggerTypeDB.h"
#include "SurfaceDB.h"
#include "AttachmentDB.h"
#include "DamageTypes.h"
#include "ModelsDB.h"
#include "PropsDB.h"
#include "SoundMixerDB.h"
#include "SoundOcclusionDB.h"
#include "SlowMoDB.h"
#include "DialogueDB.h"
#include "AnimationTreePackedMgr.h"
#include "SonicsDB.h"
#include "StringEditMgr.h"
#include "GameModesDB.h"
#include "BroadcastDB.h"
#include "ObjectiveDB.h"
#include "ActivateDB.h"
#include "CollisionsDB.h"
#include "DamageFxDB.h"
#include "iperformancemonitor.h"
#include "CTFDB.h"
#include "TeamClientFXDB.h"
#include "ControlPointDB.h"


// Parent game performance system.  Most game related performance monitors should
// derive from this parent.
#ifdef _CLIENTBUILD
static CTimedSystem g_tsGameClient("GameClient", NULL);
#endif // _CLIENTBUILD
#ifdef _SERVERBUILD
static CTimedSystem g_tsGameServer("GameServer", NULL);
#endif // _SERVERBUILD

CGlobalMgr *CGlobalMgr::s_pSingleton = NULL;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CGlobalMgr::CGlobalMgr()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CGlobalMgr::CGlobalMgr()
:	m_nRefCount				( 0 )
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CGlobalMgr::~CGlobalMgr()
//              
//	PURPOSE:	Call Term BEFORE deleting allocated aggregates, as doing this
//				the other way causes engine crashes on exit.
//              
//----------------------------------------------------------------------------
/*virtual*/ CGlobalMgr::~CGlobalMgr()
{
	Internal_Term( );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Init()
//
//	PURPOSE:	Initialize, called externally
//
// ----------------------------------------------------------------------- //
bool CGlobalMgr::Init(const CErrorHandler &cError)
{
	if (GetSingleton())
	{
		++GetSingleton()->m_nRefCount;
		return true;
	}

	SetSingleton(debug_new(CGlobalMgr));

	++GetSingleton()->m_nRefCount;

	return GetSingleton()->Internal_Init(cError);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Internal_Init()
//
//	PURPOSE:	Initialize, called internally
//
// ----------------------------------------------------------------------- //

bool CGlobalMgr::Internal_Init(const CErrorHandler &cError)
{
	// Initialise the global internal database interface...

	if( !LoadDatabaseInterface() )
	{
		cError.ShutdownWithError( "LTDatabaseMgr", "GameDatabase.dll" );
		return false;
	}

	// initialise the global string keeper interface...
	if( !LoadStringEditInterface() )
	{
		cError.ShutdownWithError( "LTIStringEditMgr", SKDB_DLL_NAME );
		return false;
	}

	if( !OpenStringEditDatabase() )
	{
		cError.ShutdownWithError( "LTDBStringEditMgr", SKDB_DLL_NAME );
		return false;
	}

	if (!SoundFilterDB::Instance().Init())
	{
		cError.ShutdownWithError("SoundFilterDB", DB_Default_File);
		return false;
	}

	if (!CWeaponDB::Instance().Init())
	{
		cError.ShutdownWithError("WeaponDB", DB_Default_File);
		return false;
	}

	if (!CMissionDB::Instance().Init())
	{
		cError.ShutdownWithError("MissionDB", DB_Default_File);
		return false;
	}

	if (!CFXDB::Instance().Init())
	{
			cError.ShutdownWithError( "FXDB", DB_Default_File );
		return false;
	}

	if (!CSoundDB::Instance().Init())
	{
			cError.ShutdownWithError( "SoundDB", DB_Default_File );
		return false;
	}

	if (!CSurfaceDB::Instance().Init())
	{
		cError.ShutdownWithError("SurfaceDB", DB_Default_File);
		return false;
	}

	if (!CDamageTypeDB::Instance().Init())
	{
		cError.ShutdownWithError("DamageTypeDB", DB_Default_File);
		return false;
	}

	if (!CNavMarkerTypeDB::Instance().Init())
	{
		cError.ShutdownWithError("NavMarkerTypeDB", DB_Default_File);
		return false;
	}

	if (!CNavMarkerLayoutDB::Instance().Init())
	{
		cError.ShutdownWithError("NavMarkerLayoutDB", DB_Default_File);
		return false;
	}

	if (!ModelsDB::Instance().Init())
	{
		cError.ShutdownWithError("ModelsDB", DB_Default_File);
		return false;
	}

	if (!PropsDB::Instance().Init())
	{
		cError.ShutdownWithError("PropsDB", DB_Default_File);
		return false;
	}

	if(!CShatterTypeDB::Instance().Init())
	{
		cError.ShutdownWithError("CShatterTypeDB", const_cast<char*>(DB_Default_File));
		return false;
	}

	if (!SoundFilterDB::Instance().Init())
	{
		cError.ShutdownWithError("SoundFilterDB", DB_Default_File);
		return false;
	}


	if (!TriggerTypeDB::Instance().Init() )
	{
		cError.ShutdownWithError("TriggerTypeDB", DB_Default_File);
		return false;			
	}

	if (!AttachmentDB::Instance().Init())
	{
		cError.ShutdownWithError("AttachmentDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( Activate ).Init())
	{
		cError.ShutdownWithError("ActivateDB", DB_Default_File);
		return false;			
	}

	if (!CSoundMixerDB::Instance().Init())
	{
		cError.ShutdownWithError("CSoundMixerDB", DB_Default_File);
		return false;			
	}

	if (!CSoundOcclusionDB::Instance().Init())
	{
		cError.ShutdownWithError("CSoundOcclusionDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( SlowMo ).Init() )
	{
		cError.ShutdownWithError("SlowMoDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( DamageFxDB ).Init() )
	{
		cError.ShutdownWithError("DamageFxDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( Dialogue ).Init() )
	{
		cError.ShutdownWithError("DialogueDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( Collisions ).Init() )
	{
		cError.ShutdownWithError("Collisions", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( CollisionProperty ).Init() )
	{
		cError.ShutdownWithError("CollisionProperty", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( GameModes ).Init() )
	{
		cError.ShutdownWithError("GameModes", DB_Default_File);
		return false;			
	}

	if (!CAnimationTreePackedMgr::Instance().InitAnimationTreePackedMgr())
	{
		cError.ShutdownWithError("AnimationTreePackedMgr", DB_Default_File);
		return false;
	}

	if( !SonicsDB::Instance().Init() )
	{
		cError.ShutdownWithError("Sonics", DB_Default_File);
		return false;
	}

	if( !DATABASE_CATEGORY( RefillStations ).Init( ))
	{
		cError.ShutdownWithError( "RefillStations", DB_Default_File );
		return false;
	}

	if (!DATABASE_CATEGORY( Broadcast ).Init() )
	{
		cError.ShutdownWithError("BroadcastDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( BroadcastSet ).Init() )
	{
		cError.ShutdownWithError("BroadcastDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( BroadcastGlobal ).Init() )
	{
		cError.ShutdownWithError("BroadcastDB", DB_Default_File);
		return false;			
	}

	if (!DATABASE_CATEGORY( Objective ).Init() )
	{
		cError.ShutdownWithError("ObjectiveDB", DB_Default_File);
		return false;			
	}

	DATABASE_CATEGORY( CTFFlagBase ).Init( );
	DATABASE_CATEGORY( CTFRules ).Init( );
	DATABASE_CATEGORY( TeamClientFX ).Init( );
	DATABASE_CATEGORY( CPRules ).Init( );
	DATABASE_CATEGORY( CPTypes ).Init( );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Term()
//
//	PURPOSE:	Term, called externally
//
// ----------------------------------------------------------------------- //

void CGlobalMgr::Term()
{
	if (!GetSingleton())
		return;

	if (GetSingleton()->m_nRefCount > 1)
	{
		--GetSingleton()->m_nRefCount;
	}
	else
	{
		delete GetSingleton();
		SetSingleton(NULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Term()
//
//	PURPOSE:	Term, called internally
//
// ----------------------------------------------------------------------- //

void CGlobalMgr::Internal_Term()
{
	SoundFilterDB::Instance().Term();
	CSoundMixerDB::Instance().Term();
	CSoundOcclusionDB::Instance().Term();

	if( g_pWeaponDB )
	{
		g_pWeaponDB->Term( );
	}

	if( g_pModelsDB )
	{
		g_pModelsDB->Term();
	}

	if( g_pPropsDB )
	{
		g_pPropsDB->Term();
	}

	CShatterTypeDB::Instance().Term();

	TriggerTypeDB::Instance().Term();
	AttachmentDB::Instance().Term();
	ModelsDB::Instance().Term();

	if( g_pAnimationTreePackedMgr )
	{
		g_pAnimationTreePackedMgr->TermAnimationTreePackedMgr( );
	}

	CloseStringEditDatabase();
	FreeStringEditInterface();

	FreeDatabaseInterface();
}
