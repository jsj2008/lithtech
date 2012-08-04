// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTraitor.h
//
// PURPOSE : This sensor handles detecting 'traitors'.  Traitors are AIs 
//			 who deal damage to AIs they like.
//
// CREATED : 3/15/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORTRAITOR_H_
#define _AISENSORTRAITOR_H_

#include "AISensorAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorTraitor
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorTraitor : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	// Helper class for storing a collection of information about friendly
	// fire incidents.

	struct FriendlyFireRecord
	{
		FriendlyFireRecord()
		{
			Clear();
		}

		void Clear()
		{
			m_hDamager			= NULL;
			m_flTotalDamage		= 0.0f;
			m_flLastDamageTime	= -DBL_MAX;
		}

		LTObjRef	m_hDamager;
		float		m_flTotalDamage;
		double		m_flLastDamageTime;
	};

	enum kConst
	{
		// Arbitrarily selected number of enemies to be tracked, based on the
		// number of hits an AI can likely sustain before dying.  This number 
		// can be modified as needed.

		kMaxPotentialTraitorsTracked = 4,
	};

public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTraitor, kSensor_Traitor );

	// Ctor/Dtor

	CAISensorTraitor();
	virtual ~CAISensorTraitor();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Handle input stimulation from the Stimulus System.  This 

	virtual void	InitSensor( EnumAISensorType eSensorType, CAI* pAI );
	virtual bool	StimulateSensor( CAIStimulusRecord* pStimulusRecord );
	virtual bool	UpdateSensor();

private:
	PREVENT_OBJECT_COPYING(CAISensorTraitor);

	int					GetFireRecordIndex( HOBJECT hDamager );

	// Percent damage an AI must sustain from an ally before the AI decides the 
	// ally is a traitor.
	float				m_flDamageThreshold;

	// History of friendly fire incidents.  The max specified is based on the 
	// number of hits an AI is likely to sustain before dying.  This number 
	// can be raised or lowered if we discover the AI needs to track more 
	// targets.
	FriendlyFireRecord	m_aFriendlyFireHistory[kMaxPotentialTraitorsTracked];
	
	// Object that the AI has decided is a traitor.  The AI can only track a 
	// single traitor at a time.  This restricts the extra intersect tests 
	// this sensor causes per frame to 1 (used for determining visibility to
	// the traitor)
	LTObjRef			m_hTraitorCharacter;
};

#endif // _AISENSORTRAITOR_H_
