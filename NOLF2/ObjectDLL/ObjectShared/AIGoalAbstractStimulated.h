// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractStimulated.h
//
// PURPOSE : AIGoalAbstractStimulated abstract class definition
//
// CREATED : 8/20/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ABSTRACT_STIMULATED_H__
#define __AIGOAL_ABSTRACT_STIMULATED_H__

#include "AIGoalAbstract.h"
#include "LTObjRef.h"
#include "RelationChangeObserver.h"

// Forward declarations.
enum  EnumAISenseType;


// For constructor.  warning C4355: 'this' : used in base member initializer list
#pragma warning( push )
#pragma warning( disable : 4355 )

class CAIGoalAbstractStimulated : public CAIGoalAbstract, public ILTObjRefReceiver, public IRelationChangeObserver
{
	protected:

		typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Goal);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	InitGoal(CAI* pAI);
		virtual void	InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime);

		// Activation.

		virtual void ActivateGoal();

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// IRelationChangeReceiver function
		virtual int OnRelationChange(HOBJECT hObject);

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:
		enum RelationReactions
		{
			eIgnoreChange,
			eNullImportance
		};

		void SetRelationChangeResponse(RelationReactions eReaction);

	protected:

		CAIGoalAbstractStimulated();

		EnumAISenseType		m_eSenseType;
		LTObjRefNotifier	m_hStimulusSource;
		LTObjRefNotifier	m_hStimulusTarget;
		LTFLOAT				m_fStimulusTime;
		RelationReactions	m_eOnRelationChangeAction;

		// Don't Save: 
		RelationChangeNotifier m_hRelationNotifier; // Reset before set
};


#pragma warning( pop )


#endif
