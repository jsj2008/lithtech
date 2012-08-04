// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeIntro.cpp
//
// PURPOSE : AINodeIntro class implementation
//
// CREATED : 02/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeIntro.h"

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINODEINTRO CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINODEINTRO 0

#endif

BEGIN_CLASS(AINodeIntro)

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(ReactivationTime,			0.f,			PF_HIDDEN, "TODO:PROPDESC")
	ADD_BOOLPROP_FLAG(OneWay,					false,			PF_HIDDEN, "TODO:PROPDESC")

	// Add Intro properties.

	ADD_REALPROP_FLAG(PauseTime,				0.0f,			0, "The AI will pause for the specified amount of time after playing a non-looping animation [seconds]")

	// Override the base class version:

	ADD_STRINGPROP_FLAG(SmartObject,			"None",			0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeIntro, AINodeSmartObject, CF_HIDDEN_AINODEINTRO, AINodeIntroPlugin, "When set as an AI's InitialNode in ConfigureAI, the AI walks to this node and plays the animation specified by the SmartObject")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeIntro)
CMDMGR_END_REGISTER_CLASS(AINodeIntro, AINodeSmartObject)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeIntro::Con/destructor
//
//	PURPOSE:	Con/destruct the object.
//
// ----------------------------------------------------------------------- //

AINodeIntro::AINodeIntro()
{
	m_fPauseTime = 0.f;
}

AINodeIntro::~AINodeIntro()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeIntro::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINodeIntro::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_fPauseTime = pProps->GetReal( "PauseTime", (float)m_fPauseTime );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeIntro::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINode
//              
//----------------------------------------------------------------------------

void AINodeIntro::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_DOUBLE(m_fPauseTime);
}

void AINodeIntro::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_DOUBLE(m_fPauseTime);
}

