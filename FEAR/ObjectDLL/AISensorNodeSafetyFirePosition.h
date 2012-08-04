// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeSafetyFirePosition.h
//
// PURPOSE : This sensor handles finding SafetyFirePosition nodes.  AIs 
//			can only detect SafetyFirePosition nodes belonging to the 
//			AINodeSafety instance the AI is currently at.
//
// CREATED : 2/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORNODESAFETYFIREPOSITION_H_
#define _AISENSORNODESAFETYFIREPOSITION_H_


#include "AISensorNode.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorNodeSafetyFirePosition
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorNodeSafetyFirePosition : public CAISensorNode
{
	typedef CAISensorNode super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeSafetyFirePosition, kSensor_NodeSafetyFirePosition );

	// Ctor/Dtor

	CAISensorNodeSafetyFirePosition();
	virtual ~CAISensorNodeSafetyFirePosition();

	// CAISensorAbstract members.

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);
	virtual bool	UpdateSensor();

private:
	PREVENT_OBJECT_COPYING(CAISensorNodeSafetyFirePosition);

	// This handle stores the HOBJECT of the AINodeSafety instances 
	// AINodeSafetyFirePositions currently in memory.

	LTObjRef		m_hSafetyNodeCacheSource;
};

#endif // _AISENSORNODESAFETYFIREPOSITION_H_

