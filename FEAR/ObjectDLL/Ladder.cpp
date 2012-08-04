// ----------------------------------------------------------------------- //
//
// MODULE  : Ladder.cpp
//
// PURPOSE : Implementation of Ladded object
//
// CREATED : 06/21/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "Ladder.h"
#include "SurfaceFunctions.h"
#include "SurfaceFlagsOverrideHelpers.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool UnsupportedMsg
//
//  PURPOSE:	Message handler for unsupported messages
//
// ----------------------------------------------------------------------- //

static bool UnsupportedMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->CPrint( "WARNING! - this MSG is not supported by the LADDER object:" );
	}

	return false;
}

extern bool ValidateMsgBool( ILTPreInterface *pInterface, ConParse &cpMsgParams );


LINKFROM_MODULE( Ladder );

BEGIN_CLASS( Ladder )
	ADD_STRINGPROP_FLAG(SURFACE_FLAGS_OVERRIDE, SURFACE_FLAGS_UNKNOWN_STR, PF_STATICLIST, "This dropdown menu allows you to choose a specific surface flag for the Ladder.  NOTE:  If the Unknown surface flag is used, the Ladder surface flag will be used.")
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH( Ladder, GameBase, CF_WORLDMODEL, LadderPlugin, DefaultPrefetch<Ladder>, "Binding brushes to this object creates a space that will allow the the player to climb up and down" )

CMDMGR_BEGIN_REGISTER_CLASS( Ladder )
	ADD_MESSAGE( VISIBLE,		2,	UnsupportedMsg,	NULL,	"VISIBLE", "Unsupported by Ladder objects.", "" )
	ADD_MESSAGE( SOLID,			2,	ValidateMsgBool,	MSG_HANDLER( GameBase, HandleSolidMsg ),	"SOLID <bool>", "Toggles whether the ladder can be climbed.", "msg <ObjectName> (SOLID 1)<BR>msg <ObjectName> (SOLID 0)" )
	ADD_MESSAGE( HIDDEN,		2,	ValidateMsgBool,	MSG_HANDLER( GameBase, HandleHiddenMsg ),	"HIDDEN <bool>", "Toggles whether the ladder can be climbed.", "msg <ObjectName> (HIDDEN 1)<BR>msg <ObjectName> (HIDDEN 0)" )
	ADD_MESSAGE( CASTSHADOW,	2,	UnsupportedMsg,	NULL,	"CASTSHADOW", "Unsupported by Ladder objects.", "" )
	ADD_MESSAGE( SETPOS,		4,	UnsupportedMsg,	NULL,	"SETPOS", "Unsupported by Ladder objects.", ""  )
	ADD_MESSAGE( MOVETOPOS,		4,	UnsupportedMsg,	NULL,	"MOVETOPOS", "Unsupported by Ladder objects.", "" )
	ADD_MESSAGE( SETROTATION,	4,	UnsupportedMsg,	NULL,	"SETROTATION", "Unsupported by Ladder objects.", "" )
CMDMGR_END_REGISTER_CLASS( Ladder, GameBase )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Ladder::Ladder
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

Ladder::Ladder( )
:	GameBase				( OT_WORLDMODEL ),
	m_eSurfaceOverrideType  ( ST_UNKNOWN )
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Ladder::~Ladder
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

Ladder::~Ladder( )
{

}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Ladder::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 Ladder::EngineMessageFn(  uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
	case MID_PRECREATE:
		{
			uint32 nRes = GameBase::EngineMessageFn(messageID, pData, fData);
			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;
			ReadProp(&pOCS->m_cProperties);
			PostReadProp(pOCS);
			return nRes;
		}
		break;
	case MID_OBJECTCREATED:
		{
			uint32 nRes = GameBase::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate( );
			}

			return nRes;
		}
		break;

	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ladder::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Ladder::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return;

	const char *pszSurfaceOverride = pProps->GetString( SURFACE_FLAGS_OVERRIDE_STR, "" );
	if( pszSurfaceOverride && pszSurfaceOverride[0] )
	{
		HSURFACE hSurface = g_pSurfaceDB->GetSurface( pszSurfaceOverride );
		if( hSurface )
		{
			m_eSurfaceOverrideType = g_pSurfaceDB->GetSurfaceType(hSurface);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ladder::PostReadProp
//
//	PURPOSE:	Update the ObjectCreateStruct when creating the object
//
// ----------------------------------------------------------------------- //

void Ladder::PostReadProp(ObjectCreateStruct *pStruct)
{
	pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_RAYHIT;
	pStruct->SetFileName(pStruct->m_Name);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ladder::InitialUpdate
//
//	PURPOSE:	Setup the object.
//
// ----------------------------------------------------------------------- //

void Ladder::InitialUpdate( )
{

	uint32 dwUsrFlags =  SurfaceToUserFlag(m_eSurfaceOverrideType);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUsrFlags, 0xFF000000);

	g_pLTServer->SetObjectShadowLOD( m_hObject, eEngineLOD_Never );

	// Set our special effect message.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_LADDER_ID );
	cMsg.Writeuint8(m_eSurfaceOverrideType);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ladder::GetPrefetchResourceList
//
//	PURPOSE:	determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void Ladder::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// Get the weapon and ammo database records...
	char szSurfaceFlagsOverrideName[256] = {'\0'};
	pInterface->GetPropString(pszObjectName, "SurfaceFlagsOverride", szSurfaceFlagsOverrideName, LTARRAYSIZE(szSurfaceFlagsOverrideName), "");
	if (!LTStrEmpty(szSurfaceFlagsOverrideName))
	{
		HSURFACE hSurface = g_pSurfaceDB->GetSurface(szSurfaceFlagsOverrideName);
		GetRecordResources(Resources, hSurface, true);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderPlugin::PreHook_EditStringList
//
//	PURPOSE:	build string lists for world edit dropdowns
//
// ----------------------------------------------------------------------- //

LadderPlugin::LadderPlugin()
{
}

LadderPlugin::~LadderPlugin()
{
}


LTRESULT LadderPlugin::PreHook_EditStringList(const char* szRezPath,
											   const char* szPropName,
											   char** aszStrings,
											   uint32* pcStrings,
											   const uint32 cMaxStrings,
											   const uint32 cMaxStringLength)
{
	if( LTStrIEquals( SURFACE_FLAGS_OVERRIDE_STR, szPropName ))
	{

		if (CSurfaceDBPlugin::Instance().PopulateStringList(aszStrings, pcStrings,
			cMaxStrings, cMaxStringLength))
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
