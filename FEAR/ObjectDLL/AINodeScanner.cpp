// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeScanner.cpp
//
// PURPOSE : AINodeScanner class implementation
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeScanner.h"
#include "DEditColors.h"

LINKFROM_MODULE( AINodeScanner );

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINODESCANNER CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINODESCANNER 0

#endif

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeScanner)

	ADD_DEDIT_COLOR( AINodePatrol )
	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")

	ADD_REALPROP_FLAG(PauseTime,			0.0f,					0, "Time scanner pauses at node. [Seconds]")
	ADD_BOOLPROP_FLAG(DefaultNode,			false,					0, "If true the scanner comes to rest at this node.")
	ADD_COMMANDPROP_FLAG(ShutDownCommand,	"",						0|PF_NOTIFYCHANGE, "Command run when scanner reaches its default position.")

	// Hide SmartObject properties.

	ADD_STRINGPROP_FLAG(SmartObject,			"None", 		PF_HIDDEN|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(Command,				"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")

END_CLASS_FLAGS(AINodeScanner, AINodePatrol, CF_HIDDEN_AINODESCANNER, "This is a special AINodePatrol that defines a place to pause and look around.")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeScanner)
CMDMGR_END_REGISTER_CLASS(AINodeScanner, AINodePatrol)

AINodeScanner::AINodeScanner()
{
	m_bDefaultNode = false;
	m_fPauseTime = 0.f;
}

AINodeScanner::~AINodeScanner()
{
}

void AINodeScanner::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_bDefaultNode = pProps->GetBool( "DefaultNode", m_bDefaultNode );

	m_fPauseTime = pProps->GetReal( "PauseTime", (float)m_fPauseTime );

	const char* pszPropString = pProps->GetCommand( "ShutDownCommand", "" );
	if( pszPropString[0] )
	{
		m_strPatrolCommand = pszPropString;
	}
}

void AINodeScanner::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_bool( m_bDefaultNode );
	SAVE_DOUBLE( m_fPauseTime );
}

void AINodeScanner::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_bool( m_bDefaultNode );
	LOAD_DOUBLE( m_fPauseTime );
}
