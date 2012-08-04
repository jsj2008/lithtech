// ----------------------------------------------------------------------- //
//
// MODULE  : AISpawner.cpp
//
// PURPOSE : AISpawner class - implementation
//
// CREATED : 01/16/2004
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISpawner.h"
#include "AIConfig.h"
#include "AIAssert.h"
#include "AIUtils.h"

LINKFROM_MODULE( AISpawner );

BEGIN_CLASS(AISpawner)
	ADD_STRINGPROP_FLAG( SpawnedAIName, "", PF_NOTIFYCHANGE, "The optional name that will be assigned to the spawned AI.") \
	ADD_AICONFIG_AGGREGATE( PF_GROUP(6) )

	// Mimic the AttributeOverrides group in AI.cpp to keep settings in 
	// consistent places.  In the future, consider moving this into AIConfig.
	// For now, this is being left under AttributeOverrides as we can't rename
	// the property without losing values, and having a new property starting 
	// with OV_ starting somewhere different than the rest would be confusing.

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP(4), "TODO:PROPDESC")
		ADD_BOOLPROP_FLAG(OV_Senses,			true, PF_GROUP(4), "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AISpawner, Spawner, 0, CAISpawnerPlugin, "This object allows the creation of CAI objects.  A spawn can be performed using a template object specified in the Target property.")

CMDMGR_BEGIN_REGISTER_CLASS( AISpawner )
CMDMGR_END_REGISTER_CLASS( AISpawner, Spawner )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISpawner::AISpawner
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

AISpawner::AISpawner()
{
	AddAggregate( &m_AIConfig );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISpawner::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

bool AISpawner::ReadProp(const GenericPropList *pProps)
{
	m_strSpawnedAIName = pProps->GetString( "SpawnedAIName", "" );

	return super::ReadProp( pProps );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISpawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AISpawner::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	super::Save( pMsg, dwSaveFlags );

	if (!pMsg) return;

	SAVE_STDSTRING( m_strSpawnedAIName );
	m_AIConfig.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISpawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AISpawner::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	super::Load( pMsg, dwLoadFlags );

	if (!pMsg) return;

	LOAD_STDSTRING( m_strSpawnedAIName );
	m_AIConfig.Load( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISpawner::Spawn
//
//	PURPOSE:	Spawn an object...
//
// ----------------------------------------------------------------------- //

BaseClass* AISpawner::Spawn( const char *pszSpawnString, const char *pszObjName, const LTVector &vPos, const LTRotation &rRot )
{
	// Use the name assigned in the command, if one was specified.

	const char* pszAssignedName = "";
	if( pszObjName[0] ) 
	{
		pszAssignedName = pszObjName;
	}

	// Use the name assigned in the WorldEdit, if one was specified.

	else if( !m_strSpawnedAIName.empty() )
	{
		pszAssignedName = m_strSpawnedAIName.c_str();
	}

	// Spawn the object.

	BaseClass* pBaseClass = super::Spawn( pszSpawnString, pszAssignedName, vPos, rRot );
	if( pBaseClass )
	{
		// Configure the AI.

		if( IsAI( pBaseClass->m_hObject ) )
		{
			CAI* pAI = (CAI*)pBaseClass;
			m_AIConfig.ConfigureAI( pAI );
		}
		else {
			AIASSERT( 0, m_hObject, "Trying to spawn an object that is not a CAI!" );
		}
	}

	return pBaseClass;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAISpawnerPlugin::PreHook_EditStringList
//
//  PURPOSE:	Handle the AIConfig plugin.
//
// ----------------------------------------------------------------------- //

LTRESULT CAISpawnerPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// Let the AIConfig plugin have a go at it...

	if( m_AIConfigPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	// Default behavior.

	return super::PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpawnerPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CAISpawnerPlugin::PreHook_PropChanged( const char *szObjName,
												const char *szPropName, 
												const int  nPropType, 
												const GenericProp &gpPropValue,
												ILTPreInterface *pInterface,
												const char *szModifiers )
{
	// Add any named AI to the dynamic objects...
	if( LTStrIEquals( "SpawnedAIName", szPropName ))
	{
		const char *pszAIName = gpPropValue.GetString( );
		if( pszAIName && pszAIName[0] )
		{
			CCommandMgrPlugin::DYNAMIC_OBJECT obj;
			obj.m_sName = pszAIName;
			obj.m_sClassName = "CAI";

			if( !obj.m_sClassName.empty() && !obj.m_sName.empty() )
			{
				CCommandMgrPlugin::AddDynamicObject( obj );
			}
		}

		return LT_OK;
	}
	
	if( m_AIConfigPlugin.PreHook_PropChanged(szObjName,
											 szPropName, 
											 nPropType, 
											 gpPropValue,
											 pInterface,
											 szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	// Default behavior...
	return super::PreHook_PropChanged( szObjName, szPropName, nPropType, gpPropValue, pInterface, szModifiers );
}
