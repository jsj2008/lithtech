// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityAbstract.h
//
// PURPOSE : AIActivityAbstract abstract class definition
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIVITY_ABSTRACT_H__
#define __AIACTIVITY_ABSTRACT_H__

#include "AIClassFactory.h"


//
// ENUM: Types of activities.
//
enum EnumAIActivityType
{
	kActivity_InvalidType= -1,
	#define ACTIVITY_TYPE_AS_ENUM 1
	#include "AIEnumActivityTypes.h"
	#undef ACTIVITY_TYPE_AS_ENUM

	kActivity_Count,
};

//
// STRINGS: const strings for activity types.
//
static const char* s_aszActivityTypes[] =
{
	#define ACTIVITY_TYPE_AS_STRING 1
	#include "AIEnumActivityTypes.h"
	#undef ACTIVITY_TYPE_AS_STRING
};

typedef std::bitset<kActivity_Count> AIActivityBitSet;
enum	ENUM_AIActivitySet { kAIActivitySet_Invalid = -1, };


// Forward declarations.

class	CAISquad;

#define MAX_PARTICIPANTS	20

// ----------------------------------------------------------------------- //

enum ENUM_AIACTIVITY_STATUS
{
	kActStatus_Invalid = -1,
	kActStatus_Advancing,
	kActStatus_Complete,
	kActStatus_Failed,
	kActStatus_Initialized,
	kActStatus_Searching,
	kActStatus_Suppressing,
	kActStatus_Updating,
};

// ----------------------------------------------------------------------- //

class CAIActivityAbstract : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( Activity );

		CAIActivityAbstract();
		virtual ~CAIActivityAbstract();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	InitActivity();
		void			HookParent(CAISquad* pSquad) {m_pSquad = pSquad;}

		uint32			GetNumPotentialParticipants() const { return m_cPotentialParticipants; }
		virtual bool	IsActivityRelevant();
		virtual bool	FindActivityParticipants() { return false; }

		virtual bool	ActivateActivity();
		virtual void	DeactivateActivity();
		virtual bool	UpdateActivity();
		
		virtual void	ClearDeadAI();

		void			CalcActivityAABB( LTRect3f* pAABB );

		int				GetActivityPriority() const { return m_nActivityPriority; }
		double			GetNextActivityUpdateTime() const { return m_fNextActivityUpdateTime; }

	protected:

		ENUM_AIACTIVITY_STATUS	m_eActStatus;

		int						m_nActivityPriority;

		float					m_fActivityUpdateRate;
		double					m_fNextActivityUpdateTime;
		double					m_fActivityActivateTime;
		double					m_fActivityTimeOut;
		double					m_fActivityExpirationTime;

		CAISquad*				m_pSquad;
		uint32					m_cPotentialParticipants;
		LTObjRef				m_aPotentialParticipants[MAX_PARTICIPANTS];
};


#endif
