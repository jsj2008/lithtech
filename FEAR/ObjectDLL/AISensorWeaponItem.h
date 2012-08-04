// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorWeaponItem.h
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORWEAPONITEM_H_
#define _AISENSORWEAPONITEM_H_

LINKTO_MODULE(AISensorWeaponItem);

#include "AISensorAbstractStimulatable.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorWeaponItem
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorWeaponItem : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorWeaponItem, kSensor_WeaponItem );

	// Ctor/Dtor

	CAISensorWeaponItem();
	virtual ~CAISensorWeaponItem();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// CAISensorAbstractStimulatable members

	virtual float		GetSenseDistSqr( float fStimulusRadius );
	virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
	virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
	virtual bool		UpdateSensor();

private:

	PREVENT_OBJECT_COPYING(CAISensorWeaponItem);
};

#endif // _AISENSORWEAPONITEM_H_
