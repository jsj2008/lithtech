// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeLead.cpp
//
// PURPOSE : 
//
// CREATED : 4/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeLead.h"

LINKFROM_MODULE(AINodeLead);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeLead 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeLead CF_HIDDEN

#endif

BEGIN_CLASS(AINodeLead)

	ADD_VECTORPROP_VAL_FLAG(Dims,					NODE_DIMS, NODE_DIMS, NODE_DIMS, PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")
	ADD_REALPROP_FLAG( StartWaitingRadius,			512.0f,			PF_RADIUS,		"If the distance between the AI and the character he is leading exceeds this distance, the leader should pause and wait for the follower to catch up (see ResumePathingRadius description).")
	ADD_REALPROP_FLAG( ResumePathingRadius,			200.0f,			PF_RADIUS,		"Once an AI has stopped pathing and is waiting for the following character to catch up, the AI will resume pathing once the follower is inside this radius.")
	ADD_STRINGPROP_FLAG( LeadWait,					"None",			PF_STATICLIST,	"Dialogue played when the AI starts waiting.")
	ADD_STRINGPROP_FLAG( LeadResume,				"None", 		PF_STATICLIST,	"Dialogue played when the AI resumes movement to the AINodeLead.")
		
END_CLASS_FLAGS_PLUGIN(AINodeLead, AINode, CF_HIDDEN_AINodeLead, AINodeLeadPlugin, "This object is the destination object when using the LEAD command to script an AI to lead the player to a destination.")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeLead)
CMDMGR_END_REGISTER_CLASS(AINodeLead, AINode)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeLead::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodeLead::AINodeLead()
{
}

AINodeLead::~AINodeLead()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeLead::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodeLead
//              
//----------------------------------------------------------------------------

void AINodeLead::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_INT_CAST( m_eLeadResume, EnumAISoundType );
	LOAD_INT_CAST( m_eLeadWait, EnumAISoundType );
	LOAD_FLOAT( m_flStartWaitingRadiusSqr );
	LOAD_FLOAT( m_flResumePathingRadiusSqr );
}

void AINodeLead::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_INT( m_eLeadResume );
	SAVE_INT( m_eLeadWait );
	SAVE_FLOAT( m_flStartWaitingRadiusSqr );
	SAVE_FLOAT( m_flResumePathingRadiusSqr );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeLead::ReadProp
//              
//	PURPOSE:	Handle saving and restoring the AINodeLead
//              
//----------------------------------------------------------------------------

void AINodeLead::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	m_eLeadResume = AISoundUtils::Enum( pProps->GetString( "LeadResume", "" ) );
	m_eLeadWait = AISoundUtils::Enum( pProps->GetString( "LeadWait", "" ) );

	m_flStartWaitingRadiusSqr = pProps->GetReal( "StartWaitingRadius", FLT_MAX );
	m_flStartWaitingRadiusSqr *= m_flStartWaitingRadiusSqr;

	m_flResumePathingRadiusSqr = pProps->GetReal( "ResumePathingRadius", FLT_MAX );
	m_flResumePathingRadiusSqr *= m_flResumePathingRadiusSqr;

	AIASSERT1( m_flStartWaitingRadiusSqr > m_flResumePathingRadiusSqr, m_hObject,
		"Level design issue: StartWaitingRadius is greater than the ResumePathingRadius.  This will result in the AI starting to wait then starting to repath the next frame.", GetNodeName() );
}

float AINodeLead::GetStartWaitingRadiusSqr() const
{
	return m_flStartWaitingRadiusSqr;
}

float AINodeLead::GetResumePathingRadiusSqr() const
{
	return m_flResumePathingRadiusSqr;
}

EnumAISoundType AINodeLead::GetAISoundType_LeadWait() const
{
	return m_eLeadWait;
}

EnumAISoundType AINodeLead::GetAISoundType_LeadResume() const
{
	return m_eLeadResume;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

LTRESULT AINodeLeadPlugin::PreHook_EditStringList( const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength )
{
	if ( LTStrIEquals( szPropName, "LeadResume" ) )
	{
		AISoundUtils::AddToStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength, "Lead" );
		return LT_OK;
	}

	if ( LTStrIEquals( szPropName, "LeadWait" ) )
	{
		AISoundUtils::AddToStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength, "Lead" );
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
