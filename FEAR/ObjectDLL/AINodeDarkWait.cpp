// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeDarkWait.cpp
//
// PURPOSE : 
//
// CREATED : 8/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeDarkWait.h"
#include "AI.h"
#include "CharacterDB.h"
#include "DEditColors.h"

LINKFROM_MODULE(AINodeDarkWait);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeDarkWait 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeDarkWait CF_HIDDEN

#endif

BEGIN_CLASS(AINodeDarkWait)

	ADD_DEDIT_COLOR(AINodeDarkWait)

	// Override the base class version:

	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_STRINGPROP_FLAG(SmartObject,			"Chant",		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeDarkWait, AINodeSmartObject, CF_HIDDEN_AINodeDarkWait, AINodeDarkWaitPlugin, "TODO:CLASSDESC")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeDarkWait)
CMDMGR_END_REGISTER_CLASS(AINodeDarkWait, AINodeSmartObject)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeDarkWait::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINodeDarkWait::AINodeDarkWait()
{
}

AINodeDarkWait::~AINodeDarkWait()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeDarkWait::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINodeDarkWait
//              
//----------------------------------------------------------------------------

void AINodeDarkWait::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void AINodeDarkWait::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}
