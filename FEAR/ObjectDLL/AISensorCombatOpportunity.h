// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCombatOpportunity.h
//
// PURPOSE : This sensor handles sensing AICombatOpportunity objects, and 
//			 translating them into CAIWMFacts the AI can reason about.  
//
//			As the AI discovers AINodeCombatOpportunity and 
//			AINodeCombatOpportunityView independantly, this sensor only
//			senses AICombatOpportunity objects which have target objects,
//			as these are the only type the AI shoots.
//
//			To the same end, the sensor purges any facts about 
//			AICombatOpportunity objects with NULL targets, as these
//			are obsolete.
//
// CREATED : 6/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORCOMBATOPPORTUNITY_H_
#define _AISENSORCOMBATOPPORTUNITY_H_

LINKTO_MODULE(AISensorCombatOpportunity);


#include "AISensorAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorCombatOpportunity
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorCombatOpportunity : public CAISensorAbstract
{
	typedef CAISensorAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCombatOpportunity, kSensor_CombatOpportunity );

	// Ctor/Dtor

	CAISensorCombatOpportunity();
	virtual ~CAISensorCombatOpportunity();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	// CAISensorAbstract members.
	virtual bool	StimulateSensor( CAIStimulusRecord* pStimulusRecord );
	virtual bool	UpdateSensor();

private:

	void			SetCombatOpportunityTimeout( float fDelay );

private:
	PREVENT_OBJECT_COPYING(CAISensorCombatOpportunity);
};

#endif // _AISENSORCOMBATOPPORTUNITY_H_
