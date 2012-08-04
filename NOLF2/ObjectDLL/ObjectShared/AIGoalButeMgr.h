// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalButeMgr.h
//
// PURPOSE : Read templates for Goals.
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOALBUTE_MGR_H__
#define __AIGOALBUTE_MGR_H__

#include "GameButeMgr.h"

#include <list>

// Forward declarations.
class  CAIGoalButeMgr;
struct AIGBM_GoalSet;
struct AIGBM_SmartObjectTemplate;
enum EnumAIGoalType;
enum EnumAINodeType;
enum EnumAIStateType;

// Globals.
extern CAIGoalButeMgr* g_pAIGoalButeMgr;


//
// MACROS:
//
#define MAX_GOAL_NAME_LEN	32
#define GOAL_FILE			"Attributes\\AIGoals.txt"


//
// STRUCT: Goal Set. 
//
struct SGoalSetData
{
	HSTRING hstrParams;
};

typedef std::map<EnumAIGoalType, SGoalSetData> AIGOAL_DATA_MAP;
typedef std::vector<AIGBM_GoalSet*> AIGOAL_SET_LIST;
typedef std::vector<int> AIBRAIN_LIST;

struct AIGBM_GoalSet
{
	enum {
		kGS_None = 0,
		kGS_Permanent,
		kGS_Hidden,
	};

	AIGBM_GoalSet() { szName[0] = LTNULL; dwGoalSetFlags = kGS_None; }

	char			szName[MAX_GOAL_NAME_LEN];
	uint32			dwGoalSetFlags;
	AIGOAL_DATA_MAP	mapGoalSet;
	AIGOAL_SET_LIST lstIncludeGoalSets;
	AIBRAIN_LIST	lstRequiredBrains;
};


//
// STRUCT: Goal Template. 
//
struct AIGBM_GoalTemplate
{
	AIGBM_GoalTemplate();

	LTFLOAT fImportance;			// Base priority of goal (decays over time).
	LTFLOAT	fDecayTime;				// Amount of time for AI to lose all interest.
	LTBOOL	bFreezeDecay;			// If TRUE, other goes do not decay when this goal is active.
	LTBOOL	bLockedAnimIsInterruptable;// If TRUE, another goal can activate and cut off this goal's locked anim.
	LTBOOL	bForceAnimInterrupt;	// If TRUE, this goal can immediately interrupt another goal, even when a locked anim is playing.
	LTFLOAT	fUpdateRate;			// Rate to update Importance.
	uint32	flagSenseTriggers;		// Senses that trigger Goal.
	uint32	cAttractors;			// Number of attractors that activate Goal.
	EnumAINodeType* aAttractors;	// Attractors that activate Goal.
	LTFLOAT	fAttractorDistSqr;		// Squared distance from AI to search for Attractors.
	LTBOOL	bDeleteOnDeactivation;	// If true, goal is deleted after it deactivates.
	LTFLOAT fChanceToActivate;		// % chance of activating when criteria met.
	LTFLOAT fFrequencyMin;			// Min seconds until can activate again.
	LTFLOAT fFrequencyMax;			// Max seconds until can activate again.
	uint32	nDamagePriority;		// Priority of the goal when/if handling damage.
};


//
// STRUCT: SmartObject Template. 
//
typedef std::map<EnumAINodeType, HSTRING> SMART_OBJECT_CMD_MAP;
typedef std::vector<AIGBM_SmartObjectTemplate*> SMART_OBJECT_LIST;
typedef std::multimap<EnumAIStateType, EnumAINodeType> SMART_OBJECT_ACTIVE_CMD_MAP;

struct AIChildModelInfo 
{
	HMODELDB hmodeldb ;
	std::string  sFilename ;
	
	// ctor/dtor
	AIChildModelInfo( HMODELDB mdb , char const* pszFilename ):hmodeldb(mdb), sFilename(pszFilename) {}
	AIChildModelInfo(  ):hmodeldb(0) {}
};


struct AIGBM_SmartObjectTemplate
{
	char					szName[MAX_GOAL_NAME_LEN];
	uint32					nID;
	SMART_OBJECT_CMD_MAP	mapCmds;

	SMART_OBJECT_ACTIVE_CMD_MAP	mapActiveCmds;

	// TERRYF
	// Add childmodel names here.
	// list of child models associated with this smart object.
	typedef std::list<AIChildModelInfo> AIChldMdlInfoList ;
	typedef std::list<AIChildModelInfo>::iterator AIChldMdlInfoItr ;

	AIChldMdlInfoList    addchildmodels ;
};


//
// CLASS: AIGoalButeMgr stores data-driven goal info. 
//
class CAIGoalButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CAIGoalButeMgr();
		~CAIGoalButeMgr();

        LTBOOL	Init(const char* szAttributeFile = GOAL_FILE);
		void	Term();

		// Templates

		AIGBM_GoalTemplate* GetTemplate(const EnumAIGoalType eGoalType) { return &m_aTemplates[eGoalType]; }

		uint32				GetNumGoalSets() const { return m_lstGoalSets.size(); }
		uint32				GetGoalSetIndex(const char* szGoalSetName);
		AIGBM_GoalSet*		GetGoalSet(const uint32 iGoalSet) { return m_lstGoalSets[iGoalSet]; }
		AIGBM_GoalSet*		GetGoalSet(const char* szGoalSetName);

		uint32						GetNumSmartObjectTemplates() const { return m_lstSmartObjects.size(); }
		AIGBM_SmartObjectTemplate*	GetSmartObjectTemplate(const uint32 nID) 
									{	return nID < m_lstSmartObjects.size( ) ? m_lstSmartObjects[nID] : NULL; }
		AIGBM_SmartObjectTemplate*	GetSmartObjectTemplate(const char* szSmartObjectName);

	protected:

		void				ReadGoalSet();
		void				ReadGoalTemplate(uint32 iTemplate);
		void				ReadSmartObjectTemplate(uint32 nID);

		EnumAIGoalType		ConvertToGoalTypeEnum(char* szGoalType);

		void				GetBitFlagItems(uint32* flags, const char* szAttribute, const uint32 nNumFlags, 
											const char** aszFlags);
		void				GetEnumItems(uint32*& aItems, uint32& cItems, const char* szAttribute, const uint32 nNumEnums, 
										  const char** aszEnums);

	private :

		AIGBM_GoalTemplate*	m_aTemplates;
		AIGOAL_SET_LIST		m_lstGoalSets;
		SMART_OBJECT_LIST	m_lstSmartObjects;
};

#endif // __AIGOALBUTE_MGR_H__
