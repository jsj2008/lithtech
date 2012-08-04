// ----------------------------------------------------------------------- //
//
// MODULE  : AIWorkingMemory.h
//
// PURPOSE : AIWorkingMemory abstract class definition.
//           AIWorking memory is a central place to store the AI's
//           observations about the world.
//           AISensors and AIGoals publish and retrieve data
//           to/from AIWorkingMemory to make decisions.
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIWORKING_MEMORY_H__
#define __AIWORKING_MEMORY_H__

#include "AIClassFactory.h"
#include "DamageTypes.h"
#include "AINodeTypes.h"
#include "AIEnumStimulusTypes.h"

// Forward declarations.

class	CAI;

// ----------------------------------------------------------------------- //

enum ENUM_AIWMFACT_TYPE
{
	kFact_InvalidType = 0,
	kFact_Character,
	kFact_Damage,
	kFact_Danger,
	kFact_Desire,
	kFact_Disturbance,
	kFact_Node,
	kFact_Object,
	kFact_PathInfo,
	kFact_Task,
	kFact_Knowledge,
};

enum ENUM_AIWMFACT_FLAG
{
	kFactFlag_None					= 0,
	kFactFlag_Scripted				= ( 1 << 0 ),
	kFactFlag_CharacterIsTraitor	= ( 1 << 1 )
};

enum ENUM_AIWMDESIRE_TYPE
{
	kDesire_InvalidType = 0,
	kDesire_Berserker,
	kDesire_Covered,
	kDesire_CounterMelee,
	kDesire_FallBack,
	kDesire_Flee,
	kDesire_GetOutOfTheWay,
	kDesire_HoldPosition,
	kDesire_Lunge,
	kDesire_NeverCloak,
	kDesire_ObtainBetterWeapon,
	kDesire_Retreat,
	kDesire_Search,
	kDesire_Stunned,
	kDesire_Uncloak,
};

enum ENUM_AIWMTASK_TYPE
{
	kTask_InvalidType = 0,
	kTask_Advance,
	kTask_Alert,
	kTask_Ambush,
	kTask_Animate,
	kTask_AnimateLoop,
	kTask_AnimateInterrupt,
	kTask_AnimateInterruptLoop,
	kTask_BlitzCharacter,
	kTask_BlitzNode,
	kTask_BlitzUseNode,
	kTask_Cover,
	kTask_DismountVehicle,
	kTask_Follow,
	kTask_Goto,
	kTask_Hide,
	kTask_Intro,
	kTask_LeadCharacter,
	kTask_LeadSearch,
	kTask_PickupWeapon,
	kTask_Menace,
	kTask_MountVehicle,
	kTask_Search,
	kTask_SuppressionFire,
	kTask_ExchangeWeapon,
};

enum ENUM_AIWMKNOWLEDGE_TYPE
{
	kKnowledge_InvalidType = 0,
	kKnowledge_AnimationOffset,
	kKnowledge_AvoidNode,
	kKnowledge_BerserkerAttachedPlayer,
	kKnowledge_BerserkerKicked,
	kKnowledge_BlockedPath,
	kKnowledge_BlockedDestination,
	kKnowledge_CombatOpportunity,
	kKnowledge_DamagedAtNode,
	kKnowledge_DoorBlocked,
	kKnowledge_DoorJammed,
	kKnowledge_EnemyKnowsPosition,
	kKnowledge_FirstDangerTime,
	kKnowledge_FlamePotPosition,
	kKnowledge_InvestigatingNavMeshPoly,
	kKnowledge_KnockedDown,
	kKnowledge_LastDisturbanceSourceCharacter,
	kKnowledge_LastKnockDownTime,
	kKnowledge_LastLongRecoilTime,
	kKnowledge_Limping,
	kKnowledge_LongRecoiling,
	kKnowledge_Lunging,
	kKnowledge_MeleeBlocked,
	kKnowledge_MeleeBlockedAttacker,
	kKnowledge_MeleeBlockSuccess,
	kKnowledge_MeleeBlockFailure,
	kKnowledge_NextCombatOpportunityTargetTime,
	kKnowledge_NextGrenadeTime,
	kKnowledge_NextLungeTime,
	kKnowledge_NextRetreatTime,
	kKnowledge_NextStatusCheckTime,
	kKnowledge_NextSuppressTime,
	kKnowledge_Ownership,
	kKnowledge_PlayerFinishingMove,
	kKnowledge_Retreating,
	kKnowledge_Shoved,
	kKnowledge_SquadTarget,
	kKnowledge_Suppressing,
	kKnowledge_StalkPosition,
	kKnowledge_Touch,
	kKnowledge_UsableCombatOpportunity,
	kKnowledge_UsableWeaponItem,
	kKnowledge_WaitForBlockedPath,
	kKnowledge_WeaponItem,
	kKnowledge_WeaponBroke,
	kKnowledge_WitnessedStimulus,
};

enum ENUM_AIWMTASK_STATUS
{
	kTaskStatus_Unset = 0,
	kTaskStatus_Set,
	kTaskStatus_Done,
};


enum ENUM_FactID { kFactID_Invalid = -1, };


// ----------------------------------------------------------------------- //

//
// STRUCTS: Working Memory Fact Data
//

// Stimulus Data.

struct SAIWMFACT_DATA_STIMULUS
{
	EnumAIStimulusType	eStimulusType;
	EnumAIStimulusID	eStimulusID;
	double				fStimulationDecreaseTime;
};

// Damage Data.

struct SAIWMFACT_DATA_DAMAGE
{
	DamageType		eDamageType;
	float			fDamageAmount;
	LTVector		vDamageDir;
};

// ----------------------------------------------------------------------- //

//
// CLASS: Working Memory Fact
//

class CAIWMFact : public CAIClassAbstract
{
	public:
		// Bits connecting the mask to the attributes.

		enum EnumFactMask
		{
			kFactMask_Invalid = -1,
			kFactMask_FactID,
			kFactMask_FactType,
			kFactMask_FactFlags,
			kFactMask_SourceObject,
			kFactMask_TargetObject,
			kFactMask_NodeType,
			kFactMask_DesireType,
			kFactMask_TaskType,
			kFactMask_KnowledgeType,
			kFactMask_Stimulus,
			kFactMask_Damage,
			kFactMask_Index,
			kFactMask_Position,
			kFactMask_Direction,
			kFactMask_Radius,
			kFactMask_Time,
			kFactMask_Count,
		};

	public:
		DECLARE_AI_FACTORY_CLASS( CAIWMFact );

		CAIWMFact();
		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Deletion.

		void	MarkForDeletion();
		bool	IsDeleted() const { return m_bDeleted; }

		// Query.

		bool	MatchesQuery( const CAIWMFact& factQuery );

		// Confidence.

		float	GetConfidence( EnumFactMask eFactMask ) const;
		void	SetConfidence( EnumFactMask eFactMask, float fConfidence );

		// Accessors.

		void	SetFactType( ENUM_AIWMFACT_TYPE eFactType, float fConfidence=1.f );
		void	SetFactID( ENUM_FactID eFactID, float fConfidence=1.f );
		void	SetFactFlags( uint32 dwFactFlags, float fConfidence=1.f );
		void	SetSourceObject( HOBJECT hSourceObject, float fConfidence=1.f );
		void	SetTargetObject( HOBJECT hTargetObject, float fConfidence=1.f );
		void	SetNodeType( EnumAINodeType	eNodeType, float fConfidence=1.f );
		void	SetDesireType( ENUM_AIWMDESIRE_TYPE eDesireType, float fConfidence=1.f );
		void	SetTaskType( ENUM_AIWMTASK_TYPE eTaskType, float fConfidence=1.f );
		void	SetKnowledgeType( ENUM_AIWMKNOWLEDGE_TYPE eKnowledgeType, float fConfidence=1.f );
		void	SetStimulus( EnumAIStimulusType	eStimulusType, EnumAIStimulusID	eStimulusID, float fConfidence=1.f );
		void	SetDamage( DamageType eDamageType, float fDamageAmount, const LTVector& vDamageDir, float fConfidence=1.f );
		void	SetIndex( int nIndex, float fConfidence=1.f );
		void	SetPos( const LTVector& vPos, float fConfidence=1.f );
		void	SetDir( const LTVector& vDir, float fConfidence=1.f );
		void	SetRadius( float fRadius, float fConfidence=1.f );
		void	SetTime( double fTime, float fConfidence=1.f );

		void	SetStimulationDecreaseTime( double fTime );
		double	GetStimulationDecreaseTime() const { return m_factStimulus.fStimulationDecreaseTime; }

		ENUM_AIWMFACT_TYPE		GetFactType() const;
		ENUM_FactID				GetFactID() const;
		uint32					GetFactFlags() const;
		HOBJECT					GetSourceObject() const;
		HOBJECT					GetTargetObject() const;
		EnumAINodeType			GetNodeType() const;
		ENUM_AIWMDESIRE_TYPE	GetDesireType() const;
		ENUM_AIWMTASK_TYPE		GetTaskType() const;
		ENUM_AIWMKNOWLEDGE_TYPE	GetKnowledgeType() const;
		void					GetStimulus( EnumAIStimulusType* peStimulusType, EnumAIStimulusID* peStimulusID ) const;
		void					GetDamage( DamageType* peDamageType, float* pfDamageAmount, LTVector* pvDamageDir ) const;
		int						GetIndex() const;
		const LTVector&			GetPos() const;
		const LTVector&			GetDir() const;
		float					GetRadius() const;
		double					GetTime() const;

		bool					IsSet( EnumFactMask eFactMask ) const { return m_bitsFactMask.test( eFactMask ); }

		void					SetUpdateTime( double fUpdateTime ) { m_fUpdateTime = fUpdateTime; }
		double					GetUpdateTime() const { return m_fUpdateTime; }

	private:
		ENUM_AIWMFACT_TYPE				m_eFactType;
		ENUM_FactID						m_eFactID;

		uint32							m_dwFactFlags;
		LTObjRef						m_hSourceObject;
		LTObjRef						m_hTargetObject;
		EnumAINodeType					m_eNodeType;
		ENUM_AIWMDESIRE_TYPE			m_eDesireType;
		ENUM_AIWMTASK_TYPE				m_eTaskType;
		ENUM_AIWMKNOWLEDGE_TYPE			m_eKnowledgeType;
		SAIWMFACT_DATA_STIMULUS			m_factStimulus;
		SAIWMFACT_DATA_DAMAGE			m_factDamage;
		int								m_nIndex;
		LTVector						m_vPos;
		LTVector						m_vDir;
		float							m_fRadius;
		double							m_fTime;

		float							m_fConfidences[kFactMask_Count];

		std::bitset<kFactMask_Count>	m_bitsFactMask;

		double							m_fUpdateTime;

		bool							m_bDeleted;
};

//
// VECTOR: List of Working Memory Facts
//

typedef std::vector< CAIWMFact*, LTAllocator<CAIWMFact*, LT_MEM_TYPE_OBJECTSHELL> >	AIWORKING_MEMORY_FACT_LIST;


// ----------------------------------------------------------------------- //

//
// CLASS: Working Memory
//

class CAIWorkingMemory : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS( CAIWorkingMemory );

		CAIWorkingMemory();
		~CAIWorkingMemory();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		void ResetWorkingMemory();

		// Create.

		CAIWMFact*			CreateWMFact( ENUM_AIWMFACT_TYPE eFactType );

		// Destroy.

		void				ClearWMFact( CAIWMFact* pFactRemove );
		void				ClearWMFact( const CAIWMFact& factQuery );
		void				ClearWMFacts( const CAIWMFact& factQuery );

		// Garbage collection.

		void				CollectGarbage();

		// Query.

		CAIWMFact*			FindWMFact( const CAIWMFact& factQuery );
		int					CountMatches( const CAIWMFact& factQuery ) const;

		CAIWMFact*			FindFactNodeMax( CAI* pAI, EnumAINodeType eNodeType, uint32 dwNodeStatusMask, HOBJECT hExclude, HOBJECT hThreat );
		CAIWMFact*			FindFactNodeRandom( CAI* pAI, EnumAINodeType eNodeType, uint32 dwNodeStatusMask, HOBJECT hExclude, HOBJECT hThreat );
		CAIWMFact*			FindFactNodeMaxNearerThreat( CAI* pAI, EnumAINodeType eNodeType, HOBJECT hExclude, HOBJECT hThreat );
		CAIWMFact*			FindFactDisturbanceMax();
		CAIWMFact*			FindFactCharacterMax(CAI* pAI);
		CAIWMFact*			FindFactCharacterNearest(CAI* pAI);
		CAIWMFact*			FindFactPlayerTurret(CAI* pAI);

		const AIWORKING_MEMORY_FACT_LIST* GetFactList() const { return m_plstWMFact; }

		void				CollectFactsUnupdated(ENUM_AIWMFACT_TYPE eFactType, AIWORKING_MEMORY_FACT_LIST* pOutFactList, double fComparisonTime);

		template <typename FactCollector_t>
		void				CollectFact(FactCollector_t& rOutCollector)
		{
			AIWORKING_MEMORY_FACT_LIST::iterator itFact;
			for( itFact = m_lstWMFacts.begin(); itFact != m_lstWMFacts.end(); ++itFact )
			{
				if( !(*itFact)->IsDeleted() )
				{
					rOutCollector(*itFact);
				}
			}
		}

	protected:

		const AIWORKING_MEMORY_FACT_LIST*	m_plstWMFact;
		AIWORKING_MEMORY_FACT_LIST			m_lstWMFacts;
		ENUM_FactID							m_eNextUnusedFactID;
		bool								m_bGarbageExists;
};

// ----------------------------------------------------------------------- //

#endif
