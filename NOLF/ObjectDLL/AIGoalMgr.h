// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIGOAL_H__
#define __AIGOAL_H__

#include "ltengineobjects.h"
#include "AITypes.h"

class AIGoal : public BaseClass
{
	typedef BaseClass super;

	public :

		enum Type
		{
			eTypeInvalid,
			eTypeAttack,
			eTypeInvestigate,
			eTypeAlarm,
			eTypePatrol,
		};

	public :

		// Ctors/Dtors/etc

		AIGoal();
		virtual ~AIGoal();

		// Engine

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void ReadProp(ObjectCreateStruct *pData);

		// Type

		virtual Type GetType() { return eTypeInvalid; }

	protected :

		HSTRING			m_hstrName;
};

class AIGoalAttack : public AIGoal
{
	typedef AIGoal super;

	public :

		// Ctors/Dtors/etc

		AIGoalAttack();
		virtual ~AIGoalAttack();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

	protected :
};

class AIGoalInvestigate : public AIGoal
{
	typedef AIGoal super;

	public :

		// Ctors/Dtors/etc

		AIGoalInvestigate();
		virtual ~AIGoalInvestigate();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

	protected :
};

class AIGoalAlarm : public AIGoal
{
	typedef AIGoal super;

	public :

		// Ctors/Dtors/etc

		AIGoalAlarm();
		virtual ~AIGoalAlarm();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

	protected :
};

class AIGoalPatrol : public AIGoal
{
	typedef AIGoal super;

	public :

		// Ctors/Dtors/etc

		AIGoalPatrol();
		virtual ~AIGoalPatrol();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

	protected :
};

#endif