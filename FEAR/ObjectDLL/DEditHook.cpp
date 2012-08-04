// ----------------------------------------------------------------------- //
//
// MODULE  : DEditHook.cpp
//
// PURPOSE : DEditHook contains callbacks to GameServer.dll for certain event...
//
// CREATED : 07/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"

LINKFROM_MODULE( DEditHook );

#if !defined(PLATFORM_LINUX)

#include "DatabaseUtils.h"
#include "NavMarkerTypeDB.h"
#include "FXDB.h"
#include "SoundDB.h"
#include "SurfaceDB.h"
#include "DamageTypes.h"
#include "SoundFilterDB.h"
#include "ShatterTypeDB.h"
#include "TriggerTypeDB.h"
#include "AttachmentDB.h"
#include "CharacterDB.h"
#include "AIDB.h"
#include "ModelsDB.h"
#include "CommandDB.h"
#include "PropsDB.h"
#include "SoundMixerDB.h"
#include "SoundOcclusionDB.h"
#include "SlowMoDB.h"
#include "DialogueDB.h"
#include "CollisionsDB.h"
#include "StringEditMgr.h"
#include "GameModesDB.h"
#include "ScreenEffectDB.h"
#include "ActivateDB.h"
#include "AnimationTreePackedMgr.h"
#include "ObjectiveDB.h"
#include "ServerDB.h"
#include "CTFDB.h"
#include "TeamClientFXDB.h"
#include "MissionDB.h"
#include "ClientFXDB.h"
#include "ControlPointDB.h"

// this reference count is used to make sure that we only init the module once 
static int32 s_nRefCount = 0;

BEGIN_EXTERNC()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEditLoadModule()
//
//	PURPOSE:	This is called when WorldEdit loads GameServer.dll.  This is a good
//				place to handle any initialization of global managers.
//
// ----------------------------------------------------------------------- //

MODULE_EXPORT bool DEditLoadModule( const char *pszProjectPath )
{
	// if it's already initialized then just exit
	if( s_nRefCount++ )
		return true;

	// Load the database library for use in our databases...

	// Since it's from WorldEdit we need to specify the full path.
	// The DLL is located one directory up from the full project path...

	std::string sProjectPath = pszProjectPath;
	std::string sRoot = sProjectPath;
	sProjectPath += "\\";

	std::string::size_type nEndIndex = sRoot.rfind( "\\" );

	// Chop off the resource file directory...

	sRoot.erase( nEndIndex + 1 );
	
	if( !LoadDatabaseInterface( (sRoot + GDB_DLL_NAME).c_str() ))
	{
		return false;
	}

	if( !LoadStringEditInterface( (sRoot + SKDB_DLL_NAME).c_str() ))
	{
		return false;
	}

	if( !OpenStringEditDatabase( (sProjectPath + FILE_STRINGEDIT_DB).c_str()) )
	{
		return false;
	}

	// Initialize the databases that will be used while working from within the editor...
	if(!CSoundDB::Instance( ).Init((sProjectPath + DB_Default_File).c_str()) )
		{
			return false;
		}

	if(!CFXDB::Instance( ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}


	if(!CDamageTypeDB::Instance( ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if(!CWeaponDB::Instance( ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if(!CWeaponDBPlugin::Instance( ).Init( ))
		{
			return false;
		}

	if(!CNavMarkerTypeDB::Instance( ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if(!CNavMarkerLayoutDB::Instance( ).Init((sProjectPath + DB_Default_Localized_File).c_str()) )
	{
		return false;
	}

	if(!CShatterTypeDB::Instance( ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if(!CSurfaceDB::Instance( ).Init((sProjectPath + DB_Default_File).c_str()) )
		{
			// PM - The only way this can fail is if the surface records are invalid
			//      if that is the case then we want to display an error in 
			//      WorldEdit and not just quit out
			//return false;
		}

	if (!SoundFilterDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!TriggerTypeDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!AttachmentDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!CServerDB::Instance().Init((sProjectPath + DB_Default_File).c_str()))
	{
		return false;
	}

	if (!CCharacterDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!CAIDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!ModelsDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!PropsDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!CCommandDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!CMissionDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!DATABASE_CATEGORY( Activate ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!CSoundMixerDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!CSoundOcclusionDB::Instance().Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!DATABASE_CATEGORY( SlowMo ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!DATABASE_CATEGORY( Dialogue ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if (!DATABASE_CATEGORY( CollisionProperty ).Init((sProjectPath + DB_Default_File).c_str()) )
	{
		return false;
	}

	if( !DATABASE_CATEGORY( RefillStations ).Init( (sProjectPath + DB_Default_File).c_str() ))
	{
		return false;
	}

	if( !DATABASE_CATEGORY( GameModes ).Init( (sProjectPath + DB_Default_File).c_str() ))
	{
		return false;
	}

	if( !DATABASE_CATEGORY( Objective ).Init( (sProjectPath + DB_Default_File).c_str() ))
	{
		return false;
	}

	DATABASE_CATEGORY( ScreenEffect ).Init((sProjectPath + DB_Default_File).c_str());

	DATABASE_CATEGORY( CTFFlagBase ).Init( (sProjectPath + DB_Default_File).c_str() );

	DATABASE_CATEGORY( CPTypes ).Init( (sProjectPath + DB_Default_File).c_str() );

	DATABASE_CATEGORY( TeamClientFX ).Init( (sProjectPath + DB_Default_File).c_str() );

	// Load up all the animation trees so we have access to dynamic animation enums, etc.
	if (CAnimationTreePackedMgr::Instance().InitAnimationTreePackedMgr())
	{
		for (uint32 iModel = 0; iModel < g_pModelsDB->GetNumModels(); iModel++)
		{
			HRECORD hModel = g_pModelsDB->GetModel(iModel);
			for (uint32 i = 0; i < g_pModelsDB->GetNumModelAnimationTrees(hModel); i++)
			{
				const char* pszModel = g_pModelsDB->GetModelAnimationTree(hModel, i);
				g_pAnimationTreePackedMgr->GetAnimationTreePacked((sProjectPath + pszModel).c_str());
			}
		}
	}

	// Load and initialize the ClientFX DLL
	if(!CClientFXDB::GetSingleton().LoadFxDll((sProjectPath + "ClientFx.fxd").c_str(), false))
	{
		LTERROR("Failed to load ClientFX.fxd");
	}

	// now load in the client fx files
	if(!CClientFXDB::GetSingleton().LoadToolFxFilesInDir((sProjectPath + "ClientFX").c_str()))
	{
		LTERROR("Failed to load ClientFX files");
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEditUnloadModule()
//
//	PURPOSE:	This is called when WorldEdit unloads GameServer.dll.
//
// ----------------------------------------------------------------------- //

MODULE_EXPORT void DEditUnloadModule()
{
	LTASSERT( s_nRefCount > 0, "DEditUnloadModule call without DEditLoadModule call" );

	// if there is still a reference then do not unload
	if( --s_nRefCount )
		return;

	CloseStringEditDatabase();
	FreeStringEditInterface();

	// Unload the database library...

	FreeDatabaseInterface();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEditUnloadModule()
//
//	PURPOSE:	This is called when WorldEdit requests information on the list of commands used in the game...
//
// ----------------------------------------------------------------------- //

MODULE_EXPORT void DEditGetCommandDefs( ICommandDef *pDefs, uint32 *pNumDefs )
{
	uint32 nNumDefs = GetNumCommandDescriptions();
	
	if( pNumDefs )
		*pNumDefs = nNumDefs;

	if( pDefs && (nNumDefs > 0) )
	{
		CCommandDescription *pCmdDescs = GetCommandDescriptions();

		for( uint32 nCmd = 0; nCmd < nNumDefs; ++nCmd )
		{
			pDefs[nCmd].m_pszName = pCmdDescs[nCmd].m_pszName;
			pDefs[nCmd].m_pszSyntax = pCmdDescs[nCmd].m_pszSyntax;
			pDefs[nCmd].m_pszDescription = pCmdDescs[nCmd].m_pszDescription;
		}
	}
}

MODULE_EXPORT void DEditGetCommandClassDefs( ICommandClassDef *pDefs, uint32 *pNumDefs )
{
	uint32 nNumDefs = GetNumCmdmgrClassDescriptions();

	if( pNumDefs )
		*pNumDefs = nNumDefs;

	if( pDefs && (nNumDefs > 0) )
	{
		for( uint32 nCmd = 0; nCmd < nNumDefs; ++nCmd )
		{
			CCmdMgr_ClassDesc *pCmdDesc  = GetCmdmgrClassDescription( nCmd );
			if( !pCmdDesc )
				continue;

			pDefs[nCmd].m_pszName = pCmdDesc->m_pszName;
			pDefs[nCmd].m_pszParent = pCmdDesc->m_pszParent;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEditUnloadModule()
//
//	PURPOSE:	This is called when WorldEdit requests information on the messages used by a specific class...
//
// ----------------------------------------------------------------------- //

MODULE_EXPORT void DEditGetClassMessageDefs( const char *pszClass, ICommandMessageDef *pDefs, uint32 *pNumDefs )
{
	if( !pszClass || !pszClass[0] )
		return;

	CCmdMgr_ClassDesc *pClassDesc = GetCmdmgrClassDescription( pszClass );
	if( !pClassDesc )
		return;

	uint32 nNumDefs = pClassDesc->m_nNumMsgs;
	if( pNumDefs )
		*pNumDefs = nNumDefs;

	if( pDefs && (nNumDefs > 0) )
	{
		// The first message is bogus so skip it...
		for( uint32 nMsg = 1; nMsg < nNumDefs; ++nMsg )
		{
			pDefs[nMsg].m_pszName = pClassDesc->m_pMsgs[nMsg].m_pszName;
			pDefs[nMsg].m_pszSyntax = pClassDesc->m_pMsgs[nMsg].m_pszSyntax;
			pDefs[nMsg].m_pszDescription = pClassDesc->m_pMsgs[nMsg].m_pszDescription;
			pDefs[nMsg].m_pszExample = pClassDesc->m_pMsgs[nMsg].m_pszExample;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEditGetOcclusionInfo()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

MODULE_EXPORT void DEditGetOcclusionInfo( OcclusionSurfaceEditInfo* aSurfaceList, uint32* pnNumSurfaces,
										 const uint32 nMaxSurfaces, const uint32 cMaxStringLength )
{
	if (aSurfaceList == NULL)
	{
		return;
	}

	CSoundOcclusionDBPlugin::Instance().GetOcclusionSurfaceInfoList(aSurfaceList, 
		pnNumSurfaces, nMaxSurfaces, cMaxStringLength);
}


END_EXTERNC()

#endif // PLATFORM_LINUX

