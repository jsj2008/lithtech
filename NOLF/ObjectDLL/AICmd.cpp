// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AICmd.h"
#include "AINodeMgr.h"

static GenericProp s_gp;

const char* ToString(const HSTRING* ahstrStrings, const uint32 cStrings)
{
	static char szString[256];
	szString[0] = 0;
	LTBOOL bFirst = LTTRUE;

	for ( uint32 iString = 0 ; iString < cStrings ; iString++ )
	{
		if ( !!ahstrStrings[iString] )
		{
			if ( !bFirst )
			{
				strcat(szString, ",");
			}

			strcat(szString, g_pLTServer->GetStringData(ahstrStrings[iString]));
		}
	}

	return szString;
}

template <typename T> const char* ToString(const T& t);

const char* ToString<LTFLOAT>(const LTFLOAT& fFloat)
{
	static char szString[256];
	sprintf(szString, "%f", fFloat);
	return szString;
}

const char* ToString<uint32>(const uint32& nInt)
{
	static char szString[256];
	sprintf(szString, "%d", nInt);
	return szString;
}

const char* ToString<LTBOOL>(const LTBOOL& bBool)
{
	return bBool ? "TRUE" : "FALSE";
}

const char* ToString<HSTRING>(const HSTRING& hstrString)
{
	return g_pLTServer->GetStringData(hstrString);
}

const char* ToString<LTVector>(const LTVector& vVector)
{
	static char szString[256];
	sprintf(szString, "%f,%f,%f", EXPANDVEC(vVector));
	return szString;
}

const char* ToString<SenseType>(const SenseType& stSenseType)
{
	static char szString[256];
	sprintf(szString, "%d", stSenseType);
	return szString;
}

const char* ToString<Move>(const Move& eMove)
{
	static char szString[256];
	sprintf(szString, "%d", eMove);
	return szString;
}

const char* ToString<Posture>(const Posture& ePosture)
{
	static char szString[256];
	sprintf(szString, "%d", ePosture);
	return szString;
}

const char* ToString<Mood>(const Mood& eMood)
{
	static char szString[256];
	sprintf(szString, "%d", eMood);
	return szString;
}

const char* ToString<Task>(const Task& eTask)
{
	static char szString[256];
	sprintf(szString, "%d", eTask);
	return szString;
}

// ----------------------------------------------------------------------- //

class AICmdPlugin : public IObjectPlugin
{
	public:

        LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
		{
			if (_strcmpi("Move", szPropName) == 0)
			{
				strcpy(aszStrings[(*pcStrings)++], "Run");
				strcpy(aszStrings[(*pcStrings)++], "Walk");
				strcpy(aszStrings[(*pcStrings)++], "Swim");
			}
			else if (_strcmpi("Posture", szPropName) == 0)
			{
				strcpy(aszStrings[(*pcStrings)++], "Crouch");
				strcpy(aszStrings[(*pcStrings)++], "Stand");
			}
			else if (_strcmpi("Mood", szPropName) == 0)
			{
				strcpy(aszStrings[(*pcStrings)++], "Happy");
				strcpy(aszStrings[(*pcStrings)++], "Angry");
				strcpy(aszStrings[(*pcStrings)++], "Sad");
				strcpy(aszStrings[(*pcStrings)++], "Tense");
				strcpy(aszStrings[(*pcStrings)++], "Agree");
				strcpy(aszStrings[(*pcStrings)++], "Disagree");
			}
			else if (_strcmpi("Task", szPropName) == 0)
			{
				strcpy(aszStrings[(*pcStrings)++], "Wait");
				strcpy(aszStrings[(*pcStrings)++], "Clipboard");
				strcpy(aszStrings[(*pcStrings)++], "Dust");
				strcpy(aszStrings[(*pcStrings)++], "Sweep");
				strcpy(aszStrings[(*pcStrings)++], "Wipe");
				strcpy(aszStrings[(*pcStrings)++], "Ticket");
			}
			else
			{
				return LT_UNSUPPORTED;
			}

			return LT_OK;
		}
};

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmd)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AICmd, BaseClass, NULL, NULL, CF_HIDDEN|CF_ALWAYSLOAD, AICmdPlugin)

AICmd::AICmd() : BaseClass(OT_NORMAL)
{
	m_hstrName = LTNULL;
}

AICmd::~AICmd()
{
	FREE_HSTRING(m_hstrName);
}

uint32 AICmd::EngineMessageFn(uint32 messageID, void *pocs, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pocs, fData);

			ReadProp((ObjectCreateStruct*)pocs);

			return dwRet;
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pocs, fData);
}

void AICmd::ReadProp(ObjectCreateStruct *pocs)
{
    if ( g_pLTServer->GetPropGeneric( "Name", &s_gp ) == LT_OK )
		if ( s_gp.m_String[0] )
            m_hstrName = g_pLTServer->CreateString( s_gp.m_String );
}

const char* AICmd::ToString()
{
	strcat(m_szString, " ");
	return m_szString;
}

void AICmd::Verify()
{
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdIdle)
END_CLASS_DEFAULT_FLAGS(AICmdIdle, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdIdle::AICmdIdle()
{
}

AICmdIdle::~AICmdIdle()
{
}

void AICmdIdle::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

const char* AICmdIdle::ToString()
{
	sprintf(m_szString, "IDLE");

	return super::ToString();
}

void AICmdIdle::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAware)
END_CLASS_DEFAULT_FLAGS(AICmdAware, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAware::AICmdAware()
{
}

AICmdAware::~AICmdAware()
{
}

void AICmdAware::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

const char* AICmdAware::ToString()
{
	sprintf(m_szString, "AWARE");

	return super::ToString();
}

void AICmdAware::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdLookAt)
	ADD_VECTORPROP_VAL_FLAG(Position,			0, 0, 0,		0)
END_CLASS_DEFAULT_FLAGS(AICmdLookAt, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdLookAt::AICmdLookAt()
{
}

AICmdLookAt::~AICmdLookAt()
{
}

void AICmdLookAt::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_VECTOR("Position", m_vPosition);
}

const char* AICmdLookAt::ToString()
{
	sprintf(m_szString, "LOOKAT POSITION=%s", ::ToString(m_vPosition));

	return super::ToString();
}

void AICmdLookAt::Verify()
{
	super::Verify();

	if ( LT_INSIDE != g_pLTServer->Common()->GetPointStatus(&m_vPosition) )
	{
		Warn("AICmd \"%s\" - Position %s is not in the world", ::ToString(m_hstrName), ::ToString(m_vPosition));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdTail)
	ADD_STRINGPROP_FLAG(Node1,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node2,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node3,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node4,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node5,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node6,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node7,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node8,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node9,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node10,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node11,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node12,					"",				PF_OBJECTLINK)
END_CLASS_DEFAULT_FLAGS(AICmdTail, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdTail::AICmdTail()
{
}

AICmdTail::~AICmdTail()
{
	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		FREE_HSTRING(m_ahstrNodes[iNode]);
	}
}

void AICmdTail::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		char szProp[128];
		sprintf(szProp, "Node%d", iNode+1);

		READPROP_HSTRING(szProp, m_ahstrNodes[iNode]);
	}
}

const char* AICmdTail::ToString()
{
	sprintf(m_szString, "TAIL PTS=%s", ::ToString(m_ahstrNodes, kMaxNodes));

	return super::ToString();
}

void AICmdTail::Verify()
{
	super::Verify();

	uint32 cValidNodes = 0;
	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		if ( !!m_ahstrNodes[iNode] )
		{
			AINode* pNode = g_pAINodeMgr->GetNode(m_ahstrNodes[iNode]);
			if ( !pNode )
			{
				Warn("AICmd \"%s\" - Tail node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_ahstrNodes[iNode]));
			}
			else if ( pNode->GetType() != AINode::eTypeTail )
			{
				Warn("AICmd \"%s\" - Node \"%s\" is not a tail node", ::ToString(m_hstrName), ::ToString(m_ahstrNodes[iNode]));
			}
			else
			{
				cValidNodes++;
			}
		}
	}

	if ( cValidNodes < 3 )
	{
		Warn("AICmd \"%s\" - %s valid tail nodes specified, need a minimum of 3", ::ToString(m_hstrName), ::ToString(cValidNodes));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdFollowFootprint)
	ADD_BOOLPROP_FLAG(Search,					LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdFollowFootprint, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdFollowFootprint::AICmdFollowFootprint()
{
}

AICmdFollowFootprint::~AICmdFollowFootprint()
{
}

void AICmdFollowFootprint::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_BOOL("Search", m_bSearch);
}

const char* AICmdFollowFootprint::ToString()
{
	sprintf(m_szString, "FOLLOWFOOTPRINT SEARCH=%s", ::ToString(m_bSearch));

	return super::ToString();
}

void AICmdFollowFootprint::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdInvestigate)
	ADD_STRINGPROP_FLAG(Enemy,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(SenseType,				"",				0)
	ADD_VECTORPROP_VAL_FLAG(Position,			0, 0, 0,		0)
	ADD_BOOLPROP_FLAG(Search,					LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdInvestigate, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdInvestigate::AICmdInvestigate()
{
}

AICmdInvestigate::~AICmdInvestigate()
{
	FREE_HSTRING(m_hstrEnemy);
}

void AICmdInvestigate::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszSenseTypes[] = { "RUN", "WALK", "SWIM" };
	SenseType aeSenseTypes[] = { (SenseType)0, (SenseType)0, (SenseType)0 };
	READPROP_STRINGENUM("SenseType", m_stSenseType, aszSenseTypes, aeSenseTypes, 3);

	READPROP_HSTRING("Enemy", m_hstrEnemy);
	READPROP_VECTOR("Position", m_vPosition);
	READPROP_BOOL("Search", m_bSearch);
}

const char* AICmdInvestigate::ToString()
{
	sprintf(m_szString, "INVESTIGATE SEARCH=%s POSITION=%s ENEMY=%s SENSETYPE=%s", ::ToString(m_bSearch), ::ToString(m_vPosition), ::ToString(m_hstrEnemy), ::ToString(m_stSenseType));

	return super::ToString();
}

void AICmdInvestigate::Verify()
{
	super::Verify();

	if ( LT_INSIDE != g_pLTServer->Common()->GetPointStatus(&m_vPosition) )
	{
		Warn("AICmd \"%s\" - Position %s is not in the world", ::ToString(m_hstrName), ::ToString(m_vPosition));
	}

	if ( !!m_hstrEnemy )
	{
		HOBJECT hObject;
		if ( LT_OK != FindNamedObject(g_pLTServer->GetStringData(m_hstrEnemy), hObject) )
		{
			Warn("AICmd \"%s\" - Could not find enemy \"%s\"", ::ToString(m_hstrName), ::ToString(m_hstrEnemy));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No enemy specified", ::ToString(m_hstrName));
	}

}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdCheckBody)
	ADD_STRINGPROP_FLAG(Body,					"",				PF_OBJECTLINK)
	ADD_BOOLPROP_FLAG(Search,					LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdCheckBody, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdCheckBody::AICmdCheckBody()
{
}

AICmdCheckBody::~AICmdCheckBody()
{
	FREE_HSTRING(m_hstrBody);
}

void AICmdCheckBody::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_HSTRING("Body", m_hstrBody);
	READPROP_BOOL("Search", m_bSearch);
}

const char* AICmdCheckBody::ToString()
{
	sprintf(m_szString, "CHECKBODY SEARCH=%s BODY=%s", ::ToString(m_bSearch), ::ToString(m_hstrBody));

	return super::ToString();
}

void AICmdCheckBody::Verify()
{
	super::Verify();

	if ( !!m_hstrBody )
	{
		HOBJECT hObject;
		if ( LT_OK != FindNamedObject(g_pLTServer->GetStringData(m_hstrBody), hObject) )
		{
			Warn("AICmd \"%s\" - Could not find body \"%s\"", ::ToString(m_hstrName), ::ToString(m_hstrBody));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No body specified", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdSearch)
	ADD_BOOLPROP_FLAG(Engage,					LTFALSE,		0)
	ADD_BOOLPROP_FLAG(Face,						LTTRUE,			0)
	ADD_BOOLPROP_FLAG(Pause,					LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdSearch, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdSearch::AICmdSearch()
{
}

AICmdSearch::~AICmdSearch()
{
}

void AICmdSearch::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_BOOL("Engage", m_bEngage);
	READPROP_BOOL("Face", m_bFace);
	READPROP_BOOL("Pause", m_bPause);
}

const char* AICmdSearch::ToString()
{
	sprintf(m_szString, "SEARCH ENGAGE=%s FACE=%s PAUSE=%s", ::ToString(m_bEngage), ::ToString(m_bFace), ::ToString(m_bPause));

	return super::ToString();
}

void AICmdSearch::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdCharge)
	ADD_REALPROP_FLAG(AttackDist,				0.0f,			0)
	ADD_REALPROP_FLAG(YellDist,					0.0f,			0)
	ADD_REALPROP_FLAG(StopDist,					0.0f,			0)
END_CLASS_DEFAULT_FLAGS(AICmdCharge, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdCharge::AICmdCharge()
{
}

AICmdCharge::~AICmdCharge()
{
}

void AICmdCharge::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_FLOAT("AttackDist", m_fAttackDist);
	READPROP_FLOAT("YellDist", m_fYellDist);
	READPROP_FLOAT("StopDist", m_fStopDist);
}

const char* AICmdCharge::ToString()
{
	sprintf(m_szString, "CHARGE ATTACKDIST=%s YELLDIST=%s STOPDIST=%s", ::ToString(m_fAttackDist), ::ToString(m_fYellDist), ::ToString(m_fStopDist));

	return super::ToString();
}

void AICmdCharge::Verify()
{
	super::Verify();

	if ( m_fAttackDist < 0.0f )
	{
		Warn("AICmd \"%s\" - AttackDist is less than 0.0", ::ToString(m_hstrName));
	}

	if ( m_fYellDist < 0.0f )
	{
		Warn("AICmd \"%s\" - YellDist is less than 0.0", ::ToString(m_hstrName));
	}

	if ( m_fStopDist < 0.0f )
	{
		Warn("AICmd \"%s\" - StopDist is less than 0.0", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdChase)
END_CLASS_DEFAULT_FLAGS(AICmdChase, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdChase::AICmdChase()
{
}

AICmdChase::~AICmdChase()
{
}

void AICmdChase::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

const char* AICmdChase::ToString()
{
	sprintf(m_szString, "CHASE");

	return super::ToString();
}

void AICmdChase::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdPanic)
	ADD_STRINGPROP_FLAG(Node,					"",				PF_OBJECTLINK)
	ADD_BOOLPROP_FLAG(CanActivate,				LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdPanic, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdPanic::AICmdPanic()
{
}

AICmdPanic::~AICmdPanic()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdPanic::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_HSTRING("Node", m_hstrNode);
	READPROP_BOOL("CanActivate", m_bCanActivate);
}

const char* AICmdPanic::ToString()
{
	sprintf(m_szString, "PANIC DEST=%s CANACTIVATE=%s", ::ToString(m_hstrNode), ::ToString(m_bCanActivate));

	return super::ToString();
}

void AICmdPanic::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - Panic node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypePanic )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a panic node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No panic node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdDistress)
	ADD_BOOLPROP_FLAG(CanActivate,				LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdDistress, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdDistress::AICmdDistress()
{
}

AICmdDistress::~AICmdDistress()
{
}

void AICmdDistress::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_BOOL("CanActivate", m_bCanActivate);
}

const char* AICmdDistress::ToString()
{
	sprintf(m_szString, "DISTRESS CANACTIVATE=%s", ::ToString(m_bCanActivate));

	return super::ToString();
}

void AICmdDistress::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdDrowsy)
END_CLASS_DEFAULT_FLAGS(AICmdDrowsy, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdDrowsy::AICmdDrowsy()
{
}

AICmdDrowsy::~AICmdDrowsy()
{
}

void AICmdDrowsy::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

const char* AICmdDrowsy::ToString()
{
	sprintf(m_szString, "DROWSY");

	return super::ToString();
}

void AICmdDrowsy::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdUnconscious)
	ADD_BOOLPROP_FLAG(Aware,					LTTRUE,			0)
	ADD_REALPROP_FLAG(Time,						15.0f,			0)
END_CLASS_DEFAULT_FLAGS(AICmdUnconscious, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdUnconscious::AICmdUnconscious()
{
}

AICmdUnconscious::~AICmdUnconscious()
{
}

void AICmdUnconscious::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_BOOL("Aware", m_bAware);
	READPROP_FLOAT("Time", m_fTime);
}

const char* AICmdUnconscious::ToString()
{
	sprintf(m_szString, "UNCONSCIOUS AWARE=%s TIME=%s", ::ToString(m_bAware), ::ToString(m_fTime));

	return super::ToString();
}

void AICmdUnconscious::Verify()
{
	super::Verify();

	if ( m_fTime < 0.0f )
	{
		Warn("AICmd \"%s\" - Time is less than 0.0", ::ToString(m_hstrName), ::ToString(m_fTime));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdStunned)
END_CLASS_DEFAULT_FLAGS(AICmdStunned, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdStunned::AICmdStunned()
{
}

AICmdStunned::~AICmdStunned()
{
}

void AICmdStunned::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

const char* AICmdStunned::ToString()
{
	sprintf(m_szString, "STUNNED");

	return super::ToString();
}

void AICmdStunned::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAttack)
	ADD_BOOLPROP_FLAG(Chase,					LTTRUE,			0)
	ADD_REALPROP_FLAG(ChaseDelay,				0.0f,			0)
	ADD_STRINGPROP_FLAG(Posture,				"",				0|PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS(AICmdAttack, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAttack::AICmdAttack()
{
}

AICmdAttack::~AICmdAttack()
{
}

void AICmdAttack::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_BOOL("Chase", m_bChase);
	READPROP_FLOAT("ChaseDelay", m_fChaseDelay);

	const char* aszPostures[] = { "RUN", "WALK", "SWIM" };
	Posture aePostures[] = { (Posture)0, (Posture)0, (Posture)0, };
	READPROP_STRINGENUM("Posture", m_ePosture, aszPostures, aePostures, 3);
}

const char* AICmdAttack::ToString()
{
	sprintf(m_szString, "ATTACK CHASE=%s CHASEDELAY=%s POSTURE=%s", ::ToString(m_bChase), ::ToString(m_fChaseDelay), ::ToString(m_ePosture));

	return super::ToString();
}

void AICmdAttack::Verify()
{
	super::Verify();

	if ( m_fChaseDelay < 0.0f )
	{
		Warn("AICmd \"%s\" - ChaseDelay is less than 0.0", ::ToString(m_hstrName), ::ToString(m_fChaseDelay));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdDraw)
END_CLASS_DEFAULT_FLAGS(AICmdDraw, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdDraw::AICmdDraw()
{
}

AICmdDraw::~AICmdDraw()
{
}

void AICmdDraw::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

const char* AICmdDraw::ToString()
{
	sprintf(m_szString, "DRAW");

	return super::ToString();
}

void AICmdDraw::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAttackFromCover)
	ADD_STRINGPROP_FLAG(Node,					"",				PF_OBJECTLINK)
	ADD_LONGINTPROP_FLAG(Retries,				0,				0)
END_CLASS_DEFAULT_FLAGS(AICmdAttackFromCover, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAttackFromCover::AICmdAttackFromCover()
{
}

AICmdAttackFromCover::~AICmdAttackFromCover()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdAttackFromCover::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_INT("Retries", m_nRetries);
	READPROP_HSTRING("Node", m_hstrNode);
}

const char* AICmdAttackFromCover::ToString()
{
	sprintf(m_szString, "ATTACKFROMCOVER RETRIES=%s NODE=%s", ::ToString(m_nRetries), ::ToString(m_hstrNode));

	return super::ToString();
}

void AICmdAttackFromCover::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - Cover node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypeCover )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a cover node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No cover node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAttackFromVantage)
END_CLASS_DEFAULT_FLAGS(AICmdAttackFromVantage, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAttackFromVantage::AICmdAttackFromVantage()
{
}

AICmdAttackFromVantage::~AICmdAttackFromVantage()
{
}

void AICmdAttackFromVantage::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

const char* AICmdAttackFromVantage::ToString()
{
	sprintf(m_szString, "ATTACKFROMVANTAGE");

	return super::ToString();
}

void AICmdAttackFromVantage::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAttackFromView)
	ADD_STRINGPROP_FLAG(Node,					"",				PF_OBJECTLINK)
END_CLASS_DEFAULT_FLAGS(AICmdAttackFromView, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAttackFromView::AICmdAttackFromView()
{
}

AICmdAttackFromView::~AICmdAttackFromView()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdAttackFromView::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_HSTRING("Node", m_hstrNode);
}

const char* AICmdAttackFromView::ToString()
{
	sprintf(m_szString, "ATTACKFROMVIEW DEST=%s", ::ToString(m_hstrNode));

	return super::ToString();
}

void AICmdAttackFromView::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - View node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypeView )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a view node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No view node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAttackOnSight)
	ADD_REALPROP_FLAG(ChaseDelay,				0.0f,			0)
END_CLASS_DEFAULT_FLAGS(AICmdAttackOnSight, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAttackOnSight::AICmdAttackOnSight()
{
}

AICmdAttackOnSight::~AICmdAttackOnSight()
{
}

void AICmdAttackOnSight::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_FLOAT("ChaseDelay", m_fChaseDelay);
}

const char* AICmdAttackOnSight::ToString()
{
	sprintf(m_szString, "ATTACKONSIGHT CHASEDELAY=%s", ::ToString(m_fChaseDelay));

	return super::ToString();
}

void AICmdAttackOnSight::Verify()
{
	super::Verify();

	if ( m_fChaseDelay < 0.0f )
	{
		Warn("AICmd \"%s\" - ChaseDelay is less than 0.0", ::ToString(m_hstrName), ::ToString(m_fChaseDelay));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAssassinate)
	ADD_STRINGPROP_FLAG(Node,					"",				0|PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Move,					"WALK",			0|PF_STATICLIST)
	ADD_BOOLPROP_FLAG(IgnoreVisibility,			LTTRUE,			0)
END_CLASS_DEFAULT_FLAGS(AICmdAssassinate, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAssassinate::AICmdAssassinate()
{
}

AICmdAssassinate::~AICmdAssassinate()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdAssassinate::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_HSTRING("Node", m_hstrNode);
	READPROP_BOOL("IgnoreVisibility", m_bIgnoreVisibility);

	const char* aszMoves[] = { "RUN", "WALK", "SWIM" };
	Move aeMoves[] = { eMoveRun, eMoveWalk, eMoveSwim };
	READPROP_STRINGENUM("Move", m_eMove, aszMoves, aeMoves, 3);
}

const char* AICmdAssassinate::ToString()
{
	sprintf(m_szString, "ASSASSINATE DEST=%s IGNOREVIS=%s, MOVE=%s", ::ToString(m_hstrNode), ::ToString(m_bIgnoreVisibility), ::ToString(m_eMove));

	return super::ToString();
}

void AICmdAssassinate::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - Assassinate node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypeAssassinate )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a assassinate node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No assassinate node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdCover)
	ADD_STRINGPROP_FLAG(Node,					"",				PF_OBJECTLINK)
	ADD_LONGINTPROP_FLAG(Retries,				0,				0)
END_CLASS_DEFAULT_FLAGS(AICmdCover, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdCover::AICmdCover()
{
}

AICmdCover::~AICmdCover()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdCover::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_INT("Retries", m_nRetries);
	READPROP_HSTRING("Node", m_hstrNode);
}

const char* AICmdCover::ToString()
{
	sprintf(m_szString, "COVER RETRIES=%s NODE=%s", ::ToString(m_nRetries), ::ToString(m_hstrNode));

	return super::ToString();
}

void AICmdCover::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - Cover node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypeCover )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a cover node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No cover node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdPatrol)
	ADD_STRINGPROP_FLAG(Node1,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node2,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node3,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node4,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node5,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node6,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node7,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node8,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node9,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node10,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node11,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node12,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Task,					"WAIT",			0|PF_STATICLIST)
	ADD_BOOLPROP_FLAG(Face,						LTTRUE,			0)
	ADD_BOOLPROP_FLAG(Loop,						LTTRUE,			0)
	ADD_BOOLPROP_FLAG(Circle,					LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdPatrol, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdPatrol::AICmdPatrol()
{
}

AICmdPatrol::~AICmdPatrol()
{
	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		FREE_HSTRING(m_ahstrNodes[iNode]);
	}
}

void AICmdPatrol::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszTasks[] = { "RUN", "WALK", "SWIM" };
	Task aeTasks[] = { (Task)0, (Task)0, (Task)0, };
	READPROP_STRINGENUM("Task", m_eTask, aszTasks, aeTasks, 3);

	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		char szProp[128];
		sprintf(szProp, "Node%d", iNode+1);

		READPROP_HSTRING(szProp, m_ahstrNodes[iNode]);
	}

	READPROP_BOOL("Face", m_bFace);
	READPROP_BOOL("Loop", m_bLoop);
	READPROP_BOOL("Circle", m_bCircle);
}

const char* AICmdPatrol::ToString()
{
	sprintf(m_szString, "PATROL PTS=%s TASK=%s FACE=%s LOOP=%s CIRLCE=%s", ::ToString(m_ahstrNodes, kMaxNodes), ::ToString(m_eTask), ::ToString(m_bFace), ::ToString(m_bLoop), ::ToString(m_bCircle));

	return super::ToString();
}

void AICmdPatrol::Verify()
{
	super::Verify();

	uint32 cValidNodes = 0;
	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		if ( !!m_ahstrNodes[iNode] )
		{
			AINode* pNode = g_pAINodeMgr->GetNode(m_ahstrNodes[iNode]);
			if ( !pNode )
			{
				Warn("AICmd \"%s\" - Patrol node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_ahstrNodes[iNode]));
			}
			else if ( pNode->GetType() != AINode::eTypePatrol )
			{
				Warn("AICmd \"%s\" - Node \"%s\" is not a patrol node", ::ToString(m_hstrName), ::ToString(m_ahstrNodes[iNode]));
			}
			else
			{
				cValidNodes++;
			}
		}
	}

	if ( cValidNodes < 2 )
	{
		Warn("AICmd \"%s\" - %s valid patrol nodes specified, need a minimum of 2", ::ToString(m_hstrName), ::ToString(cValidNodes));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdGoto)
	ADD_STRINGPROP_FLAG(Node1,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node2,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node3,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node4,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node5,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node6,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node7,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node8,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node9,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node10,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node11,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Node12,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Move,					"WALK",			0|PF_STATICLIST)
	ADD_BOOLPROP_FLAG(Face,						LTTRUE,			0)
	ADD_BOOLPROP_FLAG(Loop,						LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdGoto, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdGoto::AICmdGoto()
{
}

AICmdGoto::~AICmdGoto()
{
	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		FREE_HSTRING(m_ahstrNodes[iNode]);
	}
}

void AICmdGoto::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszMoves[] = { "RUN", "WALK", "SWIM" };
	Move aeMoves[] = { eMoveRun, eMoveWalk, eMoveSwim };
	READPROP_STRINGENUM("Move", m_eMove, aszMoves, aeMoves, 3);

	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		char szProp[128];
		sprintf(szProp, "Node%d", iNode+1);

		READPROP_HSTRING(szProp, m_ahstrNodes[iNode]);
	}

	READPROP_BOOL("Face", m_bFace);
	READPROP_BOOL("Loop", m_bLoop);
}

const char* AICmdGoto::ToString()
{
	sprintf(m_szString, "GOTO PTS=%s FACE=%s LOOP=%s", ::ToString(m_ahstrNodes, kMaxNodes), ::ToString(m_bFace), ::ToString(m_bLoop));

	return super::ToString();
}

void AICmdGoto::Verify()
{
	super::Verify();

	uint32 cValidNodes = 0;
	for ( uint32 iNode = 0 ; iNode < kMaxNodes ; iNode++ )
	{
		if ( !!m_ahstrNodes[iNode] )
		{
			AINode* pNode = g_pAINodeMgr->GetNode(m_ahstrNodes[iNode]);
			if ( !pNode )
			{
				Warn("AICmd \"%s\" - Goto node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_ahstrNodes[iNode]));
			}
			else if ( pNode->GetType() != AINode::eTypeGoto )
			{
				Warn("AICmd \"%s\" - Node \"%s\" is not a goto node", ::ToString(m_hstrName), ::ToString(m_ahstrNodes[iNode]));
			}
			else
			{
				cValidNodes++;
			}
		}
	}

	if ( cValidNodes < 1 )
	{
		Warn("AICmd \"%s\" - %s valid goto nodes specified, need a minimum of 1", ::ToString(m_hstrName), ::ToString(cValidNodes));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdFlee)
	ADD_STRINGPROP_FLAG(Danger,					"",				PF_OBJECTLINK)
END_CLASS_DEFAULT_FLAGS(AICmdFlee, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdFlee::AICmdFlee()
{
}

AICmdFlee::~AICmdFlee()
{
	FREE_HSTRING(m_hstrDanger);
}

void AICmdFlee::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_HSTRING("Danger", m_hstrDanger);
}

const char* AICmdFlee::ToString()
{
	sprintf(m_szString, "FLEE DANGER=%s", ::ToString(m_hstrDanger));

	return super::ToString();
}

void AICmdFlee::Verify()
{
	super::Verify();

	if ( !!m_hstrDanger )
	{
		HOBJECT hObject;
		if ( LT_OK != FindNamedObject(g_pLTServer->GetStringData(m_hstrDanger), hObject) )
		{
			Warn("AICmd \"%s\" - Could not find danger \"%s\"", ::ToString(m_hstrName), ::ToString(m_hstrDanger));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No danger specified", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdFollow)
	ADD_REALPROP_FLAG(Range,					100.0f,			0)
	ADD_REALPROP_FLAG(RangeTime,				0.25f,			0)
	ADD_STRINGPROP_FLAG(Move,					"WALK",			0|PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS(AICmdFollow, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdFollow::AICmdFollow()
{
}

AICmdFollow::~AICmdFollow()
{
}

void AICmdFollow::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszMoves[] = { "RUN", "WALK", "SWIM" };
	Move aeMoves[] = { eMoveRun, eMoveWalk, eMoveSwim };
	READPROP_STRINGENUM("Move", m_eMove, aszMoves, aeMoves, 3);

	READPROP_FLOAT("Range", m_fRange);
	READPROP_FLOAT("RangeTime", m_fRangeTime);
}

const char* AICmdFollow::ToString()
{
	sprintf(m_szString, "FOLLOW RANGE=%s RANGETIME=%s MOVE=%s", ::ToString(m_fRange), ::ToString(m_fRangeTime), ::ToString(m_eMove));

	return super::ToString();
}

void AICmdFollow::Verify()
{
	super::Verify();

	if ( m_fRange < 0.0f )
	{
		Warn("AICmd \"%s\" - Range is less than 0.0", ::ToString(m_hstrName));
	}

	if ( m_fRangeTime < 0.0f )
	{
		Warn("AICmd \"%s\" - RangeTime is less than 0.0", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdGetBackup)
	ADD_STRINGPROP_FLAG(Node,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Move,					"WALK",			0|PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS(AICmdGetBackup, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdGetBackup::AICmdGetBackup()
{
}

AICmdGetBackup::~AICmdGetBackup()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdGetBackup::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszMoves[] = { "RUN", "WALK", "SWIM" };
	Move aeMoves[] = { eMoveRun, eMoveWalk, eMoveSwim };
	READPROP_STRINGENUM("Move", m_eMove, aszMoves, aeMoves, 3);

	READPROP_HSTRING("Node", m_hstrNode);
}

const char* AICmdGetBackup::ToString()
{
	sprintf(m_szString, "GETBACKUP DEST=%s MOVE=%s", ::ToString(m_hstrNode), ::ToString(m_eMove));

	return super::ToString();
}

void AICmdGetBackup::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - Backup node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypeCover )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a backup node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No backup node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdUseObject)
	ADD_STRINGPROP_FLAG(Node,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Move,					"WALK",			0|PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS(AICmdUseObject, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdUseObject::AICmdUseObject()
{
}

AICmdUseObject::~AICmdUseObject()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdUseObject::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszMoves[] = { "RUN", "WALK", "SWIM" };
	Move aeMoves[] = { eMoveRun, eMoveWalk, eMoveSwim };
	READPROP_STRINGENUM("Move", m_eMove, aszMoves, aeMoves, 3);

	READPROP_HSTRING("Node", m_hstrNode);
}

const char* AICmdUseObject::ToString()
{
	sprintf(m_szString, "USEOBJECT DEST=%s MOVE=%s", ::ToString(m_hstrNode), ::ToString(m_eMove));

	return super::ToString();
}

void AICmdUseObject::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - UseObject node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypeCover )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a useobject node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No useobject node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdPickupObject)
	ADD_STRINGPROP_FLAG(Node,					"",				PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Move,					"WALK",			0|PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS(AICmdPickupObject, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdPickupObject::AICmdPickupObject()
{
}

AICmdPickupObject::~AICmdPickupObject()
{
	FREE_HSTRING(m_hstrNode);
}

void AICmdPickupObject::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszMoves[] = { "RUN", "WALK", "SWIM" };
	Move aeMoves[] = { eMoveRun, eMoveWalk, eMoveSwim };
	READPROP_STRINGENUM("Move", m_eMove, aszMoves, aeMoves, 3);

	READPROP_HSTRING("Node", m_hstrNode);
}

const char* AICmdPickupObject::ToString()
{
	sprintf(m_szString, "PICKUPOBJECT DEST=%s MOVE=%s", ::ToString(m_hstrNode), ::ToString(m_eMove));

	return super::ToString();
}

void AICmdPickupObject::Verify()
{
	super::Verify();

	if ( m_hstrNode )
	{
		AINode* pNode = g_pAINodeMgr->GetNode(m_hstrNode);
		if ( !pNode )
		{
			Warn("AICmd \"%s\" - PickupObject node \"%s\" does not exist", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
		else if ( pNode->GetType() != AINode::eTypeCover )
		{
			Warn("AICmd \"%s\" - Node \"%s\" is not a pickupobject node", ::ToString(m_hstrName), ::ToString(m_hstrNode));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No pickupobject node specified!", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdTalk)
	ADD_STRINGPROP_FLAG(Face,					"",				PF_OBJECTLINK)
	ADD_REALPROP_FLAG(FaceTime,					15.0f,			0)
	ADD_STRINGPROP_FLAG(Mood,					"HAPPY",		0|PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS(AICmdTalk, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdTalk::AICmdTalk()
{
}

AICmdTalk::~AICmdTalk()
{
	FREE_HSTRING(m_hstrFace);
}

void AICmdTalk::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	const char* aszMoods[] = { "RUN", "WALK", "SWIM" };
	Mood aeMoods[] = { (Mood)0, (Mood)0, (Mood)0 };
	READPROP_STRINGENUM("Mood", m_eMood, aszMoods, aeMoods, 3);

	READPROP_HSTRING("Face", m_hstrFace);
	READPROP_FLOAT("FaceTime", m_fFaceTime);
}

const char* AICmdTalk::ToString()
{
	sprintf(m_szString, "TALK FACE=%s FACETIME=%s MOOD=%s", ::ToString(m_hstrFace), ::ToString(m_fFaceTime), ::ToString(m_eMood));

	return super::ToString();
}

void AICmdTalk::Verify()
{
	super::Verify();

	HOBJECT hObject;
	if ( !!m_hstrFace && LT_OK != FindNamedObject(g_pLTServer->GetStringData(m_hstrFace), hObject) )
	{
		Warn("AICmd \"%s\" - Could not find face \"%s\"", ::ToString(m_hstrName), ::ToString(m_hstrFace));
	}

	if ( m_fFaceTime < 0.0f )
	{
		Warn("AICmd \"%s\" - FaceTime is less than 0.0", ::ToString(m_hstrName));
	}
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAnimate)
	ADD_STRINGPROP_FLAG(Animation,				"",				0)
	ADD_BOOLPROP_FLAG(Loop,						LTFALSE,		0)
END_CLASS_DEFAULT_FLAGS(AICmdAnimate, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAnimate::AICmdAnimate()
{
}

AICmdAnimate::~AICmdAnimate()
{
	FREE_HSTRING(m_hstrAnimation);
}

void AICmdAnimate::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_HSTRING("Animation", m_hstrAnimation);
	READPROP_BOOL("Loop", m_bLoop);
}

const char* AICmdAnimate::ToString()
{
	sprintf(m_szString, "ANIMATE ANIM=%s LOOP=%s", ::ToString(m_hstrAnimation), ::ToString(m_bLoop));

	return super::ToString();
}

void AICmdAnimate::Verify()
{
	super::Verify();
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AICmdAttackProp)
	ADD_STRINGPROP_FLAG(Prop,					"",				PF_OBJECTLINK)
END_CLASS_DEFAULT_FLAGS(AICmdAttackProp, AICmd, NULL, NULL, CF_ALWAYSLOAD)

AICmdAttackProp::AICmdAttackProp()
{
}

AICmdAttackProp::~AICmdAttackProp()
{
	FREE_HSTRING(m_hstrProp);
}

void AICmdAttackProp::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);

	READPROP_HSTRING("Prop", m_hstrProp);
}

const char* AICmdAttackProp::ToString()
{
	sprintf(m_szString, "ATTACKPROP PROP=%s LOOP=%s", ::ToString(m_hstrProp));

	return super::ToString();
}

void AICmdAttackProp::Verify()
{
	super::Verify();

	if ( !!m_hstrProp )
	{
		HOBJECT hObject;
		if ( LT_OK != FindNamedObject(g_pLTServer->GetStringData(m_hstrProp), hObject) )
		{
			Warn("AICmd \"%s\" - Could not find prop \"%s\"", ::ToString(m_hstrName), ::ToString(m_hstrProp));
		}
	}
	else
	{
		Warn("AICmd \"%s\" - No prop specified", ::ToString(m_hstrName));
	}
}
