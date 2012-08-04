// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeDisturbance.h
//
// PURPOSE : AINodeDisturbance class definition
//
// CREATED : 5/30/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_DISTURBANCE_H_
#define _AI_NODE_DISTURBANCE_H_

#include "AINode.h"

LINKTO_MODULE( AINodeDisturbance );

// Forward declarations.

enum EnumAIStimulusID;


class AINodeDisturbance : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeDisturbance();
		virtual ~AINodeDisturbance();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pocs);
		virtual bool OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		
		// Type

		EnumAINodeType GetType() { return kNode_Disturbance; }

	protected:

		uint32				m_nAlarmLevel;
		LTFLOAT				m_fDuration;
		EnumAIStimulusID	m_eStimID;
};

//-----------------------------------------------------------------

#endif // _AI_NODE_DISTURBANCE_H_
