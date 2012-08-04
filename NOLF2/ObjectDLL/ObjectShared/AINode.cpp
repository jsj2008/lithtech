// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AITypes.h"
#include "AINode.h"
#include "ServerUtilities.h"
#include "AIUtils.h"
#include "AIButeMgr.h"
#include "AIHuman.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "ObjectMsgs.h"
#include "DEditColors.h"
#include "AnimationPropStrings.h"
#include "AIGoalButeMgr.h"
#include "ParsedMsg.h"
#include "AnimationMgr.h"
#include "AIState.h"
#include "Weapon.h"
#include "Attachments.h"
#include "RelationButeMgr.h"
#include "RelationMgr.h"

extern CVarTrack			g_ShowNodesTrack;


LINKFROM_MODULE( AINode );

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINode)

	ADD_VECTORPROP_VAL_FLAG(Dims,		16.0f, 16.0f, 16.0f,	PF_HIDDEN | PF_DIMS)
	ADD_BOOLPROP_FLAG(Face,				LTTRUE,					0)
	ADD_STRINGPROP_FLAG(Alignment,		"None", 				PF_STATICLIST)
	ADD_BOOLPROP_FLAG(StartDisabled,	LTFALSE,				0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AINode, GameBase, NULL, NULL, 0, AINodePlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINode)

	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )
	CMDMGR_ADD_MSG( ENABLE, 1, NULL, "ENABLE" )
	CMDMGR_ADD_MSG( DISABLE, 1, NULL, "DISABLE" )

CMDMGR_END_REGISTER_CLASS(AINode, GameBase)



AINode::AINode()
{
	m_vPos = LTVector(0,0,0);
	m_vUp = LTVector(0,0,0);
	m_vForward = LTVector(0,0,0);
	m_vRight = LTVector(0,0,0);
	m_vInitialPitchYawRoll = LTVector(0,0,0);

	m_bFaceNodeForward = LTTRUE;

	m_fNodeReactivationTime = 0.f;
	m_fNodeNextActivationTime = 0.f;
	m_fNodeLastActivationTime = 0.f;

	m_fRadius = (LTFLOAT)INT_MAX;
	m_fRadiusSqr = (LTFLOAT)INT_MAX;

	m_hstrName = LTNULL;

	m_bContainingVolumeValid = false;
	m_pContainingVolume = LTNULL;

	m_nRequiredRelationTemplateID = -1;

	m_hNodeOwner = LTNULL;

	m_nNodeLockCount = 0;
	m_bNodeEnabled = LTTRUE;
}

AINode::~AINode()
{
	FREE_HSTRING(m_hstrName);
}

void AINode::Init()
{
	// Jump out if we've already done this
	/*
	if ( m_bContainingVolumeValid )
	{
		return;
	}
	*/

	// Find volume containing node.

	LTFLOAT fSearchY = 64.f;
	m_pContainingVolume = g_pAIVolumeMgr->FindContainingVolume( LTNULL, m_vPos, eAxisAll, fSearchY, LTNULL );
	if ( !m_pContainingVolume )
	{
		AIASSERT( 0, m_hObject, "AINode::Init: Could not find containing volume." );
	}

	m_bContainingVolumeValid = true;
}

uint32 AINode::EngineMessageFn(uint32 messageID, void *pv, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pv, fData);

			if ( (int)fData == PRECREATE_WORLDFILE || (int)fData == PRECREATE_STRINGPROP )
			{
				ReadProp((ObjectCreateStruct*)pv);

				if ( pv )
				{
					ObjectCreateStruct* pocs = (ObjectCreateStruct*)pv;
					pocs->m_Flags = FLAG_FORCEOPTIMIZEOBJECT;
				}
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
			SetNextUpdate(0.0f);
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pv, fData);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::OnTrigger
//
//  PURPOSE:	Handle recieving a trigger msg from another object
//
// ----------------------------------------------------------------------- //

bool AINode::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_Lock("LOCK");
	static CParsedMsg::CToken s_cTok_Unlock("UNLOCK");
	static CParsedMsg::CToken s_cTok_Enable("ENABLE");
	static CParsedMsg::CToken s_cTok_Disable("DISABLE");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_nNodeLockCount++;
	}
	else if( cMsg.GetArg(0) == s_cTok_Unlock )
	{
		m_nNodeLockCount--;
	}
	else if( cMsg.GetArg(0) == s_cTok_Enable )
	{
		m_bNodeEnabled = LTTRUE;

		AITRACE( AIShowNodes, ( m_hObject, "Enabling node" ) );

		//if we're drawing debug stuff, update
		if (g_ShowNodesTrack.GetFloat())
		{
			HideSelf();
			DrawSelf();
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Disable )
	{
		m_bNodeEnabled = LTFALSE;

		AITRACE( AIShowNodes, ( m_hObject, "Disabling node" ) );

		//if we're drawing debug stuff, update
		if (g_ShowNodesTrack.GetFloat())
		{
			HideSelf();
			DrawSelf();
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Remove )
	{
		AIError( "Attempting to remove AINode \"%s\"! Disabling instead.", ::ToString( GetName() ) );
		m_bNodeEnabled = LTFALSE;
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
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
	// of UseObject nodes, only useable by the owner of the guard node).

	AIASSERT( ( !m_hNodeOwner ) || ( !hOwner ) || ( m_hNodeOwner == hOwner ), m_hObject, "AINode::SetNodeOwner: Node already has an owner." );
	m_hNodeOwner = hOwner;

	AITRACE( AIShowNodes, ( m_hObject, "Setting owner: %s", GetObjectName( m_hNodeOwner ) ) );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINode::Lock / Unlock / IsLocked
//
//  PURPOSE:	Lock nodes, reset reactivation time.
//
// ----------------------------------------------------------------------- //

void AINode::Lock(HOBJECT hAI)
{
	AITRACE( AIShowNodes, ( hAI, "Locking node %s (%d)\n", ::ToString( GetName() ), m_nNodeLockCount ) );
	++m_nNodeLockCount; 
}

void AINode::Unlock(HOBJECT hAI)
{
	AITRACE( AIShowNodes, ( hAI, "Unlocking node %s (%d)\n", ::ToString( GetName() ), m_nNodeLockCount ) );
	AIASSERT(m_nNodeLockCount > 0, m_hObject, "AINode::Unlock: Node is not locked.");
	--m_nNodeLockCount;
}

LTBOOL AINode::IsLocked() const
{
	return (m_nNodeLockCount > 0);
}

void AINode::ResetActivationTime()
{
	ResetActivationTime( m_fNodeReactivationTime );
}

void AINode::ResetActivationTime(LTFLOAT fResetTime)
{
	m_fNodeLastActivationTime = g_pLTServer->GetTime();

	// Set next valid activation time.

	m_fNodeNextActivationTime = m_fNodeLastActivationTime + fResetTime;
}

LTBOOL AINode::IsTimedOut() const
{
	return ( m_fNodeNextActivationTime > g_pLTServer->GetTime() ); 
}

void AINode::ReadProp(ObjectCreateStruct* pocs)
{
    if ( (g_pLTServer->GetPropGeneric( "Pos", &g_gp ) == LT_OK) )
		if ( g_gp.m_String[0] )
			sscanf(g_gp.m_String, "%f %f %f", &m_vPos.x, &m_vPos.y, &m_vPos.z);
	
    if ( g_pLTServer->GetPropGeneric( "Name", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrName = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "Radius", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_fRadius = g_gp.m_Float;
			m_fRadiusSqr = m_fRadius * m_fRadius;
		}
	}

    if ( g_pLTServer->GetPropGeneric( "Alignment", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] && stricmp( g_gp.m_String, "None" ) )
		{
			m_nRequiredRelationTemplateID = CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->GetObjectRelationMgrTemplateIDByName( g_gp.m_String );
		}
	}

    if ( g_pLTServer->GetPropGeneric( "StartDisabled", &g_gp ) == LT_OK )
	{
		m_bNodeEnabled = (g_gp.m_Bool ? LTFALSE : LTTRUE);
	}

	READPROP_BOOL("Face", m_bFaceNodeForward);

	LTVector vAngles;
    if ( g_pLTServer->GetPropRotationEuler( "Rotation", &vAngles ) == LT_OK )
		m_vInitialPitchYawRoll = vAngles;

	LTRotation rRot(VEC_EXPAND(m_vInitialPitchYawRoll));
	m_vRight = rRot.Right();
	m_vUp = rRot.Up();
	m_vForward = rRot.Forward();

	// Add Node to the NodeMgr.
	AIASSERT(g_pAINodeMgr, m_hObject, "AINode::ReadProp: NodeMgr is NULL.");
	g_pAINodeMgr->AddNode( GetType(), this);
}

void AINode::Save(ILTMessage_Write *pMsg)
{
	SAVE_VECTOR(m_vPos);
	SAVE_VECTOR(m_vUp);
	SAVE_VECTOR(m_vForward);
	SAVE_VECTOR(m_vRight);
	SAVE_VECTOR(m_vInitialPitchYawRoll);
	SAVE_HSTRING(m_hstrName);
	SAVE_INT(m_nNodeLockCount);
	SAVE_BOOL(m_bNodeEnabled);
	SAVE_FLOAT(m_fRadius);
	SAVE_FLOAT(m_fRadiusSqr);
	SAVE_BOOL(m_bFaceNodeForward);
	SAVE_TIME(m_fNodeNextActivationTime);
	SAVE_FLOAT(m_fNodeReactivationTime);
	SAVE_TIME(m_fNodeLastActivationTime);
	SAVE_HOBJECT(m_hNodeOwner);
	SAVE_COBJECT(m_pContainingVolume);
	SAVE_DWORD(m_nRequiredRelationTemplateID);
}

void AINode::Load(ILTMessage_Read *pMsg)
{
	LOAD_VECTOR(m_vPos);
	LOAD_VECTOR(m_vUp);
	LOAD_VECTOR(m_vForward);
	LOAD_VECTOR(m_vRight);
	LOAD_VECTOR(m_vInitialPitchYawRoll);
	LOAD_HSTRING(m_hstrName);
	LOAD_INT(m_nNodeLockCount);
	LOAD_BOOL(m_bNodeEnabled);
	LOAD_FLOAT(m_fRadius);
	LOAD_FLOAT(m_fRadiusSqr);
	LOAD_BOOL(m_bFaceNodeForward);
	LOAD_TIME(m_fNodeNextActivationTime);
	LOAD_FLOAT(m_fNodeReactivationTime);
	LOAD_TIME(m_fNodeLastActivationTime);
	LOAD_HOBJECT(m_hNodeOwner);
	LOAD_COBJECT(m_pContainingVolume, AIVolume);
	LOAD_DWORD(m_nRequiredRelationTemplateID);
}

void AINode::Verify()
{
	if ( NULL == g_pAIVolumeMgr->FindContainingVolume(LTNULL, m_vPos, eAxisAll, 106.0f) )
	{
		Warn("AINode \"%s\" is not in an AIVolume!", g_pLTServer->GetStringData(m_hstrName));
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
  	system << LineSystem::Arrow( vNodePos, vNodePos + rRot.Forward() * 16.0f, Color::Blue, 255);
  	system << LineSystem::Arrow( vNodePos, vNodePos + rRot.Up() * 16.0f, Color::Green, 255);
  	system << LineSystem::Arrow( vNodePos, vNodePos + rRot.Right() * 16.0f, Color::Red, 255);
  
   	system	<< LineSystem::Box(vNodePos, LTVector(16.0f, 16.0f, 16.0f), GetDebugColor(), 126);
     									
  	char szObjectName[256];
  	g_pLTServer->GetObjectName(m_hObject, szObjectName, 256);
  	system.SetDebugString(szObjectName);
  
   	return 0;
}
   
int AINode::HideSelf()
{
  	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowNode");
  	system.SetDebugString("");
  	system.Clear( );
  
   	return 0;
}

extern CRelationButeMgr s_RelationButeMgr;

LTRESULT AINodePlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !s_RelationButeMgr.IsInitialized() )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\RelationData.txt", szRezPath);
		s_RelationButeMgr.SetInRezFile(LTFALSE);
	    s_RelationButeMgr.Init(szFile);
	}
		
	if ( !_strcmpi("Alignment", szPropName) )
	{
		strcpy( aszStrings[(*pcStrings)++], "None" );

		CRelationButeMgr::_mapNameToID* pNames = s_RelationButeMgr.GetMapObjectRelationMgrNameToID();
		CRelationButeMgr::_mapNameToID::iterator it;

		for( it = pNames->begin(); it != pNames->end(); ++it )
		{
			strcpy( aszStrings[(*pcStrings)++], it->first.c_str() );
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT AINodePlugin::PreHook_PropChanged( const char *szObjName, const char *szPropName, const int nPropType, const GenericProp &gpPropValue, ILTPreInterface *pInterface, const char *szModifiers )
{
	if( !_stricmp( szPropName, "Command" ) && gpPropValue.m_String[0] )
	{
		ConParse cpCmd;
		cpCmd.Init( gpPropValue.m_String );

		while( LT_OK == pInterface->Parse( &cpCmd ))
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

				char szPropVal[MAX_GP_STRING_LEN] = {0};
				LTSNPrintF( szPropVal, ARRAY_LEN( szPropVal ), "msg <CAIHuman> (%s)", sCmd.c_str() );

				GenericProp gp;
				LTStrCpy( gp.m_String, szPropVal, ARRAY_LEN( gp.m_String ));

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

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeDeath)
	ADD_DEDIT_COLOR( AINodeCover )
END_CLASS_DEFAULT(AINodeDeath, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeDeath)
CMDMGR_END_REGISTER_CLASS(AINodeDeath, AINode)

// ----------------------------------------------------------------------- //


BEGIN_CLASS(AINodeGoto)
	ADD_STRINGPROP_FLAG(Command,				"",				0|PF_NOTIFYCHANGE)
END_CLASS_DEFAULT(AINodeGoto, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeGoto)
CMDMGR_END_REGISTER_CLASS(AINodeGoto, AINode)


AINodeGoto::AINodeGoto()
{
	m_hstrGotoCmd = LTNULL;
}

AINodeGoto::~AINodeGoto()
{
	FREE_HSTRING( m_hstrGotoCmd );
}

void AINodeGoto::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

	// Read goto command.

	if ( g_pLTServer->GetPropGeneric( "Command", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrGotoCmd = g_pLTServer->CreateString( g_gp.m_String );
		}
	}
}

void AINodeGoto::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_HSTRING(m_hstrGotoCmd);
}

void AINodeGoto::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_HSTRING(m_hstrGotoCmd);
}


// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeCover)

	ADD_DEDIT_COLOR( AINodeCover )

	ADD_BOOLPROP_FLAG(Duck,						LTFALSE,		0)
	ADD_BOOLPROP_FLAG(BlindFire,				LTFALSE,		0)
	ADD_BOOLPROP_FLAG(1WayRoll,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(2WayRoll,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(1WayStep,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(2WayStep,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(IgnoreDir,				LTFALSE,		0)
	ADD_REALPROP_FLAG(Fov,						60.0f,			0|PF_FIELDOFVIEW)
	ADD_REALPROP_FLAG(Radius,					384.0f,			0|PF_RADIUS)
	ADD_REALPROP_FLAG(ThreatRadius,				256.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(Object,					"",				0|PF_OBJECTLINK)
	ADD_REALPROP_FLAG(Timeout,					5.0f,			0)
	ADD_REALPROP_FLAG(HitpointsBoost,			2.0f,			0)

END_CLASS_DEFAULT(AINodeCover, AINodeChangeWeapons, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeCover)
CMDMGR_END_REGISTER_CLASS(AINodeCover, AINodeChangeWeapons)

AINodeCover::AINodeCover()
{
	m_bIgnoreDir = LTFALSE;
	m_fFovDp = 0.0f;
	m_dwFlags = 0x0;
	m_hstrObject = LTNULL;
	m_fRadiusSqr = 0.0f;
	m_fThreatRadiusSqr = 0.0f;
	m_fTimeout = 999999.0f;
	m_fHitpointsBoost = 2.0f;
}

AINodeCover::~AINodeCover()
{
	FREE_HSTRING(m_hstrObject);
}

void AINodeCover::ClearObject()
{
	FREE_HSTRING(m_hstrObject);
}

void AINodeCover::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

    if ( g_pLTServer->GetPropGeneric( "Duck", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagDuck : 0x0;

    if ( g_pLTServer->GetPropGeneric( "BlindFire", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagBlind : 0x0;

    if ( g_pLTServer->GetPropGeneric( "1WayRoll", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlag1WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "2WayRoll", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlag2WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "1WayStep", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlag1WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "2WayStep", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlag2WayCorner : 0x0;

    if ( g_pLTServer->GetPropGeneric( "IgnoreDir", &g_gp ) == LT_OK )
		m_bIgnoreDir = g_gp.m_Bool;

    if ( g_pLTServer->GetPropGeneric( "Fov", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fFovDp = FOV2DP(g_gp.m_Float);

    if ( g_pLTServer->GetPropGeneric( "ThreatRadius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fThreatRadiusSqr = g_gp.m_Float*g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "Object", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrObject = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "Timeout", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fTimeout = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "HitpointsBoost", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fHitpointsBoost = g_gp.m_Float;

	// Clean up pitch/yaw/roll if we're two way cover

	LTRotation rRot(EXPANDVEC(m_vInitialPitchYawRoll));

	if ( m_dwFlags & kFlag2WayCorner )
	{
		LTVector vForward = rRot.Forward();

		LTFLOAT fPitch, fYaw, fRoll;

		fPitch = 0.0f;
		fYaw = (LTFLOAT)acos(vForward.z);
		fRoll = 0.0f;

		rRot = LTRotation(fPitch, fYaw, fRoll);
	}

	m_vRight = rRot.Right();
	m_vUp = rRot.Up();
	m_vForward = rRot.Forward();
}

void AINodeCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bIgnoreDir);
	SAVE_FLOAT(m_fFovDp);
	SAVE_DWORD(m_dwFlags);
	SAVE_HSTRING(m_hstrObject);
	SAVE_FLOAT(m_fThreatRadiusSqr);
	SAVE_FLOAT(m_fTimeout);
	SAVE_FLOAT(m_fHitpointsBoost);
}

void AINodeCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bIgnoreDir);
	LOAD_FLOAT(m_fFovDp);
	LOAD_DWORD(m_dwFlags);
	LOAD_HSTRING(m_hstrObject);
	LOAD_FLOAT(m_fThreatRadiusSqr);
	LOAD_FLOAT(m_fTimeout);
	LOAD_FLOAT(m_fHitpointsBoost);
}

void AINodeCover::Verify() 
{
	super::Verify();

	if ( m_dwFlags & kFlag1WayCorner && m_dwFlags & kFlag2WayCorner )
	{
		// We can't be two way AND one way cover!

		Warn("AINode %s has 2way and 1way cover specified!", g_pLTServer->GetStringData(m_hstrName));
		m_dwFlags = 0;
	}
}

EnumNodeStatus AINodeCover::GetStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	LTVector vThreatPos;
    g_pLTServer->GetObjectPos(hThreat, &vThreatPos);

	// Check if the threat is too close to the AI,
	// and is blocking the path to the node.

	if ( VEC_DISTSQR(vPos, vThreatPos) < g_pAIButeMgr->GetSenses()->fThreatTooCloseDistanceSqr )
	{
		LTVector vToThreat = vThreatPos - vPos;
		LTVector vToNode = m_vPos - vPos;
	
		if( vToThreat.Dot( vToNode ) > 0.f )
		{
			return kStatus_ThreatBlockingPath;
		}
	}

	if ( VEC_DISTSQR(m_vPos, vThreatPos) < m_fThreatRadiusSqr )
	{
		return kStatus_ThreatInsideRadius;
	}

	if ( IsIgnoreDir() ) return kStatus_Ok;

	LTVector vThreatDir = vThreatPos - m_vPos;
	vThreatDir.Normalize();

	if ( vThreatDir.Dot(m_vForward) > m_fFovDp )
	{
		return kStatus_Ok;
	}
	else
	{
		return kStatus_ThreatOutsideFOV;
	}
}


// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeView)

	ADD_DEDIT_COLOR( AINodeView )

	ADD_REALPROP_FLAG(Radius,					512.0f,			0|PF_RADIUS)

END_CLASS_DEFAULT(AINodeView, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeView)
CMDMGR_END_REGISTER_CLASS(AINodeView, AINode)

AINodeView::AINodeView()
{
	m_fRadiusSqr = 0.0f;
}

AINodeView::~AINodeView()
{
}

void AINodeView::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);
}

void AINodeView::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void AINodeView::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void AINodeView::Verify() 
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeVantage)

	ADD_DEDIT_COLOR( AINodeVantage )

	ADD_BOOLPROP_FLAG(IgnoreDir,				LTFALSE,		0)
	ADD_REALPROP_FLAG(Fov,						90.0f,			0|PF_FIELDOFVIEW)
	ADD_REALPROP_FLAG(Radius,					512.0f,			0|PF_RADIUS)
	ADD_REALPROP_FLAG(ThreatRadius,				256.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(Object,					"",				0|PF_OBJECTLINK)

END_CLASS_DEFAULT(AINodeVantage, AINodeChangeWeapons, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeVantage)
CMDMGR_END_REGISTER_CLASS(AINodeVantage, AINodeChangeWeapons)

BEGIN_CLASS(AINodeVantageRoof)
END_CLASS_DEFAULT(AINodeVantageRoof, AINodeVantage, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeVantageRoof)
CMDMGR_END_REGISTER_CLASS(AINodeVantageRoof, AINodeVantage)

AINodeVantage::AINodeVantage()
{
	m_bIgnoreDir = LTFALSE;
	m_fFovDp = 0.0f;
	m_fRadiusSqr = 0.0f;
	m_fThreatRadiusSqr = 0.0f;
	m_hstrObject = LTNULL;
}

AINodeVantage::~AINodeVantage()
{
	FREE_HSTRING( m_hstrObject );
}

void AINodeVantage::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

    if ( g_pLTServer->GetPropGeneric( "IgnoreDir", &g_gp ) == LT_OK )
		m_bIgnoreDir = g_gp.m_Bool;

    if ( g_pLTServer->GetPropGeneric( "Fov", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fFovDp = FOV2DP(g_gp.m_Float);

    if ( g_pLTServer->GetPropGeneric( "ThreatRadius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fThreatRadiusSqr = g_gp.m_Float*g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "Object", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrObject = g_pLTServer->CreateString( g_gp.m_String );
}

void AINodeVantage::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bIgnoreDir);
	SAVE_FLOAT(m_fFovDp);
	SAVE_FLOAT(m_fThreatRadiusSqr);
	SAVE_HSTRING(m_hstrObject);
}

void AINodeVantage::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bIgnoreDir);
	LOAD_FLOAT(m_fFovDp);
	LOAD_FLOAT(m_fThreatRadiusSqr);
	LOAD_HSTRING(m_hstrObject);
}

EnumNodeStatus AINodeVantage::GetStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	LTVector vThreatPos;
    g_pLTServer->GetObjectPos(hThreat, &vThreatPos);

	// Check if the threat is too close to the AI,
	// and is blocking the path to the node.

	if ( VEC_DISTSQR(vPos, vThreatPos) < g_pAIButeMgr->GetSenses()->fThreatTooCloseDistanceSqr )
	{
		LTVector vToThreat = vThreatPos - vPos;
		LTVector vToNode = m_vPos - vPos;
	
		if( vToThreat.Dot( vToNode ) > 0.f )
		{
			return kStatus_ThreatBlockingPath;
		}
	}

	if ( VEC_DISTSQR(m_vPos, vThreatPos) < m_fThreatRadiusSqr )
	{
		return kStatus_ThreatInsideRadius;
	}

	if ( IsIgnoreDir() ) return kStatus_Ok;

	LTVector vThreatDir = vThreatPos - m_vPos;
	vThreatDir.Normalize();

	if ( vThreatDir.Dot(m_vForward) > m_fFovDp )
	{
		return kStatus_Ok;
	}
	else
	{
		return kStatus_ThreatOutsideFOV;
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeSearch)

	ADD_DEDIT_COLOR( AINodeSearch )

    ADD_REALPROP_FLAG(ResetTime,				30.f,			0)
	ADD_BOOLPROP_FLAG(ShineFlashlight,			LTFALSE,		0)
	ADD_BOOLPROP_FLAG(LookUnder,				LTFALSE,		0)
	ADD_BOOLPROP_FLAG(LookOver,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(LookUp,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(LookLeft,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(LookRight,				LTFALSE,		0)
	ADD_BOOLPROP_FLAG(KnockOnDoor,				LTFALSE,		0)
	ADD_BOOLPROP_FLAG(Alert,					LTFALSE,		0)
	ADD_STRINGPROP_FLAG(SearchType,				"Default", 		0|PF_STATICLIST)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodeSearch, AINode, NULL, NULL, 0, AINodeSearchPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeSearch)
CMDMGR_END_REGISTER_CLASS(AINodeSearch, AINode)

AINodeSearch::AINodeSearch()
{
	m_dwFlags = 0x0;
	m_bSearched = LTFALSE;
	m_eSearchType = kSearch_Default;
}

AINodeSearch::~AINodeSearch()
{
}

void AINodeSearch::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

	if ( g_pLTServer->GetPropGeneric( "ResetTime", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fNodeReactivationTime = g_gp.m_Float;

	if ( g_pLTServer->GetPropGeneric( "SearchType", &g_gp ) == LT_OK )
	{
		if( stricmp( g_gp.m_String, "OneWay" ) == 0 )
		{
			m_eSearchType = kSearch_OneWay;	
		}
		else if( stricmp( g_gp.m_String, "Corner" ) == 0 )
		{
			m_eSearchType = kSearch_Corner;	
		}
	}

    if ( g_pLTServer->GetPropGeneric( "ShineFlashlight", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagShineFlashlight : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookUnder", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagLookUnder : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookOver", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagLookOver : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookUp", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagLookUp : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookLeft", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagLookLeft : 0x0;

    if ( g_pLTServer->GetPropGeneric( "LookRight", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagLookRight : 0x0;

    if ( g_pLTServer->GetPropGeneric( "KnockOnDoor", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagKnockOnDoor : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Alert", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagAlert : 0x0;
}

void AINodeSearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_dwFlags);
	SAVE_BOOL(m_bSearched);
	SAVE_DWORD(m_eSearchType);
}

void AINodeSearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD(m_dwFlags);
	LOAD_BOOL(m_bSearched);
	LOAD_DWORD_CAST(m_eSearchType, EnumSearchType);
}

void AINodeSearch::Search()
{
	_ASSERT(!m_bSearched);
	m_bSearched = LTTRUE;
}

EnumNodeStatus AINodeSearch::GetStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	if ( m_bSearched )
	{
		return kStatus_SearchedRecently;
	}

	return kStatus_Ok;
}

LTRESULT AINodeSearchPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !_strcmpi("SearchType", szPropName) )
	{
		strcpy(aszStrings[(*pcStrings)++], "Default");
		strcpy(aszStrings[(*pcStrings)++], "OneWay");
		strcpy(aszStrings[(*pcStrings)++], "Corner");

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodePanic)

	ADD_BOOLPROP_FLAG(Stand,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(Crouch,					LTFALSE,		0)
	ADD_REALPROP_FLAG(Radius,					512.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(Object,					"",				0|PF_OBJECTLINK)

END_CLASS_DEFAULT(AINodePanic, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodePanic)
CMDMGR_END_REGISTER_CLASS(AINodePanic, AINode)

AINodePanic::AINodePanic()
{
	m_dwFlags = 0x0;
	m_hstrObject = LTNULL;
	m_fRadiusSqr = 0.0f;
}

AINodePanic::~AINodePanic()
{
	FREE_HSTRING(m_hstrObject);
}

void AINodePanic::ClearObject()
{
	FREE_HSTRING(m_hstrObject);
}

void AINodePanic::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

    if ( g_pLTServer->GetPropGeneric( "Stand", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagStand : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Crouch", &g_gp ) == LT_OK )
		m_dwFlags |= g_gp.m_Bool ? kFlagCrouch : 0x0;

    if ( g_pLTServer->GetPropGeneric( "Object", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrObject = g_pLTServer->CreateString( g_gp.m_String );

	// Force infinite radius.

	m_fRadius = FLT_MAX;
	m_fRadiusSqr = FLT_MAX;
}

void AINodePanic::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_dwFlags);
	SAVE_HSTRING(m_hstrObject);
}

void AINodePanic::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD(m_dwFlags);
	LOAD_HSTRING(m_hstrObject);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeAssassinate)

	ADD_REALPROP_FLAG(Radius,					512.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(Movement,				"Walk", 		0|PF_STATICLIST)
	ADD_BOOLPROP_FLAG(IgnoreVisibility,			LTTRUE,			0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodeAssassinate, AINode, NULL, NULL, 0, AINodeAssassinatePlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeAssassinate)
CMDMGR_END_REGISTER_CLASS(AINodeAssassinate, AINode)


AINodeAssassinate::AINodeAssassinate()
{
	m_fRadiusSqr		= 0.0f;
	m_eMovement			= kAP_Walk;
	m_bIgnoreVisibility	= LTTRUE;
}

AINodeAssassinate::~AINodeAssassinate()
{
}

void AINodeAssassinate::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

	// Read movement to use to get to the object.
	if ( g_pLTServer->GetPropGeneric( "Movement", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_eMovement = CAnimationMgrList::GetPropFromName( g_gp.m_String );
		}
	}
	
	// Read ignoreVisibility flag.
	READPROP_BOOL("IgnoreVisibility", m_bIgnoreVisibility);
}

void AINodeAssassinate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_eMovement);
	SAVE_BOOL(m_bIgnoreVisibility);
}

void AINodeAssassinate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
	LOAD_BOOL(m_bIgnoreVisibility);
}

LTRESULT AINodeAssassinatePlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !_strcmpi("Movement", szPropName) )
	{
		strcpy(aszStrings[(*pcStrings)++], "Walk");
		strcpy(aszStrings[(*pcStrings)++], "Run");

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeUseObject)

	ADD_STRINGPROP_FLAG(Object,					"",				0|PF_OBJECTLINK)
	ADD_REALPROP_FLAG(Radius,					512.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(Movement,				"Walk", 		0|PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SmartObject,			"LightSwitch", 	0|PF_STATICLIST)
    ADD_STRINGPROP_FLAG(PreActivateCommand,		"",				0)
    ADD_STRINGPROP_FLAG(PostActivateCommand,	"",				0)
    ADD_REALPROP_FLAG(ReactivationTime,			0.f,			0)
	ADD_STRINGPROP_FLAG(FirstSound,				"None", 		0|PF_STATICLIST)
	ADD_STRINGPROP_FLAG(FidgetSound,			"None", 		0|PF_STATICLIST)
	ADD_BOOLPROP_FLAG(OneWay,					LTFALSE,		0)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodeUseObject, AINode, NULL, NULL, 0, AINodeUseObjectPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeUseObject)
CMDMGR_END_REGISTER_CLASS(AINodeUseObject, AINode)

AINodeUseObject::AINodeUseObject()
{
	m_hstrObject	= LTNULL;
	m_hUseObject	= LTNULL;
	m_hstrPreActivateCommand = LTNULL;
	m_hstrPostActivateCommand = LTNULL;
	m_fRadiusSqr	= 0.0f;
	m_eMovement		= kAP_Walk;
	m_eFirstSound	= kAIS_None;
	m_eFidgetSound	= kAIS_None;
	m_eSmartObjectState = kState_SmartObjectDefault;
	m_nSmartObjectID = -1;
	m_bOneWay = LTFALSE;
}

AINodeUseObject::~AINodeUseObject()
{
	FREE_HSTRING(m_hstrObject);
	FREE_HSTRING(m_hstrPreActivateCommand);
	FREE_HSTRING(m_hstrPostActivateCommand);
}

LTBOOL AINodeUseObject::NodeTypeIsActive(EnumAINodeType eNodeType)
{
	// The actual node type is always true.

	if( eNodeType == GetType() )
	{
		return LTTRUE;
	}

	AIGBM_SmartObjectTemplate *pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(m_nSmartObjectID);

	// Find the active node types for the current smartobject state.

	SMART_OBJECT_ACTIVE_CMD_MAP::iterator it;
	for( it = pSmartObject->mapActiveCmds.lower_bound( m_eSmartObjectState ); it != pSmartObject->mapActiveCmds.upper_bound( m_eSmartObjectState ); ++it )
	{
		if( it->second == eNodeType )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

void AINodeUseObject::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

    if ( g_pLTServer->GetPropGeneric( "Object", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrObject = g_pLTServer->CreateString( g_gp.m_String );

	if ( g_pLTServer->GetPropGeneric( "ReactivationTime", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fNodeReactivationTime = g_gp.m_Float;

	// Read movement to use to get to the object.
	if ( g_pLTServer->GetPropGeneric( "Movement", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_eMovement = CAnimationMgrList::GetPropFromName( g_gp.m_String );
		}
	}

	READPROP_BOOL("OneWay", m_bOneWay);

	// Read first sound.
	if ( g_pLTServer->GetPropGeneric( "FirstSound", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			uint32 iSound;
			for(iSound=0; iSound < kAIS_Count; ++iSound)
			{
				if( stricmp(s_aszAISoundTypes[iSound], g_gp.m_String) == 0 )
				{
					m_eFirstSound = (EnumAISoundType)iSound;
					break;
				}
			}
			AIASSERT(iSound < kAIS_Count, m_hObject, "AINodeUseObject::ReadProp: Unrecognized AISound type.");
		}
	}

	// Read fidget sound.
	if ( g_pLTServer->GetPropGeneric( "FidgetSound", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			uint32 iSound;
			for(iSound=0; iSound < kAIS_Count; ++iSound)
			{
				if( stricmp(s_aszAISoundTypes[iSound], g_gp.m_String) == 0 )
				{
					m_eFidgetSound = (EnumAISoundType)iSound;
					break;
				}
			}
			AIASSERT(iSound < kAIS_Count, m_hObject, "AINodeUseObject::ReadProp: Unrecognized AISound type.");
		}
	}

	// Read activate commands.

	if ( g_pLTServer->GetPropGeneric( "PreActivateCommand", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrPreActivateCommand = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

	if ( g_pLTServer->GetPropGeneric( "PostActivateCommand", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrPostActivateCommand = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

	// Read SmartObject type.

	AIASSERT(g_pAINodeMgr, m_hObject, "AINodeUseObject::ReadProp: NodeMgr is NULL.");

	AIGBM_SmartObjectTemplate* pSmartObject = LTNULL;
	if ( g_pLTServer->GetPropGeneric( "SmartObject", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(g_gp.m_String);
			AIASSERT(pSmartObject != LTNULL, m_hObject, "AINodeUseObject::ReadProp: SmartObject is NULL.");
			if( pSmartObject )
			{
				m_nSmartObjectID = pSmartObject->nID;

				// Add the node to the AINodeMgr for each node type that the SmartObject has a command for.
				for(uint32 iNodeType = kNode_BeginUseObject + 1; iNodeType < kNode_EndUseObject; ++iNodeType)
				{
					EnumAINodeType eNodeType = (EnumAINodeType)iNodeType;
					if( pSmartObject->mapCmds.find( eNodeType ) != pSmartObject->mapCmds.end() )
					{
						// Add Node to the NodeMgr again for each defined node type.
						g_pAINodeMgr->AddNode(eNodeType, this);
					}
				}
			}
		}
	}
}

void AINodeUseObject::Init()
{
	super::Init();

	// This is not a valid object.
	if( m_nSmartObjectID == -1 )
		return;

	// Cache childmodels here.
	// Look up the smartobject in the goalbutemgr:
	AIGBM_SmartObjectTemplate *pSmartObject ;
	pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(m_nSmartObjectID);
	
	AIGBM_SmartObjectTemplate::AIChldMdlInfoItr itr, hold_itr ;
		
	// go through the requested childmodels. If some of them cannot be loaded
	// then remove them from the list.
	for( itr = pSmartObject->addchildmodels.begin() ;
		 itr != pSmartObject->addchildmodels.end() ;)
	{
		hold_itr = itr ;
		itr++;
		AIChildModelInfo& aiChildModelInfo = *hold_itr;
		if(g_pModelLT->CacheModelDB(aiChildModelInfo.sFilename.c_str( ), aiChildModelInfo.hmodeldb ) != LT_OK )
		{
			pSmartObject->addchildmodels.erase(hold_itr);
			AIASSERT2(0, m_hObject, "SmartObject %s Could not load file %s ", pSmartObject->szName, aiChildModelInfo.sFilename.c_str( ));
		}
	}
		
}

HOBJECT	AINodeUseObject::GetHObject()
{
	if ( ( !m_hUseObject ) && m_hstrObject )
	{
		HOBJECT hUseObject = NULL;
		LTRESULT res = FindNamedObject( m_hstrObject, hUseObject );
		m_hUseObject = hUseObject;
		AIASSERT1(( res == LT_OK ), m_hObject, "AINodeUseObject::GetHObject: UseObject '%s' does not exist.", ::ToString(m_hstrObject) );
	}

	return m_hUseObject;
}

HSTRING	AINodeUseObject::GetSmartObjectCommand(EnumAINodeType eNodeType)
{
	AIGBM_SmartObjectTemplate *pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(m_nSmartObjectID);

	SMART_OBJECT_CMD_MAP::iterator it = pSmartObject->mapCmds.find(eNodeType);
	if(it != pSmartObject->mapCmds.end())
	{
		return it->second;
	}

	return LTNULL;
}

void AINodeUseObject::PreActivate()
{
	// Run the node's pre-activate command.

	if( m_hstrPreActivateCommand )
	{
		const char *szCmd = g_pLTServer->GetStringData( m_hstrPreActivateCommand );

		if( g_pCmdMgr->IsValidCmd( szCmd ) )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, GetHObject() );
		}
	}
}

void AINodeUseObject::PostActivate()
{
	// Run the node's post-activate command.

	if( m_hstrPostActivateCommand )
	{
		const char *szCmd = g_pLTServer->GetStringData( m_hstrPostActivateCommand );

		if( g_pCmdMgr->IsValidCmd( szCmd ) )
		{
			g_pCmdMgr->Process( szCmd, m_hObject, GetHObject() );
		}
	}

	ResetActivationTime();
}

EnumNodeStatus AINodeUseObject::GetStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	LTVector vThreatPos;
    g_pLTServer->GetObjectPos(hThreat, &vThreatPos);

	// Check if the threat is too close to the AI,
	// and is blocking the path to the node.

	if ( VEC_DISTSQR(vPos, vThreatPos) < g_pAIButeMgr->GetSenses()->fThreatTooCloseDistanceSqr )
	{
		LTVector vToThreat = vThreatPos - vPos;
		LTVector vToNode = m_vPos - vPos;
	
		if( vToThreat.Dot( vToNode ) > c_fFOV60 )
		{
			return kStatus_ThreatBlockingPath;
		}
	}

	if ( VEC_DISTSQR(m_vPos, vThreatPos) < g_pAIButeMgr->GetSenses()->fThreatTooCloseDistanceSqr )
	{
		return kStatus_ThreatInsideRadius;
	}

	return kStatus_Ok;
}


void AINodeUseObject::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HSTRING(m_hstrObject);
	SAVE_HOBJECT(m_hUseObject);
	SAVE_HSTRING(m_hstrPreActivateCommand);
	SAVE_HSTRING(m_hstrPostActivateCommand);
	SAVE_DWORD(m_eMovement);
	SAVE_DWORD(m_nSmartObjectID);
	SAVE_DWORD(m_eFirstSound);
	SAVE_DWORD(m_eFidgetSound);
	SAVE_DWORD(m_eSmartObjectState);
	SAVE_BOOL(m_bOneWay);

// The smartobjects don't need to get saved since they are static across all levels.  AIGoalButeMgr
// initializes all the smartobjects when the server gets initialized.  Saving them, then loading
// them makes no sense.
/*
	// save child model info
	AIGBM_SmartObjectTemplate *pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(m_nSmartObjectID);
	SAVE_DWORD(pSmartObject->addchildmodels.size());
	
	if( pSmartObject->addchildmodels.size())
	{
		AIGBM_SmartObjectTemplate::AIChldMdlInfoItr itr = pSmartObject->addchildmodels.begin();
		AIGBM_SmartObjectTemplate::AIChldMdlInfoItr itr_end = pSmartObject->addchildmodels.end() ;
		

		// save childmodels.
		while(  itr != itr_end	)
		{
			SAVE_CHARSTRING( (*itr).sFilename.c_str( ) );
			itr++;
		}
	}
*/
}

void AINodeUseObject::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HSTRING(m_hstrObject);
	LOAD_HOBJECT(m_hUseObject);
	LOAD_HSTRING(m_hstrPreActivateCommand);
	LOAD_HSTRING(m_hstrPostActivateCommand);
	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
	LOAD_DWORD(m_nSmartObjectID);
	LOAD_DWORD_CAST(m_eFirstSound, EnumAISoundType);
	LOAD_DWORD_CAST(m_eFidgetSound, EnumAISoundType);
	LOAD_DWORD_CAST(m_eSmartObjectState, EnumAIStateType);
	LOAD_BOOL(m_bOneWay);

// The smartobjects don't need to get saved since they are static across all levels.  AIGoalButeMgr
// initializes all the smartobjects when the server gets initialized.  Saving them, then loading
// them makes no sense.
/*
	// Load ChildModels, if there are any.
	uint32 cm_size ;

	LOAD_DWORD(cm_size);
	
	if( cm_size )
	{
		AIGBM_SmartObjectTemplate *pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(m_nSmartObjectID);

		char szFilename[MAX_PATH];
		while( cm_size-- )
		{
			AIChildModelInfo cminfo;
			
			LOAD_CHARSTRING( szFilename, ARRAY_LEN( szFilename ));
			cminfo.sFilename = szFilename;

			if(g_pModelLT->CacheModelDB(cminfo.sFilename.c_str( ), cminfo.hmodeldb ) != LT_OK )
			{
				AIASSERT2(0, m_hObject, "SmartObject %s Could not load file %s ", pSmartObject->szName, cminfo.sFilename.c_str( ));
			}
			else
			{
				pSmartObject->addchildmodels.push_back( cminfo );
			}
		}
	}
*/

	// Cache all the models used by this smartobject.
	AIGBM_SmartObjectTemplate *pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(m_nSmartObjectID);
	if( pSmartObject )
	{
		AIGBM_SmartObjectTemplate::AIChldMdlInfoItr itr, hold_itr ;

		// go through the requested childmodels. If some of them cannot be loaded
		// then remove them from the list.
		for( itr = pSmartObject->addchildmodels.begin() ;
			 itr != pSmartObject->addchildmodels.end() ;)
		{
			hold_itr = itr ;
			itr++;
			AIChildModelInfo& aiChildModelInfo = *hold_itr;
			if(g_pModelLT->CacheModelDB(aiChildModelInfo.sFilename.c_str( ), aiChildModelInfo.hmodeldb ) != LT_OK )
			{
				pSmartObject->addchildmodels.erase(hold_itr);
				AIASSERT2(0, m_hObject, "SmartObject %s Could not load file %s ", pSmartObject->szName, aiChildModelInfo.sFilename.c_str( ));
			}
		}
	}
}

// ------------------------------------------------------------------------
// ChildModels( obj, bInit )
// start means we 
// add/remove childmodels 
// ------------------------------------------------------------------------
void AINodeUseObject::ApplyChildModels( HOBJECT hObj ) 
{ 
	AIGBM_SmartObjectTemplate *pSmartObject = g_pAIGoalButeMgr->GetSmartObjectTemplate(m_nSmartObjectID);

	
	AIGBM_SmartObjectTemplate::AIChldMdlInfoItr itr ;
		
	// go through the requested childmodels. If some of them cannot be loaded
	// then remove them from the list.
	for( itr = pSmartObject->addchildmodels.begin() ;
		 itr != pSmartObject->addchildmodels.end() ;
		 itr++	)
	{
			g_pModelLT->AddChildModelDB( hObj, (*itr).hmodeldb);
	}			
}

extern CAIGoalButeMgr s_AIGoalButeMgr;

LTRESULT AINodeUseObjectPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( g_pAIGoalButeMgr == LTNULL )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\AIGoals.txt", szRezPath);
		s_AIGoalButeMgr.SetInRezFile(LTFALSE);
	    s_AIGoalButeMgr.Init(szFile);
	}
		
	if ( !_strcmpi("Movement", szPropName) )
	{
		strcpy(aszStrings[(*pcStrings)++], "Walk");
		strcpy(aszStrings[(*pcStrings)++], "Run");

		return LT_OK;
	}

	if ( !_strcmpi("SmartObject", szPropName) )
	{
		uint32 cSmartObjects = s_AIGoalButeMgr.GetNumSmartObjectTemplates();
		for(uint32 iSmartObject=0; iSmartObject < cSmartObjects; ++iSmartObject)
		{
			strcpy(aszStrings[(*pcStrings)++], s_AIGoalButeMgr.GetSmartObjectTemplate(iSmartObject)->szName);
		}

		return LT_OK;
	}

	if( ( !_strcmpi("FirstSound", szPropName) ) ||
		( !_strcmpi("FidgetSound", szPropName) ) )
	{
		for(uint32 iSound=0; iSound < kAIS_Count; ++iSound)
		{
			strcpy(aszStrings[(*pcStrings)++], s_aszAISoundTypes[iSound]);
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeBackup)

	ADD_REALPROP_FLAG(AttractRadius,			1024.0f,		0|PF_RADIUS)
	ADD_REALPROP_FLAG(ResetTime,				60.0f,			0)
	ADD_REALPROP_FLAG(StimulusRadius,			512.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(Movement,				"Run", 			0|PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(CryWolfCount,			3,				0)
	ADD_STRINGPROP_FLAG(Command,				"",				0|PF_NOTIFYCHANGE)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodeBackup, AINode, NULL, NULL, 0, AINodeBackupPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeBackup)
CMDMGR_END_REGISTER_CLASS(AINodeBackup, AINode)

AINodeBackup::AINodeBackup()
{
	m_fRadiusSqr		= 0.0f;
	m_eMovement			= kAP_Run;
	m_fStimulusRadius	= 0.f;
	m_fStimulusRadiusSqr= 0.f;
	m_cCryWolf			= 0;
	m_cArrivalCount		= 0;
	m_hstrBackupCmd		= LTNULL;
}

AINodeBackup::~AINodeBackup()
{
	FREE_HSTRING( m_hstrBackupCmd );
}

void AINodeBackup::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

	if ( g_pLTServer->GetPropGeneric( "ResetTime", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fNodeReactivationTime = g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "AttractRadius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fRadiusSqr = g_gp.m_Float*g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "StimulusRadius", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_fStimulusRadius = g_gp.m_Float;
			m_fStimulusRadiusSqr = m_fStimulusRadius * m_fStimulusRadius;
		}
	}

    if ( g_pLTServer->GetPropGeneric( "CryWolfCount", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_cCryWolf = (uint32)g_gp.m_Long;

	// Read movement to use to get to the node.
	if ( g_pLTServer->GetPropGeneric( "Movement", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_eMovement = CAnimationMgrList::GetPropFromName( g_gp.m_String );
		}
	}

	// Read backup command.

	if ( g_pLTServer->GetPropGeneric( "Command", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrBackupCmd = g_pLTServer->CreateString( g_gp.m_String );
		}
	}
}

EnumNodeStatus AINodeBackup::GetStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	LTVector vThreatPos;
    g_pLTServer->GetObjectPos(hThreat, &vThreatPos);

	// Check if the threat is too close to the AI,
	// and is blocking the path to the node.

	if ( VEC_DISTSQR(vPos, vThreatPos) < g_pAIButeMgr->GetSenses()->fThreatTooCloseDistanceSqr )
	{
		LTVector vToThreat = vThreatPos - vPos;
		LTVector vToNode = m_vPos - vPos;
	
		if( vToThreat.Dot( vToNode ) > c_fFOV60 )
		{
			return kStatus_ThreatBlockingPath;
		}
	}

	if ( VEC_DISTSQR(m_vPos, vThreatPos) < g_pAIButeMgr->GetSenses()->fThreatTooCloseDistanceSqr )
	{
		return kStatus_ThreatInsideRadius;
	}

	return kStatus_Ok;
}

void AINodeBackup::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_eMovement);
	SAVE_VECTOR(m_vEnemySeenPos);
	SAVE_FLOAT(m_fStimulusRadius);
	SAVE_FLOAT(m_fStimulusRadiusSqr);
	SAVE_DWORD(m_cCryWolf);
	SAVE_DWORD(m_cArrivalCount);
	SAVE_HSTRING(m_hstrBackupCmd);
}

void AINodeBackup::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
	LOAD_VECTOR(m_vEnemySeenPos);
	LOAD_FLOAT(m_fStimulusRadius);
	LOAD_FLOAT(m_fStimulusRadiusSqr);
	LOAD_DWORD(m_cCryWolf);
	LOAD_DWORD(m_cArrivalCount);
	LOAD_HSTRING(m_hstrBackupCmd);
}

LTRESULT AINodeBackupPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{		
	if ( !_strcmpi("Movement", szPropName) )
	{
		strcpy(aszStrings[(*pcStrings)++], "Walk");
		strcpy(aszStrings[(*pcStrings)++], "Run");

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeTraining)
	
ADD_STRINGPROP_FLAG(Cmd,						"",				0|PF_OBJECTLINK)
ADD_REALPROP_FLAG(Radius,						512.0f,			0|PF_RADIUS)

END_CLASS_DEFAULT(AINodeTraining, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeTraining)
CMDMGR_END_REGISTER_CLASS(AINodeTraining, AINode)

AINodeTraining::AINodeTraining()
{
	m_hstrCmd = LTNULL;
	m_fRadiusSqr = 0.0f;
}

AINodeTraining::~AINodeTraining()
{
	FREE_HSTRING(m_hstrCmd);
}

void AINodeTraining::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

    if ( g_pLTServer->GetPropGeneric( "Cmd", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrCmd = g_pLTServer->CreateString( g_gp.m_String );
}

void AINodeTraining::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HSTRING(m_hstrCmd);
}

void AINodeTraining::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HSTRING(m_hstrCmd);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeTrainingFailure)
END_CLASS_DEFAULT(AINodeTrainingFailure, AINodeTraining, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeTrainingFailure)
CMDMGR_END_REGISTER_CLASS(AINodeTrainingFailure, AINodeTraining)

AINodeTrainingFailure::AINodeTrainingFailure()
{
}

AINodeTrainingFailure::~AINodeTrainingFailure()
{
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeTrainingSuccess)
END_CLASS_DEFAULT(AINodeTrainingSuccess, AINodeTraining, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeTrainingSuccess)
CMDMGR_END_REGISTER_CLASS(AINodeTrainingSuccess, AINodeTraining)

AINodeTrainingSuccess::AINodeTrainingSuccess()
{
}

AINodeTrainingSuccess::~AINodeTrainingSuccess()
{
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeTail)
END_CLASS_DEFAULT(AINodeTail, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeTail)
CMDMGR_END_REGISTER_CLASS(AINodeTail, AINode)

AINodeTail::AINodeTail()
{
}

AINodeTail::~AINodeTail()
{
}

// ----------------------------------------------------------------------- //
AINodePatrolPlugin::AINodePatrolPlugin()
{
	Register("Action", &s_aszAnimProp[kAP_None], 1);
	Register("Action", &s_aszAnimProp[kAP_Wait], 1);
	Register("Action", &s_aszAnimProp[kAP_Fidget], 1);
	Register("Action", &s_aszAnimProp[kAP_LookAround], 1);
}

LTRESULT AINodePatrolPlugin::PreHook_PropChanged( const char *szObjName, const char *szPropName, const int nPropType, const GenericProp &gpPropValue, ILTPreInterface *pInterface, const char *szModifiers )
{
	if( LT_OK == m_AINodePlugin.PreHook_PropChanged( szObjName,
													 szPropName,
													 nPropType, 
													 gpPropValue,
													 pInterface,
													 szModifiers ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

BEGIN_CLASS(AINodePatrol)

	ADD_DEDIT_COLOR( AINodePatrol )

	ADD_STRINGPROP_FLAG(Next,	"",			0|PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Action, "",			0|PF_STATICLIST)
	ADD_STRINGPROP_FLAG(Command,"",			0|PF_NOTIFYCHANGE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodePatrol, AINode, NULL, NULL, 0, AINodePatrolPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINodePatrol)
CMDMGR_END_REGISTER_CLASS(AINodePatrol, AINode)

AINodePatrol::AINodePatrol()
{
	m_pNext = LTNULL;
	m_pPrev = LTNULL;

	m_hstrPatrolCommand = LTNULL;
}

AINodePatrol::~AINodePatrol()
{
	FREE_HSTRING( m_hstrPatrolCommand );
}

void AINodePatrol::Init()
{
	super::Init();

	HOBJECT hObject = LTNULL;
	if ( LT_OK == FindNamedObject(m_hstrNext, hObject) && hObject )
	{	
		if ( IsKindOf(hObject, "AINodePatrol") )
		{
			FREE_HSTRING(m_hstrNext);
			m_pNext = (AINodePatrol*)g_pLTServer->HandleToObject(hObject);

			if( m_pNext == this )
			{
				AIError("AINodePatrol: Node \"%s\" points to self.", ::ToString( GetName() ) );
				m_pNext = LTNULL;
			}
			else {
				m_pNext->SetPrev(this);
			}
		}
		else 
		{
			AIError("AINodePatrol: Next node \"%s\" is not an AINodePatrol", g_pLTServer->GetStringData(m_hstrNext));
			FREE_HSTRING(m_hstrNext);
			m_hstrNext = LTNULL;
		}
	}
	else if( m_hstrNext )
	{
		AIError("AINodePatrol: Could not find Next node \"%s\"", g_pLTServer->GetStringData(m_hstrNext));
		FREE_HSTRING(m_hstrNext);
		m_hstrNext = LTNULL;
	}
}

void AINodePatrol::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

	char *szAction;
	READPROP_STRING("Action", szAction);
	m_ePatrolAction = CAnimationMgrList::GetPropFromName(szAction);

	READPROP_HSTRING("Next", m_hstrNext);

	// Read patrol command.

	if ( g_pLTServer->GetPropGeneric( "Command", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrPatrolCommand = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

}

void AINodePatrol::SetPrev(AINodePatrol* pPrev)
{
	AIASSERT( !m_pPrev, m_hObject, "AINodePatrol::SetPrev: Multiple nodes point to same next." ); 
	m_pPrev = pPrev; 
}

void AINodePatrol::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_ePatrolAction);
	SAVE_COBJECT(m_pNext);
	SAVE_COBJECT(m_pPrev);
	SAVE_HSTRING(m_hstrPatrolCommand);
}

void AINodePatrol::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST(m_ePatrolAction, EnumAnimProp);
	LOAD_COBJECT(m_pNext, AINodePatrol);
	LOAD_COBJECT(m_pPrev, AINodePatrol);
	LOAD_HSTRING(m_hstrPatrolCommand);
}


//----------------------------------------------------------------------------
//              
//	CLASS:		AINodeObstruct
//              
//	PURPOSE:	Obstruction nodes are to be placed in positions where, if an 
//				AI moved to this location, they will .. Obstruct something.
//				Typically, this may mean that the AI is obstruction either 
//				another AI, or player progress.  This is primarily for
//				defensive movements.
//
//				Currently, this is a copy of the vantage node.  This is because
//				at the moment it is unspecialized.  It will be in the future
//				when the requirements for this node are better known.
//              
//----------------------------------------------------------------------------
BEGIN_CLASS(AINodeObstruct)

	ADD_DEDIT_COLOR( AINodeObstruct )

	ADD_BOOLPROP_FLAG(IgnoreDir,				LTFALSE,		0)
	ADD_REALPROP_FLAG(Fov,						90.0f,			0|PF_FIELDOFVIEW)
	ADD_REALPROP_FLAG(Radius,					512.0f,			0|PF_RADIUS)
	ADD_REALPROP_FLAG(ThreatRadius,				256.0f,			0|PF_RADIUS)
	ADD_STRINGPROP_FLAG(ThreatRadiusReaction,	"ATTACK",		0)
	ADD_STRINGPROP_FLAG(DamageCmd,				"REEVALUATE",	0)

END_CLASS_DEFAULT(AINodeObstruct, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeObstruct)
CMDMGR_END_REGISTER_CLASS(AINodeObstruct, AINode)


AINodeObstruct::AINodeObstruct()
{
	m_bIgnoreDir = LTFALSE;
	m_fFovDp = 0.0f;
	m_fRadiusSqr = 0.0f;
	m_fThreatRadiusSqr = 0.0f;
	m_hstrThreatRadiusReaction = LTNULL;
	m_hstrDamageCmd = LTNULL;
	m_eMovement	= kAP_Run;
}

AINodeObstruct::~AINodeObstruct()
{
	FREE_HSTRING(m_hstrThreatRadiusReaction);
	FREE_HSTRING(m_hstrDamageCmd);
}

void AINodeObstruct::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	if ( g_pLTServer->GetPropGeneric( "IgnoreDir", &g_gp ) == LT_OK )
		m_bIgnoreDir = g_gp.m_Bool;

    if ( g_pLTServer->GetPropGeneric( "Fov", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fFovDp = FOV2DP(g_gp.m_Float);

    if ( g_pLTServer->GetPropGeneric( "Radius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fRadiusSqr = g_gp.m_Float*g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "ThreatRadius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fThreatRadiusSqr = g_gp.m_Float*g_gp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "ThreatRadiusReaction", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrThreatRadiusReaction = g_pLTServer->CreateString( g_gp.m_String );

    if ( g_pLTServer->GetPropGeneric( "DamageCmd", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
            m_hstrDamageCmd = g_pLTServer->CreateString( g_gp.m_String );

	// Read movement to use to get to the object.
	if ( g_pLTServer->GetPropGeneric( "Movement", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_eMovement = CAnimationMgrList::GetPropFromName( g_gp.m_String );
}

void AINodeObstruct::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bIgnoreDir);
	LOAD_FLOAT(m_fFovDp);
	LOAD_FLOAT(m_fThreatRadiusSqr);
	LOAD_HSTRING(m_hstrThreatRadiusReaction);
	LOAD_HSTRING(m_hstrDamageCmd);
	LOAD_DWORD_CAST(m_eMovement, EnumAnimProp);
}

void AINodeObstruct::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bIgnoreDir);
	SAVE_FLOAT(m_fFovDp);
	SAVE_FLOAT(m_fThreatRadiusSqr);
	SAVE_HSTRING(m_hstrThreatRadiusReaction);
	SAVE_HSTRING(m_hstrDamageCmd);
	SAVE_DWORD(m_eMovement);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeObstruct::GetStatus()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
EnumNodeStatus AINodeObstruct::GetStatus(const LTVector& vPos, HOBJECT hThreat) const
{
	LTVector vOrigin;
    g_pLTServer->GetObjectPos(hThreat, &vOrigin);

	if ( IsThreatInRadius(vOrigin) )
		return kStatus_ThreatInsideRadius;

	if ( IsIgnoreDir() )
		return kStatus_Ok;

	if ( IsThreatInFOV(vOrigin) )
		return kStatus_Ok;

	return kStatus_ThreatOutsideFOV;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeObstruct::IsThreatInRadius()
//              
//	PURPOSE:	Returns true if the passed point is in the radius, false if it
//				is not.
//              
//----------------------------------------------------------------------------
bool AINodeObstruct::IsThreatInRadius( const LTVector& vThreatPos ) const
{
	if ( VEC_DISTSQR(m_vPos, vThreatPos) < m_fThreatRadiusSqr )
		return true;
	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeObstruct::IsThreatInFOV()
//              
//	PURPOSE:	Returns true if the passed in point is inside the FOV, false
//				if it is not.
//              
//----------------------------------------------------------------------------
bool AINodeObstruct::IsThreatInFOV( const LTVector& vThreatPos ) const
{
	LTVector vThreatDir = vThreatPos - m_vPos;
	vThreatDir.Normalize();

	if ( vThreatDir.Dot(m_vForward) > m_fFovDp )
		return true;
	return false;
}

//----------------------------------------------------------------------------


class AINodeObstructPlugin : public IObjectPlugin
{
	public:
		AINodeObstructPlugin()
		{
			sm_bInitted = LTFALSE;
		}
        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
		{
			if ( !sm_bInitted )
			{
				char szFile[256];
				sprintf(szFile, "%s\\Attributes\\Attachments.txt", szRezPath);
				sm_bInitted = LTTRUE;
				sm_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
			}

			strcpy(aszStrings[0], "<None>");

			for (int x = 0; x < AINodeChangeWeapons::GetWeaponCount(); ++x)
			{
				char szRequirement[128];
				sprintf(szRequirement, "Requirement%d", x);
				if (!strcmp(szPropName,szRequirement))
				{
					sm_WeaponMgrPlugin.PopulateStringList(aszStrings+1, pcStrings, cMaxStrings-1, cMaxStringLength);
					return LT_OK;
				}

				char szChangeTo[128];
				sprintf(szChangeTo, "Weapon%d", x);
				if (!strcmp(szPropName,szChangeTo))
				{
					sm_WeaponMgrPlugin.PopulateStringList(aszStrings+1, pcStrings, cMaxStrings-1, cMaxStringLength);
					return LT_OK;
				}
			}

			return LT_UNSUPPORTED;
		}

	protected :

	private :

		static LTBOOL				sm_bInitted;
		static CWeaponMgrPlugin		sm_WeaponMgrPlugin;
};

LTBOOL AINodeObstructPlugin::sm_bInitted;
CWeaponMgrPlugin AINodeObstructPlugin::sm_WeaponMgrPlugin;

BEGIN_CLASS(AINodeChangeWeapons)

	PROP_DEFINEGROUP(ChangeWeapons, PF_GROUP(6)) \
		ADD_STRINGPROP_FLAG(Requirement0,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Weapon0,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Requirement1,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Weapon1,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Requirement2,	NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \
		ADD_STRINGPROP_FLAG(Weapon2,		NO_ATTACHMENT,		PF_GROUP(6)|PF_STATICLIST) \

END_CLASS_DEFAULT_FLAGS_PLUGIN(AINodeChangeWeapons, AINode, NULL, NULL, CF_HIDDEN, AINodeObstructPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeChangeWeapons)
CMDMGR_END_REGISTER_CLASS(AINodeChangeWeapons, AINode)

const int AINodeChangeWeapons::cm_WeaponCount = 3;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeChangeWeapons Constructor/Destructor
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
AINodeChangeWeapons::AINodeChangeWeapons()
{
}
/*virtual*/ AINodeChangeWeapons::~AINodeChangeWeapons()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeChangeWeapons::ReadProp()
//              
//	PURPOSE:	Reads this classes special data members.
//              
//----------------------------------------------------------------------------
/*virtual*/ void AINodeChangeWeapons::ReadProp(ObjectCreateStruct* pocs)
{
	// Read all of the weapon sets:

	m_WeaponSets.resize( cm_WeaponCount+1 );

	for ( unsigned int x = 0; x < m_WeaponSets.size(); x++ )
	{
		ChangeWeaponSet& WeaponSet = m_WeaponSets.at(x);

		{
			char szChangeTo[128];
			sprintf( szChangeTo, "Weapon%d", x);
			const char* pszChangeToWeapon = pocs->m_cProperties.GetPropString(szChangeTo, NULL);
			(pszChangeToWeapon != NULL && stricmp(pszChangeToWeapon, NO_ATTACHMENT)) ? WeaponSet.m_szChangeToWeapon.assign(pszChangeToWeapon) : WeaponSet.m_szChangeToWeapon.assign("");
		}

		{
			char szRequirement[128];
			sprintf( szRequirement, "Requirement%d", x);
			const char* pszWeaponRequirement = pocs->m_cProperties.GetPropString(szRequirement, NULL);
			(pszWeaponRequirement != NULL && stricmp(pszWeaponRequirement, NO_ATTACHMENT)) ? WeaponSet.m_szChangeToWeaponRequirement.assign( pszWeaponRequirement ) : WeaponSet.m_szChangeToWeaponRequirement.assign("");
		}
	}

	AINode::ReadProp(pocs);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeChangeWeapons::Load()/Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void AINodeChangeWeapons::Save(ILTMessage_Write *pMsg)
{
	// 
	AINode::Save(pMsg);

	SAVE_DWORD(m_WeaponSets.size());
	for ( unsigned int x = 0; x < m_WeaponSets.size(); x++ )
	{
		ChangeWeaponSet& WeaponSet = m_WeaponSets.at(x);

		HSTRING hChangeToWeapon = g_pLTServer->CreateString(const_cast<char*>(WeaponSet.m_szChangeToWeapon.c_str()));
		HSTRING hWeaponRequirement = g_pLTServer->CreateString(const_cast<char*>(WeaponSet.m_szChangeToWeaponRequirement.c_str()));
		
		SAVE_HSTRING( hChangeToWeapon );
		SAVE_HSTRING( hWeaponRequirement );

		FREE_HSTRING(hChangeToWeapon);
		FREE_HSTRING(hWeaponRequirement);
	}
}

/*virtual*/ void AINodeChangeWeapons::Load(ILTMessage_Read *pMsg)
{
	// 
	AINode::Load(pMsg);

	int iSize;
	LOAD_DWORD(iSize);
	m_WeaponSets.resize(iSize);
	for ( unsigned int x = 0; x < m_WeaponSets.size(); x++ )
	{
		ChangeWeaponSet& WeaponSet = m_WeaponSets.at(x);

		HSTRING hChangeToWeapon;
		LOAD_HSTRING(hChangeToWeapon);
		HSTRING hWeaponRequirement;
		LOAD_HSTRING(hWeaponRequirement);
		
		WeaponSet.m_szChangeToWeapon.assign(g_pLTServer->GetStringData(hChangeToWeapon));
		WeaponSet.m_szChangeToWeaponRequirement.assign(g_pLTServer->GetStringData(hWeaponRequirement));

		FREE_HSTRING(hChangeToWeapon);
		FREE_HSTRING(hWeaponRequirement);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeChangeWeapons::Verify()
//              
//	PURPOSE:	Verify that, if we have a valid ChangeToWeapon, we have a 
//				valid
//              
//----------------------------------------------------------------------------
/*virtual*/ void AINodeChangeWeapons::Verify()
{
	for ( unsigned int x = 0; x < m_WeaponSets.size(); x++ )
	{
		ChangeWeaponSet& WeaponSet = m_WeaponSets.at(x);

		if( ( strlen( WeaponSet.m_szChangeToWeapon.c_str() ) == 0 ) &&
			( strlen( WeaponSet.m_szChangeToWeaponRequirement.c_str() ) == 0 ) )
		{
			continue;
		}

		uint8 iNewAmmo, iNewWeapon;
		g_pWeaponMgr->ReadWeapon(const_cast<char*>(WeaponSet.m_szChangeToWeapon.c_str()), iNewWeapon, iNewAmmo );

		uint8 iRequiredAmmo, iRequiredWeapon;
		g_pWeaponMgr->ReadWeapon(const_cast<char*>(WeaponSet.m_szChangeToWeaponRequirement.c_str()), iRequiredWeapon, iRequiredAmmo );

		const WEAPON* pChangeTo = g_pWeaponMgr->GetWeapon(iNewWeapon);
		const WEAPON* pRequired = g_pWeaponMgr->GetWeapon(iRequiredWeapon);

		// Report if something is entered that was not recognized as a weapon.
		if ( WeaponSet.m_szChangeToWeapon.length() != 0 && !pChangeTo )
		{
			Warn("AINode %s has an unrecognized NewWeapon specified: %s!", g_pLTServer->GetStringData(m_hstrName), WeaponSet.m_szChangeToWeaponRequirement.c_str());
		}

		// Report if something is entered that was not recognized as a weapon.
		if ( WeaponSet.m_szChangeToWeaponRequirement.length() != 0 && !pRequired )
		{
			Warn("AINode %s has an unrecognized NewWeapon specified: %s!", g_pLTServer->GetStringData(m_hstrName), WeaponSet.m_szChangeToWeaponRequirement.c_str());
		}

		// If we have a NewWeapon specified, insure that we have a valid 
		// requirement as well.  If we do not, then print out a warning
		// and get rid of the NewWeapon to avoid gameplay bugs
		if ( pChangeTo && !pRequired)
		{
			Warn("AINode %s has a NewWeapon specified without a RequiredWeapon!", g_pLTServer->GetStringData(m_hstrName));
			WeaponSet.m_szChangeToWeapon.assign("");
		}
	}
	AINode::Verify();
}

bool AINodeChangeWeapons::ShouldChangeWeapons(CAIHuman* pAIHuman) const
{
	AIASSERT( pAIHuman, m_hObject, "NULL pAIHuman" );
	if (pAIHuman==NULL)
	{
		return NULL;
	}

	return (!!GetWeaponTransition(pAIHuman));
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeChangeWeapons::ShouldChangeWeapons()
//              
//	PURPOSE:	Returns true if the AI should change weapons, false if it 
//				should not.  The decision is based on matching up the entry 
//				weapon and the new weapon.  If the AI matches the entry weapon
//				AND if the AI does not have active the new weapon, then return 
//				true
//              
//----------------------------------------------------------------------------
const AINodeChangeWeapons::ChangeWeaponSet* const AINodeChangeWeapons::GetWeaponTransition(CAIHuman* pAIHuman) const
{
	AIASSERT( pAIHuman, m_hObject, "NULL pAIHuman" );
	if (pAIHuman==NULL)
	{
		return NULL;
	}

	for ( unsigned int x = 0; x < m_WeaponSets.size(); x++ )
	{
		const ChangeWeaponSet& WeaponSet = m_WeaponSets.at(x);

		uint8 iNewAmmo, iNewWeapon;
		g_pWeaponMgr->ReadWeapon(const_cast<char*>(WeaponSet.m_szChangeToWeapon.c_str()), iNewWeapon, iNewAmmo );

		uint8 iRequiredAmmo, iRequiredWeapon;
		g_pWeaponMgr->ReadWeapon(const_cast<char*>(WeaponSet.m_szChangeToWeaponRequirement.c_str()), iRequiredWeapon, iRequiredAmmo );

		const WEAPON* const pRequiredWeapon = g_pWeaponMgr->GetWeapon(iRequiredWeapon);
		const WEAPON* const pNewWeapon = g_pWeaponMgr->GetWeapon(iNewWeapon);

		if ( !pRequiredWeapon || !pNewWeapon )
		{
			continue;
		}
		
		uint8 iHolsteredAmmo, iHolsteredWeapon;
		g_pWeaponMgr->ReadWeapon(const_cast<char*>(pAIHuman->GetHolsterString()), iHolsteredWeapon, iHolsteredAmmo );

		const WEAPON* const pHolsteredWeapon = pAIHuman->GetHolsterString() ? g_pWeaponMgr->GetWeapon(iHolsteredWeapon) : NULL;
		const WEAPON* const pPrimaryWeapon = pAIHuman->GetPrimaryWeapon() ? pAIHuman->GetPrimaryWeapon()->GetWeaponData() : NULL;

		// If there is a weapon requirement, then insure that the AI either 
		// has the weapon holstered or as the primary.  If the AI has neither
		// then return false.
		bool bRequiredHolstered = (pHolsteredWeapon && pHolsteredWeapon->nDescriptionId == pRequiredWeapon->nDescriptionId );
		bool bRequiredPrimary = (pPrimaryWeapon && pPrimaryWeapon->nDescriptionId == pRequiredWeapon->nDescriptionId );
		if ( !bRequiredHolstered && !bRequiredPrimary)
		{
			continue;
		}

		// If we already have the New weapon either holstered or as our primary 
		// weapon, then there is no reason to do the change, so return false.
		bool bFoundHolstered = (pHolsteredWeapon && pHolsteredWeapon->nDescriptionId == pNewWeapon->nDescriptionId );
		bool bFoundPrimary = (pPrimaryWeapon && pPrimaryWeapon->nDescriptionId == pNewWeapon->nDescriptionId );
		if (bFoundHolstered || bFoundPrimary)
		{
			continue;
		}

		return ( &(m_WeaponSets.at(x)) );
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeChangeWeapons::GetWeaponString()
//              
//	PURPOSE:	Returns the WeaponString describing the new weapon.  Currently
//				this is defined purely as a string which the AI will need to
//				interpret.  This ought to be more centralized.
//              
//----------------------------------------------------------------------------
const char* const AINodeChangeWeapons::GetWeaponString(CAIHuman* pAIHuman) const
{
	AIASSERT( pAIHuman, m_hObject, "NULL pAIHuman" );
	if (pAIHuman==NULL)
	{
		return NULL;
	}

	return (GetWeaponTransition(pAIHuman)->m_szChangeToWeapon.c_str());
}

