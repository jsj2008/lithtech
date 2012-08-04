// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstract.h
//
// PURPOSE : AIGoalAbstract abstract class definition
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ABSTRACT_H__
#define __AIGOAL_ABSTRACT_H__

#include "AIClassFactory.h"
#include "UberAssert.h"

// Forward Declarations.
class CAI;
class CAIGoalMgr;
class AINode;
class AIVolume;
class CParsedMsg;
struct AISenseRecord;
struct UnblockNode;
struct BlockNode;
struct DamageStruct;
enum   EnumAISoundType;


//
// ENUM: Types of goals.
//
enum EnumAIGoalType
{
	kGoal_InvalidType= -1,
	#define GOAL_TYPE_AS_ENUM 1
	#include "AIGoalTypeEnums.h"
	#undef GOAL_TYPE_AS_ENUM

	kGoal_Count,
};

//
// STRINGS: const strings for goal types.
//
static const char* s_aszGoalTypes[] =
{
	#define GOAL_TYPE_AS_STRING 1
	#include "AIGOALTypeEnums.h"
	#undef GOAL_TYPE_AS_STRING
};


//
// CLASS: Instance of an AI's goal.
//
class CAIGoalAbstract : public CAIClassAbstract
{
	friend struct UnblockNode;
	friend struct BlockNode;

	typedef CAIClassAbstract super;

	public :

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Goal);

		CAIGoalAbstract( );
		virtual ~CAIGoalAbstract( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	InitGoal(CAI* pAI);
		virtual void	InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime);
		LTFLOAT			UpdateGoalTimer(LTFLOAT fTime);

		// SenseTriggers and Attractors.

		virtual LTBOOL	HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);
		virtual AINode*	HandleGoalAttractors();
		virtual AINode* FindGoalAttractors(LTBOOL bRequiresOwner, HOBJECT hOwner);

		// Triggered AI Sounds.

		virtual LTBOOL SelectTriggeredAISound(EnumAISoundType* peAISoundType);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void	UpdateGoal();
		virtual void	RecalcImportance();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage) { return LTFALSE; }
		virtual HMODELANIM GetAlternateDeathAnimation();

		// Volume Handling.

		virtual void HandleVolumeEnter(AIVolume* pVolume) {}
		virtual void HandleVolumeExit(AIVolume* pVolume) {}

		// Command Handling.

		bool HandleCommand(const CParsedMsg &cMsg);
		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue) { return LTFALSE; }

		// Scripting.

		void		SetScripted(LTBOOL bScripted) { m_bScripted = bScripted; }
		LTBOOL		IsScripted() { return m_bScripted; }

		// Data Access.

		LTFLOAT		GetCurImportance() const { return m_fCurImportance; }
		void		SetBaseImportance(LTFLOAT fImportance);
		void		SetCurImportance(LTFLOAT fImportance) { m_fCurImportance = fImportance; }
		void		SetCurToBaseImportance();
		void		SetRandomNextUpdate();
		LTFLOAT		GetLastUpdateTime() const { return m_fLastUpdateTime; }
		void		SetLastUpdateTime(LTFLOAT fTime) { m_fLastUpdateTime = fTime; }
		LTFLOAT		GetNextUpdateTime() const { return m_fNextUpdateTime; }
		void		SetNextUpdateTime(LTFLOAT fTime) { m_fNextUpdateTime = fTime; }
		LTBOOL		DoesFreezeDecay() const { return m_bFreezeDecay; }
		LTBOOL		IsLockedAnimInterruptable() const { return m_bLockedAnimIsInterruptable; }
		LTBOOL		DoesForceAnimInterrupt() const { return m_bForceAnimInterrupt; }
		void		SetDeleteGoalNextUpdate(LTBOOL bDeleteGoal) { m_bDeleteGoalNextUpdate = bDeleteGoal; }
		LTBOOL		GetDeleteGoalNextUpdate() const { return m_bDeleteGoalNextUpdate; }
		LTBOOL		RequiresImmediateResponse() const { return m_bRequiresImmediateResponse; }
		LTBOOL		RequiresUpdates() const { return m_bRequiresUpdates; }
		void		ClearImmediateResponse() { m_bRequiresImmediateResponse = LTFALSE; }
		void		SetPermanentGoal(LTBOOL b) { m_bPermanentGoal = b; }
		LTBOOL		IsPermanentGoal() const { return m_bPermanentGoal; }

		void		AddInvalidNode(HOBJECT hNode);
		bool		IsPathToNodeValid(AINode* pNode);
		void		UpdateInvalidNodeList();


	protected:

		// Attractor node search blocking.

		void			BlockAttractorNodeFromSearch(HOBJECT hNode);
		void			UnblockAttractorNodeFromSearch(HOBJECT hNode);

	protected:

		CAI*				m_pAI;							// Ptr to AI owner of goal.
		CAIGoalMgr*			m_pGoalMgr;						// Ptr to goal's manager.
		LTFLOAT				m_fBaseImportance;				// Original importance, set in Brain.
		LTFLOAT				m_fCurImportance;				// Current Importance.
		LTFLOAT				m_fLastCurToBaseTime;			// Time of last reset of importance to base.
		LTFLOAT				m_fLastUpdateTime;				// Time of last update.
		LTFLOAT				m_fNextUpdateTime;				// Time of next update.
		LTFLOAT				m_fUpdateTimer;					// Timer counting up to next update.
		LTFLOAT				m_fDecayRate;					// Rate AI loses interest.
		LTBOOL				m_bFreezeDecay;					// Freeze other goals' decays when TRUE.
		LTBOOL				m_bLockedAnimIsInterruptable;	// Locked anim can be cut off when TRUE.
		LTBOOL				m_bForceAnimInterrupt;			// This goal can immediately interrupt another goal's locked anim when TRUE.
		LTObjRef			m_hBlockedAttractorNode;		// AI Node blocked from attractor search.
		LTBOOL				m_bScripted;					// Is this goal part of a script?
		LTBOOL				m_bDeleteGoalNextUpdate;		// Flag for deletion next update.
		LTBOOL				m_bRequiresImmediateResponse;	// Goal reqires an immediate response from the GoalMgr.
		LTBOOL				m_bRequiresUpdates;				// Goal reqires updatesfrom the GoalMgr.
		LTBOOL				m_bPermanentGoal;				// Permanent goals cannot be removed.

		// NOTE:  These BOOLS should really be enumerated flags.
};


//----------------------------------------------------------------------------
//              
//	STRUCT:		INVALID_NODE
//              
//	PURPOSE:	Reference to an invalid node, with a timestamp and a locked
//				state
//              
//----------------------------------------------------------------------------
struct INVALID_NODE
{
public:
	INVALID_NODE();

	bool operator==(const LTObjRef& rhs );
	INVALID_NODE(const INVALID_NODE& rhs);
	INVALID_NODE& operator=(const INVALID_NODE& rhs );

	float		m_flTime;
	LTObjRef	m_hNode;
	BOOL		m_bBlocked;
};

//----------------------------------------------------------------------------
//              
//	STRUCT:		AutoNodeBlocker
//              
//	PURPOSE:	Helper class to automaticly add and remove nodes from the blocked
//				node set in searches
//              
//----------------------------------------------------------------------------
class AutoNodeBlocker
{
public:
	AutoNodeBlocker( HOBJECT hBlocker, std::vector<INVALID_NODE*>* pList );
	~AutoNodeBlocker();

private:
	// Copy Constructor and Asignment Operator private to prevent 
	// automatic generation and inappropriate, unintentional use
	AutoNodeBlocker(const AutoNodeBlocker& rhs) {}
	AutoNodeBlocker& operator=(const AutoNodeBlocker& rhs ) {}

	// Don't save:
	std::vector<INVALID_NODE*>* m_plistInvalidNodes;
	LTObjRef		m_hBlocker;
};

#endif
