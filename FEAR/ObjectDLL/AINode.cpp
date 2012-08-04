// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "AINode.h"
#include "AIUtils.h"
#include "AI.h"
#include "AIDB.h"
#include "AINodeMgr.h"
#include "AINavMesh.h"
#include "AIQuadTree.h"
#include "DEditColors.h"
#include "AnimationPropStrings.h"
#include "CharacterDB.h"
#include "AIWorkingMemory.h"
#include "AIWorkingMemoryCentral.h"
#include "ParsedMsg.h"
#include "AnimationContext.h"
#include "AIState.h"
#include "AIRegion.h"
#include "AIBlackBoard.h"
#include "AIWorldState.h"
#include <algorithm>

extern VarTrack			g_ShowNodesTrack;


LINKFROM_MODULE( AINode );



// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINode)

	ADD_BOOLPROP_FLAG(Face,				true,				0, "Set this to true if you want the AI who uses this node to face along the forward of the node.")
	ADD_ROTATIONPROP_FLAG(FacingOffset,						PF_RELATIVEORIENTATION, "Offset AI should face from the node's rotation." )
	ADD_STRINGPROP_FLAG(CharacterType,	"None", 			PF_STATICLIST, "This is a dropdown list that allows you to set the character type restriction for the AINode.  Only AI that are of the character type are able to use the node.")
	ENABLED_PROPS()

END_CLASS_FLAGS_PLUGIN(AINode, GameBase, 0, AINodePlugin, "This is the most basic node type.  The only purpose of a basic AINode is as a destination for a Goto command")

static bool ValidateRemoveMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateRemoveMsg()" );
		pInterface->CPrint( "    MSG - REMOVE - Unable to remove AINode.  Use DISABLE instead!" );

		return false;
	}

	return true;
}

CMDMGR_BEGIN_REGISTER_CLASS(AINode)

	ADD_MESSAGE( ENABLE,	1,	NULL,	MSG_HANDLER( AINode, HandleEnableMsg ),		"ENABLE", "Enable an AINode.", "msg AINode01 enable" )
	ADD_MESSAGE( DISABLE,	1,	NULL,	MSG_HANDLER( AINode, HandleDisableMsg ),	"DISABLE", "Disable an AINode.  Nodes may start disabled using the StartDisabled property flag in WorldEdit.", "msg AINode01 disable" )
	ADD_MESSAGE( REMOVE,	1,	ValidateRemoveMsg,	MSG_HANDLER( AINode, HandleRemoveMsg ),		"REMOVE", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS(AINode, GameBase)


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::HandleToObject
//
//  PURPOSE:	Simple static function for converting an HOBJECT of an 
//				AINode object to an AINode* pointer.  Asserts if the cast 
//				is incorrect.
//
// ----------------------------------------------------------------------- //

AINode* AINode::HandleToObject(HOBJECT hNode)
{
	AIASSERT(IsKindOf(hNode, "AINode"), hNode, "AINode::HandleToObject : Object is being cast to invalid type.");
	return (AINode*)g_pLTServer->HandleToObject(hNode);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::GetNodeType
//
//  PURPOSE:	Convert the passed in name to a node type.
//
// ----------------------------------------------------------------------- //



AINode::AINode()
{
	m_eNodeClusterID = kNodeCluster_Invalid;

	m_vPos = LTVector(0,0,0);

	m_bFaceNode = true;
	m_vFaceDir = LTVector(0,0,0);

	m_fNodeReactivationTime = 0.f;
	m_fNodeNextActivationTime = 0.f;
	m_fNodeLastActivationTime = 0.f;
	m_fNodeDepartureTime = 0.f;

	m_fRadius = (float)INT_MAX;

	m_eAIRegion = kAIRegion_Invalid;

	m_eContainingNMPoly = kNMPoly_Invalid;

	m_dwCharTypeMask = ALL_CHAR_TYPES;

	m_hNodeOwner = NULL;

	m_hLockingAI = NULL;

	m_bDynamicOnly = false;

	m_bDebugNodeIsValid = true;
}

AINode::~AINode()
{
}

void AINode::InitNode()
{
	// Find NavMesh poly containing node.

	m_eContainingNMPoly = g_pAIQuadTree->GetContainingNMPoly( m_vPos, m_dwCharTypeMask, kNMPoly_Invalid );
	if ( kNMPoly_Invalid == m_eContainingNMPoly && !AllowOutsideNavMesh() )
	{
		AIASSERT1( 0, m_hObject, "AINode '%s' is outside the NavMesh!", GetNodeName() );
	}

	// Find AIRegion by name, and record it's AIRegionID.

	if( !m_strAIRegion.empty() )
	{
		ILTBaseClass *pObject = NULL;
		if( LT_OK == FindNamedObject( m_strAIRegion.c_str(), pObject ) )
		{
			AIRegion* pAIRegion = (AIRegion*)pObject;
			if( pAIRegion )
			{
				m_eAIRegion = pAIRegion->GetAIRegionID();
			}
		}
	}

	// Create referenced node cluster.

	if( m_eNodeClusterID != kNodeCluster_Invalid )
	{
		g_pAINodeMgr->CreateNodeCluster( m_eNodeClusterID );
	}
}

uint32 AINode::EngineMessageFn(uint32 messageID, void *pv, float fData)
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

		case MID_INITIALUPDATE:
		{
			SetNextUpdate( UPDATE_NEVER );
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pv, fData);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::HandleEnableMsg
//
//  PURPOSE:	Handle a ENABLE message...
//
// ----------------------------------------------------------------------- //

void AINode::HandleEnableMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	EnableNode();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::HandleDisableMsg
//
//  PURPOSE:	Handle a DISABLE message...
//
// ----------------------------------------------------------------------- //

void AINode::HandleDisableMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	DisableNode();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::HandleRemoveMsg
//
//  PURPOSE:	Handle a REMOVE message...
//
// ----------------------------------------------------------------------- //

void AINode::HandleRemoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	AIError( "Attempting to remove AINode \"%s\"! Disabling instead.", GetNodeName() );
	HandleDisableMsg( hSender, crParsedMsg );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::EnableNode
//
//  PURPOSE:	Enable the node
//
// ----------------------------------------------------------------------- //

void AINode::EnableNode()
{
	m_EnabledValidator.SetEnabled( true );

	AITRACE( AIShowNodes, ( m_hObject, "Enabling node" ) );

	//if we're drawing debug stuff, update
	if (g_ShowNodesTrack.GetFloat())
	{
		HideSelf();
		DrawSelf();
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::DisableNode
//
//  PURPOSE:	Disable the node.
//
// ----------------------------------------------------------------------- //

void AINode::DisableNode()
{
	m_EnabledValidator.SetEnabled( false );

	AITRACE( AIShowNodes, ( m_hObject, "Disabling node" ) );

	//if we're drawing debug stuff, update
	if (g_ShowNodesTrack.GetFloat())
	{
		HideSelf();
		DrawSelf();
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::SetNodeOwner
//
//  PURPOSE:	Set handle to object who owns this node.
//
// ----------------------------------------------------------------------- //

void AINode::SetNodeOwner(HOBJECT hOwner)
{
	// Nodes may only be owned by 1 object at a time.
	// The owner may be an AI or another node.
	// (e.g. an AI owns a Guard node.  The Guard node owns a list
	// of SmartObject nodes, only useable by the owner of the guard node).

	HOBJECT hCurNodeOwner = GetNodeOwner();
	
	AIASSERT( ( !hCurNodeOwner ) || ( !hOwner ) || ( hCurNodeOwner == hOwner ), m_hObject, "AINode::SetNodeOwner: Node already has an owner." );
	m_hNodeOwner = hOwner;

	AITRACE( AIShowNodes, ( m_hObject, "Setting owner: %s", GetObjectName( m_hNodeOwner ) ) );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::GetNodeOwner
//
//  PURPOSE:	Return handle to object who owns this node.
//
// ----------------------------------------------------------------------- //

HOBJECT AINode::GetNodeOwner()
{
	if( m_hNodeOwner && IsDeadAI( m_hNodeOwner ) )
	{
		m_hNodeOwner = NULL;
	}

	return m_hNodeOwner; 
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::Lock / Unlock / IsLocked
//
//  PURPOSE:	Lock nodes, reset reactivation time.
//
// ----------------------------------------------------------------------- //

void AINode::LockNode(HOBJECT hAI)
{
	// Node is locked by someone else.

	HOBJECT hLockingAI = GetLockingAI();
	if( hLockingAI && hLockingAI != hAI )
	{
		char szName[64];
		g_pLTServer->GetObjectName( hLockingAI, szName, sizeof(szName) );
		AIASSERT2( 0, hAI, "AINode::LockNode: Node '%s' already locked by AI '%s'", GetNodeName(), szName );
		return;
	}

	// Lock the node.

	AITRACE( AIShowNodes, ( hAI, "Locking node %s\n", GetNodeName() ) );
	m_hLockingAI = hAI; 

	// Lock the node's cluster.

	if( m_eNodeClusterID != kNodeCluster_Invalid )
	{
		CAINodeCluster* pCluster = g_pAINodeMgr->GetNodeCluster( m_eNodeClusterID );
		if( pCluster )
		{
			pCluster->LockCluster( hAI );
		}
	}

	// AI records node that's locked.

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hAI );
	if( pAI )
	{
		pAI->GetAIBlackBoard()->SetBBLockedNode( m_hObject );
	}
}

void AINode::UnlockNode(HOBJECT hAI)
{
	// Node is not locked.

	HOBJECT hLockingAI = GetLockingAI();
	if( !hLockingAI )
	{
		AIASSERT1( 0, hAI, "AINode::Unlock: Node '%s' is not locked.", GetNodeName() );
		return;
	}

	// Node is locked by someone else.

	if( hLockingAI != hAI )
	{
		char szName[64];
		g_pLTServer->GetObjectName( hLockingAI, szName, sizeof(szName) );
		AIASSERT2( 0, hAI, "AINode::Unlock: Node '%s' is locked by AI '%s'.", GetNodeName(), szName );
		return;
	}

	// Unlock the node.

	AITRACE( AIShowNodes, ( hAI, "Unlocking node %s\n", GetNodeName() ) );
	m_hLockingAI = NULL;

	// Unlock the node's cluster.

	if( m_eNodeClusterID != kNodeCluster_Invalid )
	{
		CAINodeCluster* pCluster = g_pAINodeMgr->GetNodeCluster( m_eNodeClusterID );
		if( pCluster )
		{
			pCluster->UnlockCluster( hAI );
		}
	}

	// AI clears record of locked node.

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hAI );
	if( pAI )
	{
		pAI->GetAIBlackBoard()->SetBBLockedNode( NULL );
	}
}

bool AINode::IsNodeLocked()
{
	HOBJECT hLockingAI = GetLockingAI();
	return !!hLockingAI;
}

HOBJECT AINode::GetLockingAI()
{
	if( m_hLockingAI && IsDeadAI( m_hLockingAI ) )
	{
		m_hLockingAI = NULL;
	}

	return m_hLockingAI; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::IsPosInRadiusOrRegion
//
//	PURPOSE:	Return true the specified pos is in the radius or region.
//
// ----------------------------------------------------------------------- //

bool AINode::IsPosInRadiusOrRegion( const LTVector& vPos, ENUM_NMPolyID eNMPoly, float fSearchMult )
{
	// Is the pos within the AIRegion specified on the node.

	if( m_eAIRegion != kAIRegion_Invalid )
	{
		AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( m_eAIRegion );
		if( !( pAIRegion && pAIRegion->ContainsNMPoly( eNMPoly ) ) )
		{
			return false;
		}
	}

	// Is the pos within the node's radius?

	// If radius is zero, do not limit the area.

	else if( GetRadiusSqr() > 0.f ) 
	{
		// Apply search mult.

		float fRadiusSqr;
		if( fSearchMult != 1.f )
		{
			fRadiusSqr = m_fRadius * fSearchMult;
			fRadiusSqr *= fRadiusSqr;
		}
		else {
			fRadiusSqr = GetRadiusSqr();
		}

		// Bail if outside radius.

		float fDistSqr = m_vPos.DistSqr( vPos );
		if( fDistSqr > fRadiusSqr )
		{
			return false;
		}
	}

	// AI is in the radius or region.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::IsAIInRadiusOrRegion
//
//	PURPOSE:	Return true the specified AI is in the radius or region.
//
// ----------------------------------------------------------------------- //

bool AINode::IsAIInRadiusOrRegion( CAI* pAI, const LTVector& vPos, float fSearchMult )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	return IsPosInRadiusOrRegion( vPos, pAI->GetCurrentNavMeshPoly(), fSearchMult );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::IsNodeInRadiusOrRegion
//
//	PURPOSE:	Return true the specified node is in the radius or region.
//
// ----------------------------------------------------------------------- //

bool AINode::IsNodeInRadiusOrRegion( AINode* pNode )
{
	// Sanity check.

	if( !pNode )
	{
		return false;
	}

	return IsPosInRadiusOrRegion( pNode->GetPos(), pNode->GetNodeContainingNMPoly(), 1.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::IsCharacterInRadiusOrRegion
//
//	PURPOSE:	Return true the specified character is in the 
//              node's radius or region.
//
// ----------------------------------------------------------------------- //

bool AINode::IsCharacterInRadiusOrRegion( HOBJECT hChar )
{
	// Sanity check.

	if( !IsCharacter( hChar ) )
	{
		return false;
	}

	LTVector vCharPos;
	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hChar );
	g_pLTServer->GetObjectPos( hChar, &vCharPos );

	return IsPosInRadiusOrRegion( vCharPos, pChar->GetCurrentNavMeshPoly(), 1.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::GetThreatPosition
//
//	PURPOSE:	Return the position of a threat.
//
// ----------------------------------------------------------------------- //

void AINode::GetThreatPosition( CAI* pAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, LTVector* pvPos, ENUM_NMPolyID* peNMPoly )
{
	// Sanity check.

	if( !( pvPos && peNMPoly ) )
	{
		return;
	}
	
	// Don't set a NavMesh position unless the Threat is a character.
	// We could query the QuadTree, but this doesn't seem necessary.

	*peNMPoly = kNMPoly_Invalid;

	// Return the true position.

	if( eThreatPos == kThreatPos_TruePos )
	{
		g_pLTServer->GetObjectPos( hThreat, pvPos );

		if( IsCharacter( hThreat ) )
		{
			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hThreat );
			*peNMPoly = pChar->GetCurrentNavMeshPoly();
		}
		return;
	}

	// Return the Target position.

	if( eThreatPos == kThreatPos_TargetPos )
	{
		// Only use the target position if the threat matches 
		// the AI's current target object.

		if( pAI && ( pAI->GetAIBlackBoard()->GetBBTargetObject() == hThreat ) )
		{
			*pvPos = pAI->GetAIBlackBoard()->GetBBTargetPosition();
			*peNMPoly = pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly();
			return;
		}
	}

	// Default behavior.

	g_pLTServer->GetObjectPos( hThreat, pvPos );

	if( IsCharacter( hThreat ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hThreat );
		*peNMPoly = pChar->GetCurrentNavMeshPoly();
	}
}

// ----------------------------------------------------------------------- //

void AINode::HandleAIArrival( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	LockNode( pAI->m_hObject );

	pAI->GetAIWorldState()->SetWSProp( kWSK_AtNode, pAI->m_hObject, kWST_HOBJECT, m_hObject );
	pAI->GetAIWorldState()->SetWSProp( kWSK_AtNodeType, pAI->m_hObject, kWST_EnumAINodeType, GetType() );

	AITRACE( AIShowNodes, ( pAI->m_hObject, "Arrived at node: %s", GetNodeName() ) );

	if( m_bFaceNode )
	{
		pAI->GetAIBlackBoard()->SetBBFaceDir( m_vFaceDir );
	}
}

void AINode::HandleAIDeparture( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	AITRACE( AIShowNodes, ( pAI->m_hObject, "Departing node: %s", GetNodeName() ) );

	UnlockNode( pAI->m_hObject );

	// Record when someone departed this node.

	m_fNodeDepartureTime = g_pLTServer->GetTime();

	// Clear old positional info.

	pAI->GetAIWorldState()->SetWSProp( kWSK_AtNode, pAI->m_hObject, kWST_HOBJECT, 0 );
	pAI->GetAIWorldState()->SetWSProp( kWSK_AtNodeType, pAI->m_hObject, kWST_EnumAINodeType, kNode_InvalidType );
	pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, pAI->m_hObject, kWST_HOBJECT, 0 );
}

void AINode::ResetActivationTime()
{
	ResetActivationTime( m_fNodeReactivationTime );
}

void AINode::ResetActivationTime(double fResetTime)
{
	m_fNodeLastActivationTime = g_pLTServer->GetTime();

	// Set next valid activation time.

	m_fNodeNextActivationTime = m_fNodeLastActivationTime + fResetTime;
}

bool AINode::IsNodeTimedOut() const
{
	return ( m_fNodeNextActivationTime > g_pLTServer->GetTime() ); 
}

void AINode::ReadProp(const GenericPropList *pProps)
{
	const char* pszPropString = pProps->GetString( "Name", "" );
	if( pszPropString[0] )
	{
		m_strName = pszPropString;
	}

	// Consider the node's y position to be the top of the node's bounding box.
	// This ensures NavMesh searches find the correct containing NavMesh poly.

	m_vPos = pProps->GetVector( "Pos", m_vPos );
	m_vPos.y += NODE_DIMS;

	m_fRadius = pProps->GetReal( "Radius", m_fRadius );

	m_bDynamicOnly = pProps->GetBool( "DynamicOnly", m_bDynamicOnly );

	pszPropString = pProps->GetString( "CharacterType", "" );
	if( pszPropString[0] && !LTStrIEquals( pszPropString, "None" ) )
	{
		ENUM_AIAttributesID eAttributesID = g_pAIDB->GetAIAttributesRecordID( pszPropString );
		m_dwCharTypeMask = ( 1 << eAttributesID );
	}
	
	pszPropString = pProps->GetString( "Region", "" );
	if( pszPropString[0] )
	{
		m_strAIRegion = pszPropString;
	}

	m_EnabledValidator.ReadProps( pProps );
	m_bFaceNode			= pProps->GetBool( "Face", m_bFaceNode );

	m_rRot = pProps->GetRotation( "Rotation", LTRotation::GetIdentity() );

	// Apply the facing offset.

	LTRotation rRotFacing = pProps->GetRotation( "FacingOffset", LTRotation::GetIdentity() );
	rRotFacing = m_rRot * rRotFacing;
	m_vFaceDir = rRotFacing.Forward();

	// Add Node to the NodeMgr.
	AIASSERT(g_pAINodeMgr, m_hObject, "AINode::ReadProp: NodeMgr is NULL.");
	g_pAINodeMgr->AddNode( GetType(), this);
}

void AINode::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(	m_eNodeClusterID );
	SAVE_VECTOR(m_vPos);
	SAVE_ROTATION(m_rRot);
	SAVE_STDSTRING(m_strName);
	SAVE_HOBJECT(m_hLockingAI);
	m_EnabledValidator.Save( pMsg );
	SAVE_FLOAT(m_fRadius);
	SAVE_DWORD(m_eAIRegion);
	SAVE_STDSTRING(m_strAIRegion);
	SAVE_BOOL(m_bFaceNode);
	SAVE_VECTOR(m_vFaceDir);
	SAVE_TIME(m_fNodeNextActivationTime);
	SAVE_DOUBLE(m_fNodeReactivationTime);
	SAVE_TIME(m_fNodeLastActivationTime);
	SAVE_TIME(m_fNodeDepartureTime);
	SAVE_HOBJECT(m_hNodeOwner);
	SAVE_DWORD(m_eContainingNMPoly);
	SAVE_bool(m_bDynamicOnly);

	// Only save requirement if it exists.

	bool bSetRequirement = ( m_dwCharTypeMask != ALL_CHAR_TYPES );
	SAVE_BOOL( bSetRequirement );
	if( !bSetRequirement )
	{
		return;
	}

	// Save name for the flag.

	const char* pszName;
	AIDB_AttributesRecord* pRecord;
	uint32 cCharTypes = g_pAIDB->GetNumAIAttributesRecords();
	for( uint32 iType=0; iType < cCharTypes; ++iType )
	{
		if( m_dwCharTypeMask & ( 1 << iType ) )
		{
			pRecord = g_pAIDB->GetAIAttributesRecord( iType );
			pszName = pRecord ? pRecord->strName.c_str() : "";
			SAVE_CHARSTRING( pszName );
			break;
		}
	}
}

void AINode::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST( m_eNodeClusterID, EnumAINodeClusterID );
	LOAD_VECTOR(m_vPos);
	LOAD_ROTATION(m_rRot);
	LOAD_STDSTRING(m_strName);
	LOAD_HOBJECT(m_hLockingAI);
	m_EnabledValidator.Load( pMsg );
	LOAD_FLOAT(m_fRadius);
	LOAD_DWORD_CAST(m_eAIRegion, ENUM_AIRegionID);
	LOAD_STDSTRING(m_strAIRegion);
	LOAD_BOOL(m_bFaceNode);
	LOAD_VECTOR(m_vFaceDir);
	LOAD_TIME(m_fNodeNextActivationTime);
	LOAD_DOUBLE(m_fNodeReactivationTime);
	LOAD_TIME(m_fNodeLastActivationTime);
	LOAD_TIME(m_fNodeDepartureTime);
	LOAD_HOBJECT(m_hNodeOwner);
	LOAD_DWORD_CAST(m_eContainingNMPoly, ENUM_NMPolyID);
	LOAD_bool(m_bDynamicOnly);

	// Only load requirement if it exists.

	bool bSetRequirement;
	LOAD_BOOL( bSetRequirement );
	if( !bSetRequirement )
	{
			m_dwCharTypeMask = ALL_CHAR_TYPES;
		}

	// Load flag and convert from the name.

	else
	{
		char szName[64];
		LOAD_CHARSTRING( szName, ARRAY_LEN( szName ) );

		ENUM_AIAttributesID eAttributesID = g_pAIDB->GetAIAttributesRecordID( szName );
		m_dwCharTypeMask = ( 1 << eAttributesID );
	}
}

void AINode::Verify()
{
	if ( kNMPoly_Invalid == g_pAIQuadTree->GetContainingNMPoly( m_vPos, m_dwCharTypeMask, kNMPoly_Invalid ) )
	{
		Warn( "AINode \"%s\" is not in a the NavMesh!", m_strName.c_str() );
	}
}

int AINode::DrawSelf()
{
   DebugLineSystem& system = LineSystem::GetSystem(this,"ShowNode");
   system.Clear();
   
	LTVector vNodePos;
	LTRotation rRot;
	g_pLTServer->GetObjectPos(m_hObject, &vNodePos);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
  	
   
   	// Draw the Nodes
	system.AddOrientation(vNodePos, rRot, 16.0f, 255);

	static const float kfBoxDims = 16.0f;
   	system.AddBox(vNodePos, LTVector(kfBoxDims, kfBoxDims, kfBoxDims), GetDebugColor(), 126);

  	char szObjectName[256];
	GetDebugName( szObjectName, LTARRAYSIZE( szObjectName ) );

	system.SetDebugString(szObjectName);
	system.SetDebugStringPos( vNodePos + LTVector(0.0f, kfBoxDims + 16.0f, 0.0f) );

   	return 0;
}
   
int AINode::HideSelf()
{
  	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowNode");
  	system.SetDebugString("");
  	system.Clear( );
  
   	return 0;
}

void AINode::UpdateDebugDrawStatus( HOBJECT hTarget )
{
	bool bNodeIsValid = IsNodeValid( NULL, GetPos(), hTarget, kThreatPos_TruePos, kNodeStatus_All );

	// Redraw the node if its status has changed.

	if( m_bDebugNodeIsValid != bNodeIsValid )
	{
		m_bDebugNodeIsValid = bNodeIsValid;
		HideSelf();
		DrawSelf();
	}
}

void AINode::GetDebugName( char* pszBuffer, uint32 nBufferSize )
{
	g_pLTServer->GetObjectName(m_hObject, pszBuffer, nBufferSize);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::FilterStatusFlags()
//
//  PURPOSE:	Remove from the query any flags which are not supported 
//				by the game.  This allows multiple titles to use the 'all'
//				query without needing to worry about it applying queries
//				which are inappropriate for the title or are only when at 
//				the node or not at the node.
//
// ----------------------------------------------------------------------- //
uint32 AINode::FilterStatusFlags(CAI* pAI, uint32 dwStatusFlags) const
{
	// Allow a null AI pointer as a non exceptional case, as it is used in 
	// the node debug rendering.
	if (!pAI)
	{
		return dwStatusFlags;
	}

	uint32 dwFilteredStatusFlags = dwStatusFlags;
	AIDB_AINodeRecord* pRecord = g_pAIDB->GetAINodeRecord(GetType());
	AIASSERT1(pRecord, m_hObject, "FilterStatusFlags : No record for this type: %s", AINodeUtils::GetNodeTypeName( GetType() ) );
	if ( pRecord )
	{
		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		bool bAtNode =  pProp && pProp->hWSValue == m_hObject;
		if (bAtNode)
		{
			return dwFilteredStatusFlags &= pRecord->dwRelevantAtNodeStatusFlags;
		}
		else
		{
			return dwFilteredStatusFlags &= pRecord->dwRelevantNotAtNodeStatusFlags;
		}
	}

	return 0;
}

LTRESULT AINodePlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// Character Type.

	if ( !LTStrICmp( "CharacterType", szPropName ) )
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

	return LT_UNSUPPORTED;
}

LTRESULT AINodePlugin::PreHook_PropChanged( const char *szObjName, const char *szPropName, const int nPropType, const GenericProp &gpPropValue, ILTPreInterface *pInterface, const char *szModifiers )
{
	if( !LTStrICmp( szPropName, "Command" ) && gpPropValue.GetCommand()[0] )
	{
		ConParse cpCmd;
		cpCmd.Init( gpPropValue.GetCommand() );

		while( LT_OK == pInterface->Parse( &cpCmd ))
		{
			if( cpCmd.m_nArgs > 0 && cpCmd.m_Args[0] )
			{
				if( m_CmdMgrPlugin.CommandExists( cpCmd.m_Args[0] ))
				{
					if( LT_OK != m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
																	szPropName,
																	nPropType, 
																	gpPropValue,
																	pInterface,
																	szModifiers ))
					{
						return LT_UNSUPPORTED;
					}
				}
				else if( cpCmd.m_nArgs > 0 )
				{
					// Since we can send commands to AIs without using the command syntax, 
					// build the command like we were using propper syntax and and try to validate it...

					std::string sCmd = "";
					for( int i = 0; i < cpCmd.m_nArgs; ++i )
					{
						sCmd += cpCmd.m_Args[i];
						sCmd += " ";
					}

					std::string sFinalCommand = "msg <CAIHuman> (";
					sFinalCommand += sCmd;
					sFinalCommand += ')';

					GenericProp gp(sFinalCommand.c_str(), LT_PT_COMMAND);

					if( LT_OK != m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
																		szPropName,
																		nPropType, 
																		gp,
																		pInterface,
																		szModifiers ))
						{
							return LT_UNSUPPORTED;
						}
				}
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeGoto)
	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(Command,		"",									0|PF_NOTIFYCHANGE, "TODO:PROPDESC")
END_CLASS(AINodeGoto, AINode, "This node is used as a destination for a Goto command")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeGoto)
CMDMGR_END_REGISTER_CLASS(AINodeGoto, AINode)


AINodeGoto::AINodeGoto()
{
}

AINodeGoto::~AINodeGoto()
{
}

void AINodeGoto::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	// Read goto command.

	const char* pszPropString = pProps->GetCommand( "Command", "" );
	if ( pszPropString[0] )
	{
		m_strGotoCmd = pszPropString;
	}
}

void AINodeGoto::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_STDSTRING(m_strGotoCmd);
}

void AINodeGoto::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_STDSTRING(m_strGotoCmd);
}

void AINodeGoto::HandleAIArrival( CAI* pAI )
{
	super::HandleAIArrival( pAI );

	if( HasCmd() )
	{
		g_pCmdMgr->QueueCommand( GetCmd(), m_hObject, pAI->m_hObject );
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeAmbush)

	ADD_DEDIT_COLOR( AINodeAmbush )

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")

	// Add Ambush properties.

	ADD_BOOLPROP_FLAG(IgnoreDir,				false,			0, "Should the AI ignore the threat FOV on this cover node?")
	ADD_REALPROP_FLAG(Fov,						60.0f,			0|PF_CONEFOV, "The AI's threat must be within this FOV of the forward vector of the node for it to be valid cover. [Degrees]")
	ADD_REALPROP_FLAG(Radius,					384.0f,			0|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				0|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
	DAMAGED_PROPS()
	AVOID_PROPS()
	PLAYERONNODE_PROPS()
	VALIDFORFOLLOW_PROPS()
	THREATRADIUS_PROPS( 256.0f )
	BOUNDARYRADIUS_PROPS( 1024.0f )
	ADD_REALPROP_FLAG(MinExpiration,			3.0f,			0, "Minimum time AI stays at Ambush node. [Seconds]")
	ADD_REALPROP_FLAG(MaxExpiration,			20.0f,			0, "Maximum time AI stays at Ambush node. [Seconds]")
	ADD_COMMANDPROP_FLAG(Command,"",			0|PF_NOTIFYCHANGE, "Command dispatched when the AI has arrived at this node and starts playing the animation specified via the SmartObject field.")

	// Override the base class version:

	ADD_STRINGPROP_FLAG(SmartObject,			"Ambush", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeAmbush, AINodeSmartObject, 0, AINodeAmbushPlugin, "AI ambush an enemy from an AINodeAmbush")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeAmbush)
CMDMGR_END_REGISTER_CLASS(AINodeAmbush, AINodeSmartObject)

AINodeAmbush::AINodeAmbush()
{
	m_bIgnoreDir = false;
	m_fFovDp = 0.0f;
	m_fMinExpiration = 0.f;
	m_fMaxExpiration = 0.f;
	m_fExpirationTime = 0.f;
}

AINodeAmbush::~AINodeAmbush()
{
}

void AINodeAmbush::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_bIgnoreDir = pProps->GetBool( "IgnoreDir", m_bIgnoreDir );

	m_fFovDp = pProps->GetReal( "Fov", m_fFovDp );
	m_fFovDp = FOV2DP( m_fFovDp );

	m_DamagedValidator.ReadProps( pProps );
	m_AvoidValidator.ReadProps( pProps );
	m_ValidForFollowValidator.ReadProps( pProps );
	m_PlayerOnNodeValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );

	m_fMinExpiration = pProps->GetReal( "MinExpiration", m_fMinExpiration );
	m_fMaxExpiration = pProps->GetReal( "MaxExpiration", m_fMaxExpiration );

	const char* pszPropString = pProps->GetCommand( "Command", "" );
	if ( pszPropString[0] )
	{
		m_strAmbushCmd = pszPropString;
	}
}

void AINodeAmbush::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bIgnoreDir);
	SAVE_FLOAT(m_fFovDp);
	SAVE_FLOAT(m_fMinExpiration);
	SAVE_FLOAT(m_fMaxExpiration);
	SAVE_TIME(m_fExpirationTime);

	m_DamagedValidator.Save( pMsg );
	m_AvoidValidator.Save( pMsg );
	m_ValidForFollowValidator.Save( pMsg );
	m_PlayerOnNodeValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );

	SAVE_STDSTRING(m_strAmbushCmd);
}

void AINodeAmbush::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bIgnoreDir);
	LOAD_FLOAT(m_fFovDp);
	LOAD_FLOAT(m_fMinExpiration);
	LOAD_FLOAT(m_fMaxExpiration);
	LOAD_TIME(m_fExpirationTime);

	m_DamagedValidator.Load( pMsg );
	m_AvoidValidator.Load( pMsg );
	m_ValidForFollowValidator.Load( pMsg );
	m_PlayerOnNodeValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );

	LOAD_STDSTRING(m_strAmbushCmd);
}

void AINodeAmbush::InitNode()
{
	super::InitNode();

	m_ThreatRadiusValidator.InitNodeValidator( this );
	m_BoundaryRadiusValidator.InitNodeValidator( this );
}

bool AINodeAmbush::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// AI wants to avoid this node; it isn't safe to use.

	if( !m_AvoidValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	// Wait a few seconds before anyone goes to a node that someone just left.
	// HACK:  This was hacked in for FEAR.  Should be moved to a node validator.

	if( ( m_fNodeDepartureTime > 0.f ) &&
		( m_fNodeDepartureTime > g_pLTServer->GetTime() - 10.f ) )
	{
		return false;
	}

	// An AI has been damaged at this node; it isn't safe to use.

	if( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}
	if( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, GetDependency() ) )
	{
		return false;
	}

	// Node is locked by someone else.

	if( dwFilteredStatusFlags & kNodeStatus_LockedByOther )
	{
		if( pAI )
		{
			HOBJECT hLockingAI = GetLockingAI();
			if( hLockingAI && ( hLockingAI != pAI->m_hObject ) )
			{
				return false;
			}

			// Node has a dependency that is locked by someone else.

			HOBJECT hDependency = GetDependency();
			if( hDependency )
			{
				AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hDependency );
				if( pNode && 
					pNode->IsNodeLocked() &&
					( pNode->GetLockingAI() != pAI->m_hObject ) )
				{
					return false;
				}
			}
		}
	}

	// Node is disabled.

	if ( !m_EnabledValidator.Evaluate( dwFilteredStatusFlags ) )
	{
		return false;
	}

	// Player is standing on the node.

	if( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	// Node must be valid while following someone.

	if( !m_ValidForFollowValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	// AI is at the node and the node is expired.

	if( dwFilteredStatusFlags & kNodeStatus_Expired )
	{
		if( pAI && ( m_fExpirationTime > 0.f ) )
		{
			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			if( pProp && pProp->hWSValue == m_hObject )
			{
				// Expiration time has passed.

				if( g_pLTServer->GetTime() > m_fExpirationTime )
				{
					// Do not invalidate by the expiration time if AI is scripted to ambush.

					CAIWMFact factQuery;
					factQuery.SetFactType( kFact_Task );
					factQuery.SetTaskType( kTask_Ambush );
					CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
					if( ! ( pFact && pFact->GetTargetObject() == m_hObject ) )
					{
						return false;
					}
				}
			}
		}
	}

	// Threat exists.

	if( hThreat )
	{
		// Precalculate frequently used information so that it does not have 
		// to be done multiple times.

		LTVector vThreatPos;
		ENUM_NMPolyID eThreatNMPoly;
		GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );
		float fNodeDistSqr = m_vPos.DistSqr(vThreatPos);

		LTVector vThreatDir = vThreatPos - m_vPos;
		vThreatDir.y = 0.f;
		vThreatDir.Normalize();

		float flToThreatDp = vThreatDir.Dot(m_rRot.Forward());

		bool bAIIsAtNode = false;
		if( pAI )
		{
			SAIWORLDSTATE_PROP* pAtNodeProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			bAIIsAtNode = (pAtNodeProp && pAtNodeProp->hWSValue == m_hObject );
		}

		// AI is at the node and threat is looking at the AI.

		if( dwFilteredStatusFlags & kNodeStatus_ThreatLookingAtNode )
		{
			if( pAI && bAIIsAtNode )
			{
				SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_TargetIsLookingAtMe, pAI->m_hObject );
				if( pProp && pProp->bWSValue )
				{
					return false;
				}
			}
		}

		// Threat inside radius.

		if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}

		// Threat outside boundary.

		if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}

		// Threat outside FOV.

		if( dwFilteredStatusFlags & kNodeStatus_ThreatOutsideFOV )
		{
			if( !IsIgnoreDir() )
			{
				if ( flToThreatDp <= m_fFovDp )
				{
					return false;
				}
			}
		}

		// Threat is engaged in a melee combat
		// TODO: Implement

		if ( dwFilteredStatusFlags & kNodeStatus_ThreatEngagedInMelee )
		{
		}

		// Threat is outside of both the nodes FOV and the AIs FOV while 
		// the AI is at the node.
		// TODO: This is an absolute check, not a stimulus driven check.

		if ( dwFilteredStatusFlags & kNodeStatus_ThreatInUnexpectedLocation )
		{
			if ( pAI && bAIIsAtNode && flToThreatDp <= m_fFovDp )
			{
				// Is the threat also outside of the AIs FOV?  The AIs FOV defines the 
				// expected incoming approach for the enemy.  If they are outside of the node 
				// FOV but inside of the AIs FOV, ambush is still valid.
				if (!pAI->IsInsideFOV(vThreatDir.MagSqr(), vThreatDir))
				{
					return false;
				}
			}
		}

		// Threat is outside of the nodes FOV AND inside the AIs FOV AND facing away from the AI

		if ( dwFilteredStatusFlags & kNodeStatus_ThreatIsAtDisadvantage )
		{
			// Verify the threat is outside of the nodes FOV

			if ( pAI && ( flToThreatDp <= m_fFovDp ) )
			{
				// Verify that the threat it inside the AIs FOV

				if (pAI->IsInsideFOV(vThreatDir.MagSqr(), vThreatDir))
				{
					// Verify that the threat is a character facing away from the AI.

					if (IsCharacter(hThreat))
					{
						CCharacter* pThreat = (CCharacter*)g_pLTServer->HandleToObject(hThreat);
						LTRigidTransform rThreatRotation;
						pThreat->GetViewTransform(rThreatRotation);
						LTVector vThreadForward = rThreatRotation.m_rRot.Forward();
						vThreadForward.y = 0.f;
						vThreadForward.Normalize();
						if ( 0 > vThreadForward.Dot(-vThreatDir))
						{
							return false;
						}
					}
				}
			}
		}

		// Threat is looking at the AI while outside of the nodes FOV if the AI is 
		// at the node, or the threat is outside of the FOV.

		if ( dwFilteredStatusFlags & kNodeStatus_ThreatLookingAtNodeOutsideNodeFOV )
		{
			if( pAI && bAIIsAtNode )
			{
				// Verify that the target is outside of the nodes FOV

				if ( flToThreatDp <= m_fFovDp )
				{
					// Verify that the target is looking at the AI.

					SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_TargetIsLookingAtMe, pAI->m_hObject );
					if( pProp && pProp->bWSValue )
					{
						return false;
					}
				}
			}
		}

		// Threat is aware of the AIs location

		if ( pAI && ( dwFilteredStatusFlags & kNodeStatus_ThreatAwareOfPosition ) )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Knowledge );
			factQuery.SetKnowledgeType( kKnowledge_EnemyKnowsPosition );
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if (pFact
				&& (pFact->GetConfidence(CAIWMFact::kFactMask_KnowledgeType) > g_pAIDB->GetMiscFloat( "TargetIsAwareOfPositionMaxAmbush" ) ))
			{
				return false;
			}
		}
	}

	// Node is valid.

	return true;
}

void AINodeAmbush::HandleAIArrival( CAI* pAI )
{
	super::HandleAIArrival( pAI );

	// Calculate a random expiration time.

	if( ( m_fMinExpiration > 0.f ) &&
		( m_fMaxExpiration > 0.f ) )
	{
		m_fExpirationTime = g_pLTServer->GetTime() + GetRandom( m_fMinExpiration, m_fMaxExpiration );
	}

	if( HasCmd() )
	{
		g_pCmdMgr->QueueCommand( GetCmd(), m_hObject, pAI->m_hObject );
	}
}

DebugLine::Color AINodeAmbush::GetDebugColor()
{
	if( m_bDebugNodeIsValid )
	{
		return Color::Yellow;
	}

	return super::GetDebugColor();
}

// ----------------------------------------------------------------------- //

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINODECOVER CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINODECOVER 0

#endif

BEGIN_CLASS(AINodeCover)

	ADD_DEDIT_COLOR( AINodeCover )

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(ReactivationTime,			0.f,			PF_HIDDEN, "TODO:PROPDESC")

	// Add Cover properties.

	ADD_BOOLPROP_FLAG(IgnoreDir,				false,			0, "Should the AI ignore the threat FOV on this cover node?")
	ADD_REALPROP_FLAG(Fov,						20.0f,			0|PF_CONEFOV, "The AI's threat must be within this FOV of the forward vector of the node for it to be valid cover. [Degrees]")
	ADD_REALPROP_FLAG(Radius,					800.0f,			0|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				0|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
	DAMAGED_PROPS()
	AVOID_PROPS()
	PLAYERONNODE_PROPS()
	VALIDFORFOLLOW_PROPS()
	THREATRADIUS_PROPS( 500.0f )
	BOUNDARYRADIUS_PROPS( 1000.0f )
	ADD_BOOLPROP_FLAG(ThrowGrenades,			true,			0, "Should the AI throw grenades from this cover node?")

	// Override the base class version:

	ADD_STRINGPROP_FLAG(SmartObject,			"CoverStep", 	0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeCover, AINodeSmartObject, CF_HIDDEN_AINODECOVER, AINodeCoverPlugin, "AI take cover from enemy fire at AINodeCover nodes")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeCover)
CMDMGR_END_REGISTER_CLASS(AINodeCover, AINodeSmartObject)

AINodeCover::AINodeCover()
{
	m_bIgnoreDir = false;
	m_fFovDp = 0.0f;
	m_bThrowGrenades = false;
}

AINodeCover::~AINodeCover()
{
}

void AINodeCover::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_bIgnoreDir = pProps->GetBool( "IgnoreDir", m_bIgnoreDir );

	m_fFovDp = pProps->GetReal( "Fov", m_fFovDp );
	m_fFovDp = FOV2DP( m_fFovDp );

	m_DamagedValidator.ReadProps( pProps );
	m_AvoidValidator.ReadProps( pProps );
	m_ValidForFollowValidator.ReadProps( pProps );
	m_PlayerOnNodeValidator.ReadProps( pProps );
	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );

	m_bThrowGrenades = pProps->GetBool( "ThrowGrenades", m_bThrowGrenades );
}

void AINodeCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bIgnoreDir);
	SAVE_FLOAT(m_fFovDp);
	SAVE_bool(m_bThrowGrenades);

	m_DamagedValidator.Save( pMsg );
	m_AvoidValidator.Save( pMsg );
	m_ValidForFollowValidator.Save( pMsg );
	m_PlayerOnNodeValidator.Save( pMsg );
	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );
}

void AINodeCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bIgnoreDir);
	LOAD_FLOAT(m_fFovDp);
	LOAD_bool(m_bThrowGrenades);

	m_DamagedValidator.Load( pMsg );
	m_AvoidValidator.Load( pMsg );
	m_ValidForFollowValidator.Load( pMsg );
	m_PlayerOnNodeValidator.Load( pMsg );
	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );
}

void AINodeCover::InitNode()
{
	super::InitNode();

	m_ThreatRadiusValidator.InitNodeValidator( this );
	m_BoundaryRadiusValidator.InitNodeValidator( this );
}

void AINodeCover::Verify() 
{
	super::Verify();
}

bool AINodeCover::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// AI wants to avoid this node; it isn't safe to use.

	if( !m_AvoidValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	// Skip many of the validation tests if AI has been scripted to go to this node.

	if( pAI )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_Cover );
		factQuery.SetTargetObject(m_hObject);
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			return true;
		}
	}

	// Wait a few seconds before anyone goes to a node that someone just left.
	// HACK:  This was hacked in for FEAR.  Should be moved to a node validator.

	if( ( m_fNodeDepartureTime > 0.f ) &&
	    ( m_fNodeDepartureTime > g_pLTServer->GetTime() - 10.f ) )
	{
		return false;
	}

	// An AI has been damaged at this node; it isn't safe to use.

	if( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}
	if( !m_DamagedValidator.Evaluate( dwFilteredStatusFlags, pAI, GetDependency() ) )
	{
		return false;
	}

	// Node is locked by someone else.

	bool bCoverBehindAlly = false;
	if( dwFilteredStatusFlags & kNodeStatus_LockedByOther )
	{
		if( pAI )
		{
			HOBJECT hLockingAI = GetLockingAI();
			if( hLockingAI && ( hLockingAI != pAI->m_hObject ) )
			{
				return false;
			}

			// Node has a dependency.

			HOBJECT hDependency = GetDependency();
			if( hDependency )
			{
				AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( hDependency );
				if( pNodeSmartObject )
				{
					AIDB_SmartObjectRecord* pRecord = pNodeSmartObject->GetSmartObject();
					if( pRecord )
					{
						// Destination dependency is locked by someone else.

						if( ( pRecord->eDependencyType == kDependency_Destination ) &&
							( pNodeSmartObject->IsNodeLocked() ) &&
							( pNodeSmartObject->GetLockingAI() != pAI->m_hObject ) )
						{
							return false;
						}

						// Occupation dependency is not occupied by someone else.

						if( pRecord->eDependencyType == kDependency_Occupied )
						{
							// AI is taking cover behind someone.

							bCoverBehindAlly = true;

							// Node is unusable if dependency is disabled.

							if( pNodeSmartObject->IsNodeDisabled() )
							{
								return false;
							}

							// Node is unusable if locking AI does not exist.

							HOBJECT hLockingAI = pNodeSmartObject->GetLockingAI();
							CAI* pLockingAI = (CAI*)g_pLTServer->HandleToObject( hLockingAI );
							if( !pLockingAI )
							{
								return false;
							}

							// Node is not occupied if the locking AI is not at the node.

							SAIWORLDSTATE_PROP* pAtProp = pLockingAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pLockingAI->m_hObject );
							if( !( pAtProp && pAtProp->hWSValue == hDependency ) )
							{
								return false;
							}
						}
					}
				}
			}

			// Node has a cluster that is locked by someone else.

			if( m_eNodeClusterID != kNodeCluster_Invalid )
			{
				CAINodeCluster* pCluster = g_pAINodeMgr->GetNodeCluster( m_eNodeClusterID );
				if( pCluster &&
					pCluster->IsClusterLocked() &&
					( pCluster->GetLockingAI() != pAI->m_hObject ) )
				{
					return false;
				}
			}
		}
	}

	// Node is disabled.

	if ( !m_EnabledValidator.Evaluate( dwFilteredStatusFlags ) )
	{
		return false;
	}

	// Player is standing on the node.

	if( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	// Node must be valid while following someone.

	if( !m_ValidForFollowValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	// AI is at the node has not seen the threat in some time limit.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatUnseen )
	{
		if( pAI )
		{
			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			if( pProp && pProp->hWSValue == m_hObject )
			{
				double fNodeArrivalTime = pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime();
				float fAtNodeTime = (float)(g_pLTServer->GetTime() - fNodeArrivalTime);
				if( fAtNodeTime > g_pAIDB->GetAIConstantsRecord()->fThreatUnseenTime )
				{
					CAIWMFact factQuery;
					factQuery.SetFactType( kFact_Character );
					factQuery.SetTargetObject( hThreat );
					CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
					if( pFact )
					{
						float fTimeDelta = (float)(g_pLTServer->GetTime() - pFact->GetUpdateTime());
						if( fTimeDelta > g_pAIDB->GetAIConstantsRecord()->fThreatUnseenTime )
						{
							return false;
						}
					}
				}
			}
		}
	}

	// AI is at the node and threat is aiming at the AI.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatAimingAtNode )
	{
		// Require the AI to be at the node for at least a few 
		// seconds, to ensure he fires his weapon.
		// Ignore threat aiming if AI is covering behind an ally.

		if( pAI && !bCoverBehindAlly )
		{
			double fNodeArrivalTime = pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime();
			if( g_pLTServer->GetTime() > fNodeArrivalTime + 2.f )
			{
				SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
				if( pProp && pProp->hWSValue == m_hObject )
				{
					pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_TargetIsAimingAtMe, pAI->m_hObject );
					if( pProp && pProp->bWSValue )
					{
						return false;
					}
				}
			}
		}
	}

	// All further checks depend on the presence of a threat.

	if( ( !hThreat ) ||
		( pAI && pAI->GetAIBlackBoard()->GetBBTargetType() == kTarget_Leader ) ||
		( pAI && pAI->GetAIBlackBoard()->GetBBTargetType() == kTarget_Interest ) )
	{
		return true;
	}

	// Check if the threat is too close to the AI,
	// and is blocking the path to the node.

	LTVector vThreatPos;
	ENUM_NMPolyID eThreatNMPoly;
	GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );

	if( dwFilteredStatusFlags & kNodeStatus_ThreatBlockingPath )
	{
		float fAIDistSqr = vPosAI.DistSqr(vThreatPos);
		if( fAIDistSqr < g_pAIDB->GetAIConstantsRecord()->fThreatTooCloseDistanceSqr )
		{
			LTVector vToThreat = vThreatPos - vPosAI;
			LTVector vToNode = m_vPos - vPosAI;

			if( vToThreat.Dot( vToNode ) > 0.f )
			{
				return false;
			}
		}
	}

	float fNodeDistSqr = m_vPos.DistSqr(vThreatPos);

	// Threat inside radius.

	if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		return false;
	}

	// Threat outside boundary.

	if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
	{
		return false;
	}

	// Threat outside FOV.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatOutsideFOV )
	{
		if( !IsIgnoreDir() )
		{
			LTVector vThreatDir = vThreatPos - m_vPos;
			vThreatDir.y = 0.f;
			vThreatDir.Normalize();

			if ( vThreatDir.Dot(m_rRot.Forward()) <= m_fFovDp )
			{
				return false;
			}
		}
	}

	// Node is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeCover::IsAIInRadiusOrRegion
//
//	PURPOSE:	Return true the specified AI is in the radius or region.
//
// ----------------------------------------------------------------------- //

bool AINodeCover::IsAIInRadiusOrRegion( CAI* pAI, const LTVector& vPos, float fSearchMult )
{
	// Encourage AI to use cover nodes depending on occupied nodes.
	// Lying about the radius will make AI prefer to use these nodes.

	if( IsAINodeSmartObject( GetDependency() ) )
	{
		AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( GetDependency() );
		if( pNodeSmartObject->GetSmartObject() &&
			pNodeSmartObject->GetSmartObject()->eDependencyType == kDependency_Occupied )
		{
			return true;
		}
	}

	// Default behavior.

	return super::IsAIInRadiusOrRegion( pAI, vPos, fSearchMult );
}

void AINodeCover::HandleAIArrival( CAI* pAI )
{
	super::HandleAIArrival( pAI );

	// Apply cover status.

	pAI->GetAIWorldState()->SetWSProp( kWSK_CoverStatus, pAI->m_hObject, kWST_EnumAnimProp, kAP_ATVT_Uncovered );
}

void AINodeCover::HandleAIDeparture( CAI* pAI )
{
	super::HandleAIDeparture( pAI );

	// Clear cover status.

	pAI->GetAIWorldState()->SetWSProp( kWSK_CoverStatus, pAI->m_hObject, kWST_EnumAnimProp, kAP_None );
}

void AINodeCover::GetAnimProps( CAnimationProps* pProps )
{
	super::GetAnimProps( pProps );

	// Sanity check.

	if( !pProps )
	{
		return;
	}

	// The Evasive prop depends on the SmartObject and direction 
	// of the Cover Node.

	// Reverse the Right/Left if the node is rotated.

	if( m_rRot.Up().y < 0.0f )
	{
		if( pProps->Get( kAPG_MovementDir ) == kAP_MDIR_Right )
		{
			pProps->Set( kAPG_MovementDir, kAP_MDIR_Left );
		}

		if( pProps->Get( kAPG_MovementDir ) == kAP_MDIR_BackRight )
		{
			pProps->Set( kAPG_MovementDir, kAP_MDIR_BackLeft );
		}
	}
}

DebugLine::Color AINodeCover::GetDebugColor()
{
	if( m_bDebugNodeIsValid )
	{
		return Color::Yellow;
	}

	return super::GetDebugColor();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeView)

	ADD_DEDIT_COLOR( AINodeView )
	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(Radius,			512.0f,								0|PF_RADIUS, "TODO:PROPDESC")
	ADD_STRINGPROP_FLAG(Region,			"",									0|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
	PLAYERONNODE_PROPS()
	VALIDFORFOLLOW_PROPS()

END_CLASS(AINodeView, AINode, "An AI will choose to attack from a View nodes when no path can be found to the target and a valid View node is available that points to the area of the level that currently holds the target.")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeView)
CMDMGR_END_REGISTER_CLASS(AINodeView, AINode)

AINodeView::AINodeView()
{
}

AINodeView::~AINodeView()
{
}

void AINodeView::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);
	m_ValidForFollowValidator.ReadProps( pProps );
	m_PlayerOnNodeValidator.ReadProps( pProps );
}

void AINodeView::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	m_ValidForFollowValidator.Save( pMsg );
	m_PlayerOnNodeValidator.Save( pMsg );
}

void AINodeView::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	m_ValidForFollowValidator.Load( pMsg );
	m_PlayerOnNodeValidator.Load( pMsg );
}

void AINodeView::Verify() 
{
	super::Verify();
}

bool AINodeView::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// Player is standing on the node.

	if( !m_PlayerOnNodeValidator.Evaluate( dwFilteredStatusFlags, pAI, m_vPos ) )
	{
		return false;
	}

	// Node must be valid while following someone.

	if( !m_ValidForFollowValidator.Evaluate( dwFilteredStatusFlags, pAI, m_hObject ) )
	{
		return false;
	}

	// Bail if threat is not a character.

	if( !IsCharacter( hThreat ) )
	{
		return false;
	}
	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hThreat );
	if( !pChar )
	{
		return false;
	}

	// Threat outside regions.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatOutsideRegions )
	{
		// Bail if threat is not in the NavMesh.

		ENUM_NMPolyID ePoly = pChar->GetCurrentNavMeshPoly();
		if( ePoly == kNMPoly_Invalid )
		{
			return false;
		}

		// Bail if no NavMesh poly.

		CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
		if( !pPoly )
		{
			return false;
		}

		// Iterate over all AIRegions that the NavMesh poly is a part of.

		HOBJECT hNode;
		int cViewNodes, iViewNode;

		ENUM_AIRegionID eAIRegion;
		int cAIRegions = pPoly->GetNumAIRegions();
		for( int iAIRegion=0; iAIRegion < cAIRegions; ++iAIRegion )
		{
			// Skip invalid AIRegions.

			eAIRegion = pPoly->GetAIRegion( iAIRegion );
			if( eAIRegion == kAIRegion_Invalid )
			{
				continue;
			}

			AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( eAIRegion );
			if( !pAIRegion )
			{
				continue;
			}

			// Iterate over nodes in AIRegion, adding valid nodes to the list.

			cViewNodes = pAIRegion->GetNumViewNodes();
			for( iViewNode=0; iViewNode < cViewNodes; ++iViewNode )
			{
				hNode = pAIRegion->GetViewNode( iViewNode );

				// Threat is in an AIRegion that corresponds to this View Node.

				if( hNode == m_hObject )
				{
					return true;
				}
			}
		}

		// Threat is NOT in an AIRegion that corresponds to this View Node.

		return false;
	}
	
	// Node is valid.

	return true;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS( AINodeSearch )

	ADD_DEDIT_COLOR( AINodeSearch )

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use. Must be an Alarm or Switch object.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")

	// Override the base class version:

	ADD_STRINGPROP_FLAG(SmartObject,			"Search", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

	ADD_BOOLPROP_FLAG(DynamicOnly,				false,			0, "AI may not use this node in scripted situations when DynamicOnly is True.")

	// Add a list of Points of Interest.

	ADD_NAMED_OBJECT_LIST_AGGREGATE( PointsOfInterest, PF_GROUP(1), Node, "TODO:GROUPDESC", "TODO:PROPDESC" )

	// Add optional location.

	ADD_STRINGPROP_FLAG(Location,	"None", 						PF_STATICLIST,		"Location associated with this AIRegion.")

END_CLASS_FLAGS_PLUGIN( AINodeSearch, AINodeSmartObject, 0, AINodeSearchPlugin, "While searching for a threat, AI stop at search nodes and play animations" )

CMDMGR_BEGIN_REGISTER_CLASS( AINodeSearch )
CMDMGR_END_REGISTER_CLASS( AINodeSearch, AINodeSmartObject )

AINodeSearch::AINodeSearch()
{
	AddAggregate( &m_PointOfInterestNodes );

	m_eLocationAISoundType = kAIS_InvalidType;
}

AINodeSearch::~AINodeSearch()
{
}

void AINodeSearch::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_PointOfInterestNodes.ReadProp( pProps, "Node" );

	// Read the location label.

	const char* pszPropString = pProps->GetString( "Location", "" );
	if( pszPropString[0] && !LTStrIEquals( pszPropString, "None" ) )
	{
		std::string strLocation = "SearchCheck";
		strLocation += pszPropString;
		m_eLocationAISoundType = (EnumAISoundType)g_pAIDB->String2EnumIndex( strLocation.c_str(), kAIS_Count, (uint32) kAIS_InvalidType, s_aszAISoundTypes );
	}
}

void AINodeSearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_eLocationAISoundType);
}

void AINodeSearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_eLocationAISoundType, EnumAISoundType);
}

void AINodeSearch::AllNodesInitialized()
{
	super::AllNodesInitialized();

	m_PointOfInterestNodes.InitNamedObjectList( m_hObject );

	// Get the Point of Interest nodes.
	// Point of Interest nodes will be owned by this search node.  Nodes that are
	// owned may only be used by the AI who has locked to this search node.

	HOBJECT hNode;
	const char* pszNodeName;
	AINode* pNode;
	uint32 cNodes = m_PointOfInterestNodes.GetNumObjectNames();
	for( uint32 iNode=0; iNode < cNodes; ++iNode )
	{
		// Node not found.

		pszNodeName = m_PointOfInterestNodes.GetObjectName( iNode );
		hNode = m_PointOfInterestNodes.GetObjectHandle( iNode );
		if( !hNode )
		{
			AIASSERT1( 0, m_hObject, "AINodeSearch::Init: Cannot find Point of Interest node \"%s\"", pszNodeName );
			continue;
		}

		// Not a node.

		if( !IsAINode( hNode ) )
		{
			AIASSERT1( 0, m_hObject, "AINodeSearch::Init: Point of Interest object is not an AINode \"%s\"", pszNodeName );
			continue;
		}

		// Node does not exist.

		pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( !pNode )
		{
			AIASSERT1( 0, m_hObject, "AINodeSearch::Init: Cannot find Point of Interest node \"%s\"", pszNodeName );
			continue;
		}

		// Set owner to prevent other AI from using this node.

		pNode->SetNodeOwner( m_hObject );
	}

	m_PointOfInterestNodes.ClearStrings();
}

bool AINodeSearch::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if( !super::IsNodeValid( pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags ) )
	{
		return false;
	}

	// Search node is invalid if it points to Points of Interest that are timed out.

	uint32 cPointsOfInterest = m_PointOfInterestNodes.GetNumObjectHandles();
	if( cPointsOfInterest > 0 )
	{
		AINode* pNode;
		HOBJECT hNode;
		bool bNodeAvailable = false;
		for( uint32 iPoint=0; iPoint < cPointsOfInterest; ++iPoint )
		{
			hNode = m_PointOfInterestNodes.GetObjectHandle( iPoint );
			if( IsAINode( hNode ) )
			{
				pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
				if( !pNode->IsNodeTimedOut() )
				{
					bNodeAvailable = true;
					break;
				}
			}
		}

		// All Points of Interest have timed out.

		if( !bNodeAvailable )
		{
			return false;
		}
	}

	return true;
}

LTRESULT AINodeSearchPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if( LTStrIEquals( "Location", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "None" );

		strcpy( aszStrings[(*pcStrings)++], "Downstairs" );
		strcpy( aszStrings[(*pcStrings)++], "Inside" );
		strcpy( aszStrings[(*pcStrings)++], "Lab" );
		strcpy( aszStrings[(*pcStrings)++], "Office" );
		strcpy( aszStrings[(*pcStrings)++], "Outside" );
		strcpy( aszStrings[(*pcStrings)++], "Stairwell" );
		strcpy( aszStrings[(*pcStrings)++], "Upstairs" );

		return LT_OK;
	}

	// Let the superclass handle it.

	return super::PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeSmartObject)

	ADD_STRINGPROP_FLAG(ModelName,				"", PF_HIDDEN | PF_MODEL, "Specifies the ." RESEXT_MODEL_COMPRESSED " file the model's geometry is in.")
	ADD_STRINGPROP_FLAG(Object,					"",				0|PF_OBJECTLINK, "The name of the object that the AI is to use.")
	ADD_STRINGPROP_FLAG(Dependency,				"",				0|PF_OBJECTLINK, "Node that AI must visit first before visiting this node.")
	ADD_REALPROP_FLAG(Radius,					512.0f,			0|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				0|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
    ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				0|PF_NOTIFYCHANGE, "Command to run upon arrival at the node.")
    ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				0|PF_NOTIFYCHANGE, "Command to run upon departure from the node.")
    ADD_REALPROP_FLAG(ReactivationTime,			0.f,			0, "Number of seconds before anyone can use this node again.")
	ADD_STRINGPROP_FLAG(FirstSound,				"None", 		0|PF_STATICLIST, "Sound to play upon arrival at the node.")
	ADD_STRINGPROP_FLAG(FidgetSound,			"None", 		0|PF_STATICLIST, "Sound to play during intermittent fidget animations.")

END_CLASS_FLAGS_PLUGIN(AINodeSmartObject, AINode, CF_HIDDEN, AINodeSmartObjectPlugin, "AINodeSmartObject is a node that uses a SmartObject template to describe the AI's behavior at the node")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeSmartObject)
CMDMGR_END_REGISTER_CLASS(AINodeSmartObject, AINode)

AINodeSmartObject::AINodeSmartObject()
{
	m_hDependency	= NULL;
	m_nSmartObjectID = (uint32)-1;
}

AINodeSmartObject::~AINodeSmartObject()
{
}

void AINodeSmartObject::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	const char* pszPropString = pProps->GetString( "Object", "" );
	if ( pszPropString[0] )
	{
		m_strObject = pszPropString;
	}

	pszPropString = pProps->GetString( "Dependency", "" );
	if ( pszPropString[0] )
	{
		m_strDependency = pszPropString;
	}

	m_fNodeReactivationTime = pProps->GetReal( "ReactivationTime", (float)m_fNodeReactivationTime );

	// Read activate commands.

	pszPropString = pProps->GetCommand( "PreActivateCommand", "" );
	m_strPreActivateCommand = pszPropString ? pszPropString : "";

	pszPropString = pProps->GetCommand( "PostActivateCommand", "" );
	m_strPostActivateCommand = pszPropString ? pszPropString : "";

	// Read SmartObject type.

	AIASSERT(g_pAINodeMgr, m_hObject, "AINodeSmartObject::ReadProp: NodeMgr is NULL.");

	AIDB_SmartObjectRecord* pSmartObject = NULL;
	pszPropString = pProps->GetString( "SmartObject", "" );
	if( pszPropString[0] )
	{
		m_nSmartObjectID = g_pAIDB-> GetAISmartObjectRecordID( pszPropString );
		AIASSERT(m_nSmartObjectID != kAISmartObjectID_Invalid, m_hObject, "AINodeSmartObject::ReadProp: SmartObject is NULL.");
	}
}

void AINodeSmartObject::InitNode()
{
	// This is not a valid object.
	if( m_nSmartObjectID != -1 )
	{
		// Cache childmodels here.
		// Look up the smartobject in the GDB:
		AIDB_SmartObjectRecord *pSmartObject;
		pSmartObject = g_pAIDB->GetAISmartObjectRecord(m_nSmartObjectID);

		// Cache a pointer to the animation object.
		HOBJECT hObj = NULL;
		FindNamedObject(m_strObject.c_str(), hObj, false);
		m_hAnimObject = hObj;

		// Offset the node's position.
		// Do this BEFORE finding the containing NavMesh poly.

		if( pSmartObject->fNodeOffset != 0.f )
		{
			LTVector vOffset = m_vFaceDir;
			vOffset *= pSmartObject->fNodeOffset;

			m_vPos -= vOffset;
			g_pLTServer->SetObjectPos( m_hObject, m_vPos );
		}
	}

	// Default initialization.
	// Do this AFTER offsetting the node's position.

	super::InitNode();
}

HOBJECT	AINodeSmartObject::GetDependency()
{
	if ( ( !m_hDependency ) && !m_strDependency.empty() )
	{
		HOBJECT hDependency = NULL;
		LTRESULT res = FindNamedObject( m_strDependency.c_str(), hDependency );
		AIASSERT1(( res == LT_OK ), m_hObject, "AINodeSmartObject::GetDependency: Dependency Object '%s' does not exist.", m_strDependency.c_str() );
		m_hDependency = hDependency;
	}

	return m_hDependency;
}

AIDB_SmartObjectRecord* AINodeSmartObject::GetSmartObject()
{
	return g_pAIDB->GetAISmartObjectRecord( m_nSmartObjectID );
}

void AINodeSmartObject::PreActivate()
{
	// Run the node's pre-activate command.

	if( !m_strPreActivateCommand.empty() )
	{
		g_pCmdMgr->QueueCommand( m_strPreActivateCommand.c_str(), this, g_pLTServer->HandleToObject(m_hAnimObject) );
	}
}

void AINodeSmartObject::PostActivate()
{
	// Run the node's post-activate command.

	if( !m_strPostActivateCommand.empty() )
	{
		g_pCmdMgr->QueueCommand( m_strPostActivateCommand.c_str(), this, g_pLTServer->HandleToObject(m_hAnimObject) );
	}

	ResetActivationTime();
}

bool AINodeSmartObject::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	LTVector vThreatPos;
	ENUM_NMPolyID eThreatNMPoly;
	GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );

	// Threat blocking path.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatBlockingPath )
	{
		// Check if the threat is too close to the AI,
		// and is blocking the path to the node.

		if ( vPosAI.DistSqr(vThreatPos) < g_pAIDB->GetAIConstantsRecord()->fThreatTooCloseDistanceSqr )
		{
			LTVector vToThreat = vThreatPos - vPosAI;
			LTVector vToNode = m_vPos - vPosAI;

			if( vToThreat.Dot( vToNode ) > c_fFOV60 )
			{
				return false;
			}
		}
	}

	// Threat inside radius.

	if( dwFilteredStatusFlags & kNodeStatus_ThreatInsideRadius )
	{
		if ( m_vPos.DistSqr(vThreatPos) < g_pAIDB->GetAIConstantsRecord()->fThreatTooCloseDistanceSqr )
		{
			return false;
		}
	}

	// Node is valid.

	return true;
}

void AINodeSmartObject::GetAnimProps( CAnimationProps* pProps )
{
	// Sanity check.

	if( !pProps )
	{
		return;
	}

	// Get the anim props from the SmartObject.

	AIDB_SmartObjectRecord *pSmartObject = GetSmartObject();
	if( pSmartObject )
	{
		*pProps = pSmartObject->Props;
	}
}

void AINodeSmartObject::GetDebugName( char* pszBuffer, const uint32 nBufferSize )
{
	super::GetDebugName( pszBuffer, nBufferSize );

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( pSmartObject )
	{
		LTStrCat( pszBuffer, "\n", nBufferSize );
		LTStrCat( pszBuffer, pSmartObject->strName.c_str(), nBufferSize );
	}
}

void AINodeSmartObject::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_STDSTRING(m_strObject);
	SAVE_HOBJECT(m_hAnimObject);
	SAVE_STDSTRING(m_strDependency);
	SAVE_HOBJECT(m_hDependency);
	SAVE_STDSTRING(m_strPreActivateCommand);
	SAVE_STDSTRING(m_strPostActivateCommand);

	std::string strSmartObject;
	strSmartObject = g_pAIDB->GetAISmartObjectRecordName( (ENUM_AISmartObjectID)m_nSmartObjectID );
	SAVE_STDSTRING( strSmartObject );
}

void AINodeSmartObject::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_STDSTRING(m_strObject);
	LOAD_HOBJECT(m_hAnimObject);
	LOAD_STDSTRING(m_strDependency);
	LOAD_HOBJECT(m_hDependency);
	LOAD_STDSTRING(m_strPreActivateCommand);
	LOAD_STDSTRING(m_strPostActivateCommand);

	std::string strSmartObject;
	LOAD_STDSTRING( strSmartObject );
	m_nSmartObjectID = g_pAIDB->GetAISmartObjectRecordID( strSmartObject.c_str() );
}

// ------------------------------------------------------------------------
// ChildModels( obj, bInit )
// start means we 
// add/remove childmodels 
// ------------------------------------------------------------------------
void AINodeSmartObject::ApplyChildModels( HOBJECT hObj ) 
{ 
	AIDB_SmartObjectRecord *pSmartObject = g_pAIDB->GetAISmartObjectRecord(m_nSmartObjectID);
	if( !pSmartObject )
	{
		return;
	}

	AIChildModelInfo* pAIChildModelInfo;
	AICHILD_MODEL_INFO_LIST::iterator itChild;
	for( itChild = pSmartObject->lstChildModels.begin(); itChild != pSmartObject->lstChildModels.end(); ++itChild )
	{
		pAIChildModelInfo = &(*itChild);
		//g_pModelLT->AddChildModelDB( hObj, pAIChildModelInfo->strFilename.c_str() );
	}
}



AINodeSmartObjectPlugin::AINodeSmartObjectPlugin()
{
	// Nodes of type base are valid in all lists.  "None" is an example of
	// type invalid or base type.
	m_ValidNodeTypes.push_back(kNode_InvalidType);
	m_ValidNodeTypes.push_back(kNode_Base);
}

LTRESULT AINodeSmartObjectPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{	
	if ( !LTStrICmp("Movement", szPropName) )
	{
		strcpy(aszStrings[(*pcStrings)++], "Walk");
		strcpy(aszStrings[(*pcStrings)++], "Run");

		return LT_OK;
	}

	if ( !LTStrICmp("SmartObject", szPropName) )
	{
		uint32 cSmartObjects = g_pAIDB->GetNumAISmartObjectRecords();
		for(uint32 iSmartObject=0; iSmartObject < cSmartObjects; ++iSmartObject)
		{
			// Out of space to add more strings.

			if ( (*pcStrings) + 1 == cMaxStrings )
			{
				break;
			}

			// If the currently indexed smartobject is of the same type as the node, 
			// then add it to the list.

			const AIDB_SmartObjectRecord* pSmartObject = g_pAIDB->GetAISmartObjectRecord(iSmartObject);
			if ( !pSmartObject )
			{
				continue;
			}
			
			// Filter the list entry based on the node type.
			NodeTypeList::iterator it = std::find(
				m_ValidNodeTypes.begin(), m_ValidNodeTypes.end(), 
				pSmartObject->eNodeType);

			if (it != m_ValidNodeTypes.end())
			{
				LTStrCpy(aszStrings[(*pcStrings)++], pSmartObject->strName.c_str(), cMaxStringLength);
			}
		}

		// Alphabetize the strings, skipping the 'None' entry which is always first.

		if (*pcStrings > 1)
		{
			qsort( aszStrings+1, (*pcStrings)-1, sizeof( char * ), CaseInsensitiveCompare );		
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT AINodeSmartObjectPlugin::PreHook_Dims(const char* szRezPath, const char* szPropName, const char* szPropValue, 
											   char* szModelFilenameBuf, int nModelFilenameBufLen, 
											   LTVector & vDims, 
											   const char* pszObjName, ILTPreInterface *pInterface)
{
	vDims = LTVector( NODE_DIMS, NODE_DIMS, NODE_DIMS );

	ENUM_AISmartObjectID eSmartObject = g_pAIDB->GetAISmartObjectRecordID(szPropValue);
	AIDB_SmartObjectRecord* pSmartObject = g_pAIDB->GetAISmartObjectRecord(eSmartObject);
	if (pSmartObject)
	{
		LTStrCpy(szModelFilenameBuf, pSmartObject->strWorldEditModelName.c_str(), nModelFilenameBufLen);
		return LT_OK;
	}

	LTStrCpy(szModelFilenameBuf, "", nModelFilenameBufLen);
	return LT_OK;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodePatrol)

	ADD_DEDIT_COLOR( AINodePatrol )
	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use. Must be an Alarm or Switch object.")
	ADD_STRINGPROP_FLAG(Dependency,				"",				PF_HIDDEN|PF_OBJECTLINK, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(Radius,					512.0f,			PF_HIDDEN|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				PF_HIDDEN|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(ReactivationTime,			0.f,			PF_HIDDEN, "TODO:PROPDESC")

	// Do NOT hide the SmartObject template.

	ADD_STRINGPROP_FLAG(SmartObject,			"None", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

	// Add Patrol properties.

	ADD_STRINGPROP_FLAG(Next,	"",				0|PF_OBJECTLINK, "The name of the next node on the patrol path.")
	ADD_COMMANDPROP_FLAG(Command,"",			0|PF_NOTIFYCHANGE, "TODO:PROPDESC")
END_CLASS_FLAGS_PLUGIN(AINodePatrol, AINodeSmartObject, 0, AINodePatrolPlugin, "Patrolling AI follow a route defines by a chain of AINodePatrol nodes")

CMDMGR_BEGIN_REGISTER_CLASS(AINodePatrol)
CMDMGR_END_REGISTER_CLASS(AINodePatrol, AINodeSmartObject)

AINodePatrol::AINodePatrol()
{
	m_pNext = NULL;
	m_pPrev = NULL;
}

AINodePatrol::~AINodePatrol()
{
}

void AINodePatrol::InitNode()
{
	super::InitNode();

	HOBJECT hObject = NULL;
	if ( LT_OK == FindNamedObject(m_strNext.c_str(), hObject) && hObject )
	{	
		if ( IsKindOf(hObject, "AINodePatrol") )
		{
			m_strNext.clear();
			m_pNext = (AINodePatrol*)g_pLTServer->HandleToObject(hObject);

			if( m_pNext == this )
			{
				AIError("AINodePatrol: Node \"%s\" points to self.", GetNodeName() );
				m_pNext = NULL;
			}
			else {
				m_pNext->SetPrev(this);
			}
		}
		else 
		{
			AIError("AINodePatrol: Next node \"%s\" is not an AINodePatrol", m_strNext.c_str());
			m_strNext.clear();
		}
	}
	else if( !m_strNext.empty() )
	{
		AIError("AINodePatrol: Could not find Next node \"%s\"", m_strNext.c_str());
		m_strNext.clear();
	}
}

void AINodePatrol::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	const char* pszPropString = pProps->GetString( "Next", "" );
	if( pszPropString[0] )
	{
		m_strNext = pszPropString;
	}

	// Read patrol command.

	pszPropString = pProps->GetCommand( "Command", "" );
	if ( pszPropString[0] )
	{
		m_strPatrolCommand = pszPropString;
	}
}

void AINodePatrol::HandleAIArrival( CAI* pAI )
{
	super::HandleAIArrival( pAI );

	if( HasCmd() )
	{
		g_pCmdMgr->QueueCommand( GetCmd(), m_hObject, pAI->m_hObject );
	}
}

void AINodePatrol::SetPrev(AINodePatrol* pPrev)
{
	AIASSERT( !m_pPrev, m_hObject, "AINodePatrol::SetPrev: Multiple nodes point to same next." ); 
	m_pPrev = pPrev; 
}

void AINodePatrol::ClaimPatrolPath( CAI* pAI, bool bClaim )
{
	AINodePatrol* pNodePatrol = this;
	AINodePatrol* pNodeStart = this;

	typedef std::vector<AINodePatrol*, LTAllocator<AINodePatrol*, LT_MEM_TYPE_OBJECTSHELL> > TNodePatrolList;
	static TNodePatrolList vVisited;
	static TNodePatrolList::iterator it;
	vVisited.resize( 0 );

	bool bDone = false;

	// Lock all next nodes of NodeStart.

	while( pNodePatrol && !bDone )
	{
		vVisited.push_back( pNodePatrol);

		if( bClaim )
		{
			pNodePatrol->SetNodeOwner( pAI->m_hObject );
		}
		else {
			pNodePatrol->SetNodeOwner( NULL );
		}

		pNodePatrol = pNodePatrol->GetNext();

		// Check for loop.

		for(it = vVisited.begin(); it != vVisited.end(); ++it)
		{
			if( *it == pNodePatrol )
			{
				bDone = true;
				break;
			}
		}
	}

	// Lock all previous nodes of NodeStart.

	pNodePatrol = pNodeStart->GetPrev();
	while( pNodePatrol )
	{
		// Check for loop.

		for(it = vVisited.begin(); it != vVisited.end(); ++it)
		{
			if( *it == pNodePatrol )
			{
				return;
			}
		}

		vVisited.push_back( pNodePatrol);

		if( bClaim )
		{
			pNodePatrol->SetNodeOwner( pAI->m_hObject );
		}
		else {
			pNodePatrol->SetNodeOwner( NULL );
		}

		pNodePatrol = pNodePatrol->GetPrev();
	}
}

void AINodePatrol::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_COBJECT(m_pNext);
	SAVE_COBJECT(m_pPrev);
	SAVE_STDSTRING(m_strPatrolCommand);
}

void AINodePatrol::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_COBJECT(m_pNext, AINodePatrol);
	LOAD_COBJECT(m_pPrev, AINodePatrol);
	LOAD_STDSTRING(m_strPatrolCommand);
}

int AINodePatrol::DrawSelf()
{
	int nRet = super::DrawSelf();
	
	if (m_pNext)
	{
		// Use a separate line system from the base class, as the base class 
		// places a the nodes name at the nodes location.  On the client,
		// an AABB containing all of the lines is constructed, and the label
		// placed above it.  By adding this line to the base classes location,
		// the label may be placed in an unintended location.
		DebugLineSystem& system = LineSystem::GetSystem(this,"ShowPatrolNode");
		system.AddArrow(m_vPos, m_pNext->GetPos());
	}

	return nRet;
}

int AINodePatrol::HideSelf()
{
	 int nRet = super::HideSelf();

	DebugLineSystem& system = LineSystem::GetSystem(this,"ShowPatrolNode");
	system.Clear();

	return nRet;
}



// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINODEVEHICLE CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINODEVEHICLE 0

#endif

BEGIN_CLASS(AINodeVehicle)
	// Hide many of the SmartObject properties.

	ADD_REALPROP_FLAG(Radius,					512.0f,			PF_HIDDEN|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]")
	ADD_STRINGPROP_FLAG(Region,					"",				PF_HIDDEN|PF_OBJECTLINK, "Alternative to radius. The AI must be within this AIRegion to be able to use the node.")
	ADD_REALPROP_FLAG(ReactivationTime,			0.f,			PF_HIDDEN, "TODO:PROPDESC")

	// Do NOT hide the SmartObject template.

	ADD_STRINGPROP_FLAG(SmartObject,			"Motorcycle", 	0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")
	ADD_STRINGPROP_FLAG(VehicleKeyframeToRigidBody, "",			0|PF_OBJECTLINK, "The vehicle's keyframer to rigid body object.") \

END_CLASS_FLAGS_PLUGIN(AINodeVehicle, AINodeSmartObject, CF_HIDDEN_AINODEVEHICLE, AINodeVehiclePlugin, "AI will mount a vehicle from an AINodeVehicle after receiving a MOUNT command")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeVehicle)
CMDMGR_END_REGISTER_CLASS(AINodeVehicle, AINodeSmartObject)

AINodeVehicle::AINodeVehicle()
{
	m_hVehicleKeyframeToRigidBody = NULL;
}

AINodeVehicle::~AINodeVehicle()
{
}

void AINodeVehicle::InitNode()
{
	super::InitNode();

	HOBJECT hVehicleKeyframeToRigidBody;
	if( !m_strVehicleKeyframeToRigidBody.empty() )
	{
		if( LT_OK == FindNamedObject( m_strVehicleKeyframeToRigidBody.c_str(), hVehicleKeyframeToRigidBody, false ) )
		{
			m_hVehicleKeyframeToRigidBody = hVehicleKeyframeToRigidBody;
		}
	}
}

void AINodeVehicle::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	const char* pszPropString = pProps->GetString( "VehicleKeyframeToRigidBody", "" );
	if( pszPropString[0] )
	{
		m_strVehicleKeyframeToRigidBody = pszPropString;
	}
}

void AINodeVehicle::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_HOBJECT( m_hVehicleKeyframeToRigidBody );
	SAVE_STDSTRING( m_strVehicleKeyframeToRigidBody );
}

void AINodeVehicle::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_HOBJECT( m_hVehicleKeyframeToRigidBody );
	LOAD_STDSTRING( m_strVehicleKeyframeToRigidBody );
}


//---------------

// Simple object node defining macro.  Many AINodeSmartObject derived classes
// have behavior defined entirely by AINodeSmartObject.  They only different 
// in the list of SmartObject options presented in WorldEdit. As WorldEdit does not
// support dependency between fields, we must define a node and plugin per
// SmartObject type.  To avoid code duplication and other nastiness, this 
// macro can be used.

#define SIMPLE_SMARTOBJECT_NODE(node_name)							\
																	\
class AINode##node_name##Plugin : public AINodeSmartObjectPlugin	\
{																	\
public:																\
	AINode##node_name##Plugin()										\
	{																\
		AddValidNodeType(kNode_##node_name);						\
	}																\
};																	\
																	\
class AINode##node_name : public AINodeSmartObject				\
{																	\
public:																\
	EnumAINodeType GetType() const { return kNode_##node_name; }	\
																	\
};																	\
																	\
																	\
BEGIN_CLASS(AINode##node_name)									\
	ADD_STRINGPROP_FLAG(SmartObject,			"None", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC") \
	ADD_REALPROP_FLAG(Radius,					0.0f,			0|PF_RADIUS, "The AI must be within this radius to be able to use the node. [WorldEdit units]") \
END_CLASS_FLAGS_PLUGIN(AINode##node_name, AINodeSmartObject, 0, AINode##node_name##Plugin, "AI will behave according to the " #node_name "smart object") \
																	\
CMDMGR_BEGIN_REGISTER_CLASS(AINode##node_name)					\
CMDMGR_END_REGISTER_CLASS(AINode##node_name, AINodeSmartObject)


SIMPLE_SMARTOBJECT_NODE(MenacePlace)
SIMPLE_SMARTOBJECT_NODE(Flipable)
SIMPLE_SMARTOBJECT_NODE(WorkItem)
