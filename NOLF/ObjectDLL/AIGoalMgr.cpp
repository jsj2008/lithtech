// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIGoal.h"
#include "AINodeMgr.h"

static GenericProp s_gp;
static char s_szString[4096];

// ----------------------------------------------------------------------- //

class AIGoalPlugin : public IObjectPlugin
{
	public:

        LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
		{
			if ( 1 )
			{
				return LT_UNSUPPORTED;
			}

			return LT_OK;
		}
};

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIGoal)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AIGoal, BaseClass, NULL, NULL, CF_HIDDEN|CF_ALWAYSLOAD, AIGoalPlugin)

AIGoal::AIGoal() : BaseClass(OT_NORMAL)
{
	m_hstrName = LTNULL;
}

AIGoal::~AIGoal()
{
	FREE_HSTRING(m_hstrName);
}

uint32 AIGoal::EngineMessageFn(uint32 messageID, void *pocs, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = super::EngineMessageFn(messageID, pocs, fData);

			ReadProp((ObjectCreateStruct*)pocs);

			return dwRet;
		}
		break;
	}

	return super::EngineMessageFn(messageID, pocs, fData);
}

void AIGoal::ReadProp(ObjectCreateStruct *pocs)
{
	READPROP_HSTRING("Name", m_hstrName);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIGoalAttack)
END_CLASS_DEFAULT_FLAGS(AIGoalAttack, AIGoal, NULL, NULL, CF_ALWAYSLOAD)

AIGoalAttack::AIGoalAttack()
{
}

AIGoalAttack::~AIGoalAttack()
{
}

void AIGoalAttack::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIGoalInvestigate)
END_CLASS_DEFAULT_FLAGS(AIGoalInvestigate, AIGoal, NULL, NULL, CF_ALWAYSLOAD)

AIGoalInvestigate::AIGoalInvestigate()
{
}

AIGoalInvestigate::~AIGoalInvestigate()
{
}

void AIGoalInvestigate::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIGoalAlarm)
END_CLASS_DEFAULT_FLAGS(AIGoalAlarm, AIGoal, NULL, NULL, CF_ALWAYSLOAD)

AIGoalAlarm::AIGoalAlarm()
{
}

AIGoalAlarm::~AIGoalAlarm()
{
}

void AIGoalAlarm::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIGoalPatrol)
END_CLASS_DEFAULT_FLAGS(AIGoalPatrol, AIGoal, NULL, NULL, CF_ALWAYSLOAD)

AIGoalPatrol::AIGoalPatrol()
{
}

AIGoalPatrol::~AIGoalPatrol()
{
}

void AIGoalPatrol::ReadProp(ObjectCreateStruct *pocs)
{
	super::ReadProp(pocs);
}
