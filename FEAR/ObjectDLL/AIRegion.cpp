// ----------------------------------------------------------------------- //
//
// MODULE  : AIRegion.cpp
//
// PURPOSE : AI Region class implementation.
//
// CREATED : 09/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIRegion.h"
#include "AIDB.h"
#include "AINavMesh.h"
#include "AIAssert.h"
#include "AIUtils.h"
#include "AISounds.h"
#include "DebugLineSystem.h"
#include "Group.h"


// WorldEdit

LINKFROM_MODULE( AIRegion );

BEGIN_CLASS( AIRegion )
	ADD_REALPROP_FLAG(AIRegionID,	(float)kAIRegion_Invalid,		CF_HIDDEN,			"Internal ID for the AIRegion")

	ADD_STRINGPROP_FLAG(Location,	"None", 						PF_STATICLIST,		"Location associated with this AIRegion.")

	PROP_DEFINEGROUP(ViewNodes, PF_GROUP(1), "ViewNodes associated with this AIRegion") \

		ADD_STRINGPROP_FLAG(ViewNode1, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode2, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode3, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode4, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode5, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode6, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode7, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode8, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode9, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode10, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode11, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode12, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode13, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode14, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode15, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")
		ADD_STRINGPROP_FLAG(ViewNode16, "", PF_GROUP(1)|PF_OBJECTLINK, "ViewNode associated with this AIRegion")

		ADD_COMMANDPROP_FLAG(EnterCommand,	"",		0|PF_NOTIFYCHANGE, "Command run when AI enters an empty AIRegion")
		ADD_COMMANDPROP_FLAG(ExitCommand,	"",		0|PF_NOTIFYCHANGE, "Command run when AI exits an AIRegion, leaving it empty")

		ADD_AI_CHAR_TYPE_RESTRICTIONS_AGGREGATE( PF_GROUP(2) )

END_CLASS_FLAGS_PLUGIN( AIRegion, GameBase, 0, AIRegionPlugin, "AIRegions are used to group portions of an AINavMesh." )

CMDMGR_BEGIN_REGISTER_CLASS( AIRegion )

	ADD_MESSAGE( LIT,		1,	NULL,	MSG_HANDLER( AIRegion, HandleLitMsg ),		"LIT", "Light the NavMesh brushes contained by the AIRegion.", "msg AIRegion0 lit" )
	ADD_MESSAGE( UNLIT,		1,	NULL,	MSG_HANDLER( AIRegion, HandleUnlitMsg ),	"UNLIT", "Unlight the NavMesh", "msg AIRegion0 unlit" )

CMDMGR_END_REGISTER_CLASS_HANDLER( AIRegion, GameBase, 0, MSG_HANDLER( AIRegion, HandleAllMsgs ) )


#define VIEW_NODE_LABEL	"ViewNode"
#define VIEW_NODE_LABEL_BUFFER	64

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AIRegion::AIRegion()
{
	m_eAIRegionID = kAIRegion_Invalid;

	m_eLocationAISoundType = kAIS_InvalidType;

	m_cNMPolys = 0;
	m_pNMPolyList = NULL;

	m_nOccupancy = 0;

	AddAggregate( &m_CharTypeRestrictions );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::SetupAIRegion
//              
//	PURPOSE:	Setup AIRegion.
//              
//----------------------------------------------------------------------------

void AIRegion::SetupAIRegion( const LTVector& vCenter, float fRadius, uint32 cNMPolys, ENUM_NMPolyID* pNMPolyList )
{
	m_vCenter = vCenter;
	m_flRadius = fRadius;

	m_cNMPolys = cNMPolys;
	m_pNMPolyList = pNMPolyList;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::EngineMessageFn
//              
//	PURPOSE:	Handle engine messages.
//              
//----------------------------------------------------------------------------

uint32 AIRegion::EngineMessageFn(uint32 messageID, void *pv, float fData)
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pv, fData);

			if ( (int)fData == PRECREATE_WORLDFILE || (int)fData == PRECREATE_STRINGPROP )
			{
				ObjectCreateStruct* pocs = (ObjectCreateStruct*)pv;
				ReadProp(&pocs->m_cProperties);

				// Ensure the object will never be sent to the client.
				pocs->m_Flags = FLAG_NOTINWORLDTREE;
			}

			return dwRet;
		}
		break;

	case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pv);
		}
		break;

	case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pv);
		}
		break;

	case MID_ALLOBJECTSCREATED:
		{
			AddViewNodesToAIRegion();
		}
		break;

	case MID_INITIALUPDATE:
		{
			// Add the AIRegion to the NavMesh.

			if( fData != INITIALUPDATE_SAVEGAME )
			{
				if( g_pAINavMesh )
				{
					g_pAINavMesh->AddAIRegion( this );
				}
			}

			SetNextUpdate( UPDATE_NEVER );
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pv, fData);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::Save
//              
//	PURPOSE:	Save
//              
//----------------------------------------------------------------------------

void AIRegion::Save(ILTMessage_Write *pMsg)
{
	SAVE_STDSTRING(m_strName);

	SAVE_INT(m_eAIRegionID);

	SAVE_DWORD(m_eLocationAISoundType);

	SAVE_INT( m_lstViewNodes.size() );

	HOBJECT hViewNode;
	ObjRefVector::iterator itViewNode;
	for( itViewNode = m_lstViewNodes.begin(); itViewNode != m_lstViewNodes.end(); ++itViewNode )
	{
		hViewNode = *itViewNode;
		SAVE_HOBJECT( hViewNode );
	}

	SAVE_STDSTRING(m_strViewNodes);	
	SAVE_VECTOR(m_vCenter);
	SAVE_FLOAT(m_flRadius);

	SAVE_INT(m_nOccupancy);

	SAVE_STDSTRING(m_strEnterCommand);
	SAVE_STDSTRING(m_strExitCommand);

	m_CharTypeRestrictions.Save( pMsg );

	// Don't save:
	// m_cNMPolys
	// m_pNMPolyList;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::Load
//              
//	PURPOSE:	Load
//              
//----------------------------------------------------------------------------

void AIRegion::Load(ILTMessage_Read *pMsg)
{
	LOAD_STDSTRING(m_strName);

	LOAD_INT_CAST(m_eAIRegionID, ENUM_AIRegionID);

	LOAD_DWORD_CAST(m_eLocationAISoundType, EnumAISoundType);

	uint32 cViewNodes;
	LOAD_INT( cViewNodes );
	m_lstViewNodes.reserve( cViewNodes );

	HOBJECT hViewNode;
	for( uint32 iViewNode=0; iViewNode < cViewNodes; ++iViewNode )
	{
		LOAD_HOBJECT( hViewNode );
		m_lstViewNodes.push_back( hViewNode );
	}

	LOAD_STDSTRING(m_strViewNodes);	
	LOAD_VECTOR(m_vCenter);
	LOAD_FLOAT(m_flRadius);

	LOAD_INT(m_nOccupancy);

	LOAD_STDSTRING(m_strEnterCommand);
	LOAD_STDSTRING(m_strExitCommand);

	m_CharTypeRestrictions.Load( pMsg );

	// Nullify, and allow the NavMesh Generation to handle setting these.
	m_cNMPolys = 0;
	m_pNMPolyList = NULL;

	if( g_pAINavMesh )
	{
		g_pAINavMesh->AddAIRegion( this );
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AIRegion::ReadProp(const GenericPropList *pProps)
{
	const char* pszPropString = pProps->GetString( "Name", "" );
	if( pszPropString[0] )
	{
		m_strName = pszPropString;
	}

	// Read the AIRegionID assigned to this AIRegion.

	float f = pProps->GetReal( "AIRegionID", (float)m_eAIRegionID );
	m_eAIRegionID = ( ENUM_AIRegionID )( int )f;

	// Read the location label.

	pszPropString = pProps->GetString( "Location", "" );
	if( pszPropString[0] && !LTStrIEquals( pszPropString, "None" ) )
	{
		std::string strLocation = "Location";
		strLocation += pszPropString;
		m_eLocationAISoundType = (EnumAISoundType)g_pAIDB->String2EnumIndex( strLocation.c_str(), kAIS_Count, (uint32) kAIS_InvalidType, s_aszAISoundTypes );
	}

	// Read view nodes.
	// Concatenate node names into one string with comma separators.

	char szBuffer[VIEW_NODE_LABEL_BUFFER];
	for( uint32 iViewNode = 1 ; iViewNode <= MAX_VIEW_NODES ; ++iViewNode )
	{
		LTSNPrintF( szBuffer, ARRAY_LEN( szBuffer ), "%s%d", VIEW_NODE_LABEL, iViewNode );

		pszPropString = pProps->GetString( szBuffer, "" );
		if( pszPropString[0] )
		{
			if( m_strViewNodes.empty() )
			{
				m_strViewNodes = pszPropString;
			}
			else {
				m_strViewNodes += ",";
				m_strViewNodes += pszPropString;
			}
		}
	}

	// Read enter and exit commands.

	pszPropString = pProps->GetCommand( "EnterCommand", "" );
	m_strEnterCommand = pszPropString ? pszPropString : "";

	pszPropString = pProps->GetCommand( "ExitCommand", "" );
	m_strExitCommand = pszPropString ? pszPropString : "";

	// Read the character type restrictions.

	m_CharTypeRestrictions.ReadProp( pProps );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::AddViewNodesToAIRegion
//              
//	PURPOSE:	Add view nodes to the AIRegion.
//              
//----------------------------------------------------------------------------

void AIRegion::AddViewNodesToAIRegion()
{
	// Get handles to View Nodes if any exist.
	// View nodes are stored in one comma-separated string.

	if( !m_strViewNodes.empty() )
	{
		const char* pszObject;

		// Iterate over comma delineated node names.

		int iStart = 0;
		int iEnd = m_strViewNodes.find( ",", 0 );
		while( iEnd != -1 )
		{
			m_strViewNodes[iEnd] = '\0';
			pszObject = m_strViewNodes.c_str() + iStart;
			if( pszObject[0] )
			{
				AddNamedObject( pszObject );
			}

			iStart = iEnd + 1;
			iEnd = m_strViewNodes.find( ",", iStart );
		}

		// Read last node name.

		pszObject = m_strViewNodes.c_str() + iStart;
		if( pszObject[0] )
		{
			AddNamedObject( pszObject );
		}

		// Clear the string.

		m_strViewNodes.clear();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::AddNamedObject
//              
//	PURPOSE:	Add a node or group of nodes to the AIRegion.
//              
//----------------------------------------------------------------------------

void AIRegion::AddNamedObject( const char* pszObject )
{
	HCLASS hGroupClass = g_pLTServer->GetClass( "Group" );
	HCLASS hAINodeClass = g_pLTServer->GetClass( "AINode" );

	// Bail if object not found.

	ILTBaseClass *pObject = NULL;
	if( LT_OK != FindNamedObject( pszObject, pObject, false ) )
	{
		return;
	}

	// Get object's class.

	HCLASS hClass = pObject ? g_pLTServer->GetObjectClass( pObject->m_hObject ) : NULL;

	// Object is a group.

	if( g_pLTServer->IsKindOf( hClass, hGroupClass ) )
	{
		Group* pGroup = (Group*)pObject;
		if( pGroup )
		{
			StringArray* pObjectNames = pGroup->GetObjectNames();
			if( pObjectNames )
			{
				StringArray::iterator itName;
				for( itName = pObjectNames->begin(); itName != pObjectNames->end(); ++itName )
				{
					HOBJECT hObject;
					if( LT_OK == FindNamedObject( itName->c_str(), hObject ) )
					{
						hClass = g_pLTServer->GetObjectClass( hObject );
						if( g_pLTServer->IsKindOf( hClass, hAINodeClass ) )
						{
							m_lstViewNodes.push_back( hObject );
						}
					}
				}
			}
		}
	}

	// Object is a node.

	else if( g_pLTServer->IsKindOf( hClass, hAINodeClass ) )
	{
		m_lstViewNodes.push_back( pObject->m_hObject );
	}

	// Unaccepted type.

	else {
		AIASSERT( 0, NULL, "AIRegion::AddNamedObject: Object is not a node or group." );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::ContainsNMPoly()
//              
//	PURPOSE:	Return true if AIRegion contains specified NavMesh poly.
//              
//----------------------------------------------------------------------------

bool AIRegion::ContainsNMPoly( ENUM_NMPolyID ePoly )
{
	for( int iPoly=0; iPoly < m_cNMPolys; ++iPoly )
	{
		if( m_pNMPolyList[iPoly] == ePoly )
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::GetViewNode()
//              
//	PURPOSE:	Return handle to View Node at specified index.
//              
//----------------------------------------------------------------------------

HOBJECT AIRegion::GetViewNode( uint32 iNode )
{
	if( ( iNode < 0 ) || ( iNode >= m_lstViewNodes.size() ) )
	{
		return NULL;
	}

	return m_lstViewNodes[iNode];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRegion::HandleAllMsgs
//
//	PURPOSE:	Handle all commands.
//
// ----------------------------------------------------------------------- //

void AIRegion::HandleAllMsgs( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	g_vtAIConsoleVar.Init( g_pLTServer, "AIShowMessages", NULL, 0.0f );
	if( g_vtAIConsoleVar.GetFloat() <= 0.0f )
	{
		return;
	}

	switch( crParsedMsg.GetArgCount() )
	{
	case 1: ObjectCPrint( m_hObject, "Received Msg: %s", crParsedMsg.GetArg( 0 ).c_str() );
		break;
	case 2: ObjectCPrint( m_hObject, "Received Msg: %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str() );
		break;
	case 3: ObjectCPrint( m_hObject, "Received Msg: %s %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str() );
		break;
	case 4: ObjectCPrint( m_hObject, "Received Msg: %s %s %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str(), crParsedMsg.GetArg( 3 ).c_str() );
		break;
	default: ObjectCPrint( m_hObject, "Received Msg: %s %s %s %s ...", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str(), crParsedMsg.GetArg( 3 ).c_str() );
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AIRegion::HandleLitMsg
//
//  PURPOSE:	Handle a LIT message...
//
// ----------------------------------------------------------------------- //

void AIRegion::HandleLitMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	LightNMPolys( true );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AIRegion::HandleUnlitMsg
//
//  PURPOSE:	Handle a UNLIT message...
//
// ----------------------------------------------------------------------- //

void AIRegion::HandleUnlitMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	LightNMPolys( false );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::LightNMPolys()
//              
//	PURPOSE:	Set flags on NMPolys for lit or unlit.
//              
//----------------------------------------------------------------------------

void AIRegion::LightNMPolys( bool bLit )
{
	for( int iPoly=0; iPoly < m_cNMPolys; ++iPoly )
	{
		if( bLit )
		{
			g_pAINavMesh->SetNMPolyFlags( m_pNMPolyList[iPoly], kNMPolyFlag_Lit );
		}
		else {
			g_pAINavMesh->ClearNMPolyFlags( m_pNMPolyList[iPoly], kNMPolyFlag_Lit );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::EnterAIRegion()
//              
//	PURPOSE:	Add an AI to the AIRegion.
//              
//----------------------------------------------------------------------------

void AIRegion::EnterAIRegion( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Keep track of how many AI are in the AIRegion.

	++m_nOccupancy;
	AITRACE( AIShowPaths, ( pAI->m_hObject, "AIRegion '%s' occupancy: %d", GetName(), m_nOccupancy ) );

	// Run command when an AI enters an empty AIRegion.

	if( ( m_nOccupancy == 1 ) &&
		( !m_strEnterCommand.empty() ) )
	{
		g_pCmdMgr->QueueCommand( m_strEnterCommand.c_str(), pAI, pAI );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::ExitAIRegion()
//              
//	PURPOSE:	Remove an AI from the AIRegion.
//              
//----------------------------------------------------------------------------

void AIRegion::ExitAIRegion( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Keep track of how many AI are in the AIRegion.

	--m_nOccupancy;
	AITRACE( AIShowPaths, ( pAI->m_hObject, "AIRegion '%s' occupancy: %d", GetName(), m_nOccupancy ) );

	// Run command when an AI leave the AIRegion empty.

	if( ( m_nOccupancy == 0 ) &&
		( !m_strExitCommand.empty() ) )
	{
		g_pCmdMgr->QueueCommand( m_strExitCommand.c_str(), pAI, pAI );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::DrawSelf()
//              
//	PURPOSE:	Draw an AIRegion.
//              
//----------------------------------------------------------------------------

void AIRegion::DrawSelf()
{
	DebugLineSystem& system = LineSystem::GetSystem( this, "ShowAIRegions" );
	system.Clear();

	// Draw NavMesh poly edges.

	bool bDrawName = true;
	CAINavMeshPoly* pPoly;
	for( int iPoly=0; iPoly < m_cNMPolys; ++iPoly )
	{
		pPoly = g_pAINavMesh->GetNMPoly( m_pNMPolyList[iPoly] );
		if( pPoly )
		{
			pPoly->DrawSelfInAIRegion( this, bDrawName );
			bDrawName = false;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIRegion::HideSelf()
//              
//	PURPOSE:	Hide a NavMesh poly.
//              
//----------------------------------------------------------------------------

void AIRegion::HideSelf()
{
	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowAIRegions");
	system.SetDebugString("");
	system.Clear();

	CAINavMeshPoly* pPoly;
	for( int iPoly=0; iPoly < m_cNMPolys; ++iPoly )
	{
		pPoly = g_pAINavMesh->GetNMPoly( m_pNMPolyList[iPoly] );
		if( pPoly )
		{
			pPoly->HideSelfInAIRegion();
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

LTRESULT AIRegionPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if( LTStrIEquals( "Location", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "None" );

		strcpy( aszStrings[(*pcStrings)++], "Boxes" );
		strcpy( aszStrings[(*pcStrings)++], "Catwalk" );
		strcpy( aszStrings[(*pcStrings)++], "Ceiling" );
		strcpy( aszStrings[(*pcStrings)++], "Chair" );
		strcpy( aszStrings[(*pcStrings)++], "Column" );
		strcpy( aszStrings[(*pcStrings)++], "Couch" );
		strcpy( aszStrings[(*pcStrings)++], "Crate" );
		strcpy( aszStrings[(*pcStrings)++], "Cubicle" );
		strcpy( aszStrings[(*pcStrings)++], "Desk" );
		strcpy( aszStrings[(*pcStrings)++], "Door" );
		strcpy( aszStrings[(*pcStrings)++], "Kiosk" );
		strcpy( aszStrings[(*pcStrings)++], "Machine" );
		strcpy( aszStrings[(*pcStrings)++], "Pipe" );
		strcpy( aszStrings[(*pcStrings)++], "Planter" );
		strcpy( aszStrings[(*pcStrings)++], "Shelf" );
		strcpy( aszStrings[(*pcStrings)++], "Table" );
		strcpy( aszStrings[(*pcStrings)++], "VendingMachine" );
		strcpy( aszStrings[(*pcStrings)++], "Vent" );
		strcpy( aszStrings[(*pcStrings)++], "Wall" );

		return LT_OK;
	}

	// Let the AICharTypeRestrictions plugin have a go at it...

	if( m_AICharTypeRestrictionsPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

