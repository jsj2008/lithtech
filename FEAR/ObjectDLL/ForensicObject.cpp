// ----------------------------------------------------------------------- //
//
// MODULE  : ForensicObject.h
//
// PURPOSE : Game object that defines forensic areas, and properties
//
// CREATED : 11/22/04
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ForensicObject.h"
#include "WeaponDB.h"
#include "DialogueDB.h"
#include "ContainerCodes.h"
#include "AIPathMgrNavMesh.h"
#include "AIQuadTree.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"

// ----------------------------------------------------------------------- //

LINKFROM_MODULE( ForensicObject );

// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_ForensicObject 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_ForensicObject CF_HIDDEN

#endif

BEGIN_CLASS( ForensicObject )

	ADD_STRINGPROP_FLAG( FarthestPoint, "", PF_OBJECTLINK, "The name of the object (use a Point object) that specifies the farthest effective pathable distance for instincts to this forensic (i.e. where the forensic tool first starts working).  This should only be set for the primary evidence." )
	ADD_REALPROP_FLAG( CoreRadius, 50.0f, PF_RADIUS, "Radius that determines how close the player must be to use the collection tool." )
	ADD_REALPROP_FLAG( ObjectFOV, 0.0f, PF_CONEFOV, "FOV restriction (cone on object) - make sure the player is positioned correctly to collect the evidence. [in degrees] (zero = ignore)" )
	ADD_REALPROP_FLAG( CameraFOV, 60.0f, 0, "FOV restriction (cone on camera) - make sure the player is looking in the right place to collect the evidence. [in degrees]" )
	ADD_BOOLPROP_FLAG( Locked, false, 0, "Determines if the instinct nav mesh starts out disabled or not." )
	ADD_BOOLPROP_FLAG( Primary, true, 0, "Determines if this object is the primary evidence or a secondary trail marker." )
	ADD_STRINGPROP_FLAG( NavMeshType, "None", PF_STATICLIST, "Character type (AI/Attribute) used to identify the navmesh used by this forensic.")
	ADD_STRINGPROP_FLAG( DetectionTool, "None", PF_STATICLIST, "Detection Tool required to examine evidence in this volume." )
	ADD_STRINGPROP_FLAG( CollectionTool, "None", PF_STATICLIST, "Collection Tool required to extract evidence in this volume." )
	ADD_STRINGPROP_FLAG( SecondaryInfo, "None", PF_STATICLIST, "Information describing the secondary evidence leading up to this primary object.  Note: Only needs to be set on the primary evidence, and will be shared by all secondary evidence associated with this object.")
	ADD_COMMANDPROP_FLAG( ToolSelect, "", PF_NOTIFYCHANGE, "Command sent when the player takes out their tool.  This can occur multiple times." )

	ADD_STRINGPROP_FLAG(Animation, "", PF_HIDDEN, "")
	ADD_STRINGPROP_FLAG(ActivateType, "", PF_HIDDEN, "")
	ADD_REALPROP_FLAG(ActivateDist, 0.0f, PF_HIDDEN, "")
	ADD_BOOLPROP_FLAG(Radial, true, 0, "If true, the player will be aligned to the object, and the distance check will be radial.  Otherwise, the player will positioned at the closest point along the plane of the object and that distance will be used.")

END_CLASS_FLAGS_PLUGIN( ForensicObject, SpecialMove, CF_WORLDMODEL | CF_HIDDEN_ForensicObject, CForensicObjectPlugin, "This object represents a piece of evidence that can be collected." )

// ----------------------------------------------------------------------- //

CMDMGR_BEGIN_REGISTER_CLASS( ForensicObject )

	ADD_MESSAGE( LOCK, 1, NULL, MSG_HANDLER( ForensicObject, HandleMsgLock ), "LOCK", "Used to disable the instinct.", "msg <ObjectName> LOCK" )
	ADD_MESSAGE( UNLOCK, 1, NULL, MSG_HANDLER( ForensicObject, HandleMsgUnlock ), "UNLOCK", "Used to enable the instinct.", "msg <ObjectName> UNLOCK" )

CMDMGR_END_REGISTER_CLASS( ForensicObject, SpecialMove )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::ForensicObject()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ForensicObject::ForensicObject() : SpecialMove()
{
	m_fMaxDistance = 0.0f;
	m_fCoreRadius = 0.0f;
	m_fObjectFOV = 0.0f;
	m_fCameraFOV = 0.0f;
	m_bLocked = false;
	m_bPrimary = false;
	m_dwForensicTypeMask = ALL_CHAR_TYPES;
	m_rDetectionTool = NULL;
	m_rCollectionTool = NULL;
	m_rSecondaryInfo = NULL;
	m_sToolSelectCommand = "";
	m_sFarthestPointName[0] = '\0';

	m_pNMPath = AI_FACTORY_NEW( CAIPathNavMesh );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::~ForensicObject()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ForensicObject::~ForensicObject()
{
	AI_FACTORY_DELETE( m_pNMPath );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::HandleSfxMessage
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void ForensicObject::HandleSfxMessage( HOBJECT hSender, ILTMessage_Read *pMsg, uint8 nSfxId )
{
	SpecialMove::HandleSfxMessage(hSender, pMsg, nSfxId);

	switch( nSfxId )
	{
		case FORENSICFX_REQUEST_FORENSIC_DIST:
		{
			HOBJECT hObj = pMsg->ReadObject();
			OnRequestDist(hObj);
		}
		break;

		case FORENSICFX_FORENSIC_TOOL_SELECTED:
		{
			if( !m_sToolSelectCommand.empty() )
			{
				g_pCmdMgr->QueueCommand( m_sToolSelectCommand.c_str(), m_hObject, NULL );
			}
		}
		break;
	}
}

void ForensicObject::OnReleased()
{
	SpecialMove::OnReleased();

	// automatically lock once the primary evidence has been collected.
	if (m_bPrimary)
	{
		SetLocked(true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::GetForensicTypeMask()
//
//	PURPOSE:	Collects all the forensic character types used (for filtering
//				nav mesh poly lookups).
//
// ----------------------------------------------------------------------- //

uint32& ForensicObject::GetForensicTypeMask()
{
	static uint32 dwForensicTypeMask = 0;
	return dwForensicTypeMask;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::ReadProp()
//
//	PURPOSE:	Read properties from object data
//
// ----------------------------------------------------------------------- //

void ForensicObject::ReadProp( const GenericPropList *pProps )
{
	SpecialMove::ReadProp(pProps);
	if( !pProps ) return;

	LTStrCpy(m_sFarthestPointName, pProps->GetString("FarthestPoint", ""), LTARRAYSIZE(m_sFarthestPointName));

	m_fCoreRadius = LTMAX( 0.0f, pProps->GetReal( "CoreRadius", 0.0f ) );

	m_fObjectFOV = LTCos(DEG2RAD(pProps->GetReal("ObjectFOV", m_fObjectFOV)) * 0.5f);	// half-angle from center
	m_fCameraFOV = LTCos(DEG2RAD(pProps->GetReal("CameraFOV", m_fCameraFOV)) * 0.5f);	// half-angle from center

	m_bLocked = pProps->GetBool( "Locked", false );
	m_bPrimary = pProps->GetBool( "Primary", false );

	const char* pszNavMeshType = pProps->GetString( "NavMeshType", "" );
	if( pszNavMeshType && pszNavMeshType[0] )
	{
		ENUM_AIAttributesID eAttributesID = g_pAIDB->GetAIAttributesRecordID( pszNavMeshType );
		m_dwForensicTypeMask = ( 1 << eAttributesID );
	}

	if (m_bPrimary)
	{
		if (!m_bLocked)
		{
			ForensicObject::GetForensicTypeMask() |= m_dwForensicTypeMask;
		}

		const char* pszDetectionTool = pProps->GetString( "DetectionTool", "" );
		if( !LTStrEmpty(pszDetectionTool) )
		{
			m_rDetectionTool = g_pWeaponDB->GetWeaponRecord(pszDetectionTool);
		}

		const char* pszSecondaryInfo = pProps->GetString( "SecondaryInfo", "" );
		if( !LTStrEmpty(pszSecondaryInfo) )
		{
			m_rSecondaryInfo = DATABASE_CATEGORY( Dialogue ).GetRecordByName( pszSecondaryInfo );
		}

		m_sToolSelectCommand = pProps->GetCommand( "ToolSelect", "" );
	}

	const char* pszCollectionTool = pProps->GetString( "CollectionTool", "" );
	if( !LTStrEmpty(pszCollectionTool) )
	{
		m_rCollectionTool = g_pWeaponDB->GetWeaponRecord(pszCollectionTool);
	}

	// parent overrides...
	m_eAnimation = kAP_ACT_Fire;
	m_ActivateTypeHandler.SetActivateType(g_pWeaponDB->GetActionIcon(m_rDetectionTool));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::PostReadProp
//
//	PURPOSE:	Update the ObjectCreateStruct when creating the object
//
// ----------------------------------------------------------------------- //

void ForensicObject::PostReadProp(ObjectCreateStruct *pStruct)
{
	SpecialMove::PostReadProp(pStruct);
	m_vPos = pStruct->m_Pos;
	m_vDir = pStruct->m_Rotation.Forward().GetUnit();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::OnAllObjectsCreated()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

uint32 ForensicObject::OnAllObjectsCreated()
{
	if (m_bPrimary)
	{
		m_eDestPoly = g_pAIQuadTree->GetContainingNMPoly(m_vPos, m_dwForensicTypeMask, kNMPoly_Invalid);

		//!!ARL: It would be neat to keep the object around and recalculate the max
		// distance when the the player distance is calculated.  This would allow
		// us to have a smoke trail fade over time for instance - by moving the
		// fartest point closer to the source.  Of course, timed puzzles are lame.
		// On the other hand, we'll also run into problems if we decide we want the
		// source to be able to move around - like if you're tracking down a person
		// or a rat with a clue.  But that's got its own set of problems we can
		// worry about separately if we ever decide to go that route.

		HOBJECT hFarthestPoint;
		if (FindNamedObject(m_sFarthestPointName, hFarthestPoint, false) == LT_OK)
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(hFarthestPoint, &vPos);
			ENUM_NMPolyID eFarthestPoly = g_pAIQuadTree->GetContainingNMPoly(vPos, m_dwForensicTypeMask, kNMPoly_Invalid);
			m_fMaxDistance = CalcDistFrom(vPos, eFarthestPoly);

			if (m_fMaxDistance <= m_fCoreRadius)
			{
				LTERROR("FarthestPoint is inside core radius!");
				m_fMaxDistance = (m_fCoreRadius + 150.0f);
			}
		}
		else
		{
			LTASSERT_PARAM1( 0, "Could not find object named \"%s\"", m_sFarthestPointName);
			m_fMaxDistance = 500.0f;
		}
	}

	return SpecialMove::OnAllObjectsCreated();
}

void ForensicObject::WriteSFXMsg(CAutoMessage& cMsg)
{
	SpecialMove::WriteSFXMsg(cMsg);
	cMsg.Writebool( m_bPrimary );
	cMsg.WriteLTVector( m_vPos );
	cMsg.WriteLTVector( m_vDir );
	cMsg.Writefloat( m_fMaxDistance );
	cMsg.Writefloat( m_fCoreRadius );
	cMsg.Writefloat( m_fObjectFOV );
	cMsg.Writefloat( m_fCameraFOV );
	cMsg.Writeuint32( m_dwForensicTypeMask );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_rDetectionTool );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_rCollectionTool );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_rSecondaryInfo );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::OnRequestDist()
//
//	PURPOSE:	The client wants us to give them an update for the given object.
//
// ----------------------------------------------------------------------- //

void ForensicObject::OnRequestDist(HOBJECT hObj)
{
	if (!m_bPrimary)
	{
		LTERROR("Forensic distance request to non-primary object!");
		return;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(hObj, &vPos);

	ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly(vPos, m_dwForensicTypeMask, kNMPoly_Invalid);

	float fDist = CalcDistFrom(vPos, ePoly);
	
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_FORENSICOBJECT_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8(FORENSICFX_SEND_FORENSIC_DIST);
	cMsg.Writefloat(fDist);
	cMsg.WriteObject(hObj);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::GetPathedDistance()
//
//	PURPOSE:	Calculates the distance between two points via the shortest traversable path.
//
// ----------------------------------------------------------------------- //

float ForensicObject::CalcDistFrom(const LTVector& vPos, ENUM_NMPolyID eSourcePoly)
{
	uint32 nPullStringsMaxIters = g_pAIDB->GetAIConstantsRecord()->nStringPullingMaxIterations;
	if (!g_pAIPathMgrNavMesh->FindPath(NULL, m_dwForensicTypeMask, vPos, m_vPos, eSourcePoly, m_eDestPoly, nPullStringsMaxIters, kPath_Default, m_pNMPath))
		return (vPos - m_vPos).Mag();

	float fDist = m_pNMPath->GetPathDistance();
//	m_pNMPath->Clear();
	return fDist;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ForensicObject::Save( ILTMessage_Write* pMsg, uint32 nFlags )
{
	SpecialMove::Save(pMsg, nFlags);

	if( !pMsg ) return;

	SAVE_VECTOR( m_vPos );
	SAVE_VECTOR( m_vDir );
	SAVE_FLOAT( m_fMaxDistance );
	SAVE_FLOAT( m_fCoreRadius );
	SAVE_FLOAT( m_fObjectFOV );
	SAVE_FLOAT( m_fCameraFOV );
	SAVE_BOOL( m_bLocked );
	SAVE_DWORD( m_dwForensicTypeMask );
	SAVE_HRECORD( m_rDetectionTool );
	SAVE_HRECORD( m_rCollectionTool );
	SAVE_HRECORD( m_rSecondaryInfo );
	SAVE_STDSTRING( m_sToolSelectCommand );
	SAVE_CHARSTRING( m_sFarthestPointName );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ForensicObject::Load( ILTMessage_Read* pMsg, uint32 nFlags )
{
	SpecialMove::Load(pMsg, nFlags);

	if( !pMsg ) return;

	LOAD_VECTOR( m_vPos );
	LOAD_VECTOR( m_vDir );
	LOAD_FLOAT( m_fMaxDistance );
	LOAD_FLOAT( m_fCoreRadius );
	LOAD_FLOAT( m_fObjectFOV );
	LOAD_FLOAT( m_fCameraFOV );
	LOAD_BOOL( m_bLocked );
	LOAD_DWORD( m_dwForensicTypeMask );
	LOAD_HRECORD( m_rDetectionTool, g_pWeaponDB->GetWeaponsCategory() );
	LOAD_HRECORD( m_rCollectionTool, g_pWeaponDB->GetWeaponsCategory() );
	LOAD_HRECORD( m_rSecondaryInfo, DATABASE_CATEGORY( Dialogue ).GetCategory() );
	LOAD_STDSTRING( m_sToolSelectCommand );
	LOAD_CHARSTRING( m_sFarthestPointName, LTARRAYSIZE( m_sFarthestPointName ) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::SetLocked
//
//	PURPOSE:	Enable or disable this forensic.
//
// ----------------------------------------------------------------------- //

void ForensicObject::SetLocked( bool bLocked )
{
	if (!m_bPrimary)
	{
		LTERROR("Only primary Forensic objects should be locked/unlocked!");
		return;
	}

	m_bLocked = bLocked;

	if (m_bLocked)
	{
		ForensicObject::GetForensicTypeMask() &= ~m_dwForensicTypeMask;
	}
	else
	{
		ForensicObject::GetForensicTypeMask() |= m_dwForensicTypeMask;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::HandleMsgLock
//
//	PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void ForensicObject::HandleMsgLock( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	SetLocked(true);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ForensicObject::HandleMsgUnlock
//
//	PURPOSE:	Handle an UNLOCK message...
//
// ----------------------------------------------------------------------- //

void ForensicObject::HandleMsgUnlock( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	SetLocked(false);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CForensicObjectPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CForensicObjectPlugin::PreHook_EditStringList(	const char* szRezPath,
														const char* szPropName,
														char** aszStrings,
														uint32* pcStrings,
														const uint32 cMaxStrings,
														const uint32 cMaxStringLength)
{
	// DetectionTool.

	if ( LTStrIEquals( "DetectionTool", szPropName ) )
	{
		if( CategoryPlugin::Instance().PopulateStringList( g_pWeaponDB->GetWeaponsCategory(), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_OK;
		}
	}

	// CollectionTool.

	else if ( LTStrIEquals( "CollectionTool", szPropName ) )
	{
		if( CategoryPlugin::Instance().PopulateStringList( g_pWeaponDB->GetWeaponsCategory(), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_OK;
		}
	}

	// SecondaryInfo.

	else if ( LTStrIEquals( "SecondaryInfo", szPropName ) )
	{
		if( CategoryPlugin::Instance().PopulateStringList( DATABASE_CATEGORY( Dialogue ).GetCategory(), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_OK;
		}
	}

	// NavMeshType

	else if ( LTStrIEquals( "NavMeshType", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "None" );

		AIDB_AttributesRecord* pRecord;
		int cCharTypes = g_pAIDB->GetNumAIAttributesRecords();
		for( int iType=0; iType < cCharTypes; ++iType )
		{
			pRecord = g_pAIDB->GetAIAttributesRecord( iType );
			if( pRecord )
			{
				strcpy( aszStrings[(*pcStrings)++], pRecord->strName.c_str() );
			}
		}

		return LT_OK;
	}

	// Unsupported.

	return LT_UNSUPPORTED;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CForensicObjectPlugin::PreHook_PropChanged
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTRESULT CForensicObjectPlugin::PreHook_PropChanged( const char *szObjName, 
										   const char *szPropName,
										   const int nPropType,
										   const GenericProp &gpPropValue,
										   ILTPreInterface *pInterface,
										   const char *szModifiers )
{
	// Only our commands are marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
		szPropName, 
		nPropType, 
		gpPropValue,
		pInterface,
		szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

