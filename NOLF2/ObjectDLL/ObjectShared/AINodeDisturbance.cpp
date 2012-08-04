// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeDisturbance.cpp
//
// PURPOSE : AINodeDisturbance implementation
//
// CREATED : 5/30/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeDisturbance.h"
#include "ParsedMsg.h"
#include "RelationButeMgr.h"
#include "AIStimulusMgr.h"

LINKFROM_MODULE( AINodeDisturbance );

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeDisturbance)

	ADD_BOOLPROP_FLAG(Face,				LTTRUE,			0|PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Alignment,		"None", 		0|PF_STATICLIST|PF_HIDDEN)

	ADD_REALPROP_FLAG(Radius,			128.0f,			0|PF_RADIUS)
	ADD_LONGINTPROP_FLAG(AlarmLevel,	0,				0)
	ADD_REALPROP_FLAG(Duration,			10.0f,			0)

END_CLASS_DEFAULT(AINodeDisturbance, AINode, NULL, NULL)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeDisturbance)

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

CMDMGR_END_REGISTER_CLASS(AINodeDisturbance, AINode)

// ----------------------------------------------------------------------- //

AINodeDisturbance::AINodeDisturbance()
{
	m_nAlarmLevel = 0;
	m_fDuration = 0.f;
	m_eStimID = kStimID_Unset;
}

AINodeDisturbance::~AINodeDisturbance()
{
}

// ----------------------------------------------------------------------- //

void AINodeDisturbance::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

	// AINode read Radius.

	// Read AlarmLevel.

    if( g_pLTServer->GetPropGeneric( "AlarmLevel", &g_gp ) == LT_OK )
	{
		if( g_gp.m_String[0] )
		{
			m_nAlarmLevel = (uint32)g_gp.m_Long;
		}
	}

	// Read duration.

    if( g_pLTServer->GetPropGeneric( "Duration", &g_gp ) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_fDuration = g_gp.m_Float;
		}
	}
}

// ----------------------------------------------------------------------- //

void AINodeDisturbance::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_INT( m_nAlarmLevel );
	SAVE_FLOAT( m_fDuration );
	SAVE_DWORD( m_eStimID );
}

void AINodeDisturbance::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_INT( m_nAlarmLevel );
	LOAD_FLOAT( m_fDuration );
	LOAD_DWORD_CAST( m_eStimID, EnumAIStimulusID );
}

// ----------------------------------------------------------------------- //

bool AINodeDisturbance::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	// Turn On stimulus.

	if( cMsg.GetArg(0) == s_cTok_On )
	{
		// Remove old stimulus.

		if( m_eStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eStimID );
		}

		// Force all disturbances to be aligned with GoodCharacters, since they
		// will be disturbances caused by the player.

		// Re-use the AIButeMgr name for the AI instances (trying to use it as a
		// more consistant 'access key' for the AI.
		CDataUser du;
		du.InitData( "GoodCharacter" , m_hObject );

		m_eStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyDisturbanceSound, m_nAlarmLevel, m_hObject, LTNULL, du.GetData(), m_vPos, m_fRadius, m_fDuration );
	}

	// Turn Off stimulus.

	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		if( m_eStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eStimID );
			m_eStimID = kStimID_Unset;
		}
	}

	return super::OnTrigger( hSender, cMsg );
}

// ----------------------------------------------------------------------- //
