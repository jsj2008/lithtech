// ----------------------------------------------------------------------- //
//
// MODULE  : AIWorkingMemoryCentral.h
//
// PURPOSE : Special AIWorkingMemory derived class which is shared between
//			all AIs for coordination reasons.  Reasons for using this
//			include:
//
//			1) Preventing events from happening too frequently -- player 
//				experience engineering.
//			2) Coordinating ai activitities.
//
// CREATED : 12/5/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //


#ifndef __AIWORKING_MEMORY_CENTRAL_H__
#define __AIWORKING_MEMORY_CENTRAL_H__

#include "AIWorkingMemory.h"

class CAIWorkingMemoryCentral : public CAIWorkingMemory
{
public:
	void Init();
	void Term();

	void OnAIDebugCmd( HOBJECT hSender, const CParsedMsg &crParsedMsg );
};

extern CAIWorkingMemoryCentral* g_pAIWorkingMemoryCentral;

#endif// __AIWORKING_MEMORY_CENTRAL_H__
