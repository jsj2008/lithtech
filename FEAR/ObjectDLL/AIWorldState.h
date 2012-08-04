// ----------------------------------------------------------------------- //
//
// MODULE  : AIWorldState.h
//
// PURPOSE : AIWorldState abstract class definition
//
// CREATED : 2/06/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIWORLD_STATE_H__
#define __AIWORLD_STATE_H__

#include "AIClassFactory.h"
#include "AIEnumNavMeshLinkTypes.h"
#include <bitset>
#include "AnimationProp.h"
#include "AINodeTypes.h"

enum ENUM_AIWorldStateEvent
{
	kWSE_Invalid
	, kWSE_Block
	, kWSE_BlockedPath
	, kWSE_Damage
	, kWSE_Defeated
	, kWSE_EnemyInFlamePot
	, kWSE_FollowerOutOfRange
	, kWSE_IncomingMeleeAttack
	, kWSE_MeleeBlocked
	, kWSE_MeleeBlockFailure
	, kWSE_MeleeBlockSuccess
	, kWSE_Shoved
	, kWSE_Surprised
	, kWSE_EnemyInPlaceForSurpriseAttack
	, kWSE_Taser1Stunned
	, kWSE_Taser2Stunned
	, kWSE_BerserkerKicked
	, kWSE_FinishingMove
	, kWSE_WeaponBroke
};

//
// ENUM: WorldState Property Keys.
//

// NOTE!!!
// This MUST be kept in sync with the g_szAIWORLDSTATE_PROP_KEY string list.
// If this changes frequently, this should be moved to a auto-header to 
// avoid maintenance issues.
enum ENUM_AIWORLDSTATE_PROP_KEY
{
	kWSK_InvalidKey = -1,
	kWSK_AnimLooped,
	kWSK_AnimPlayed,
	kWSK_AtNode,
	kWSK_AtNodeType,
	kWSK_AtTargetPos,
	kWSK_CoverStatus,
	kWSK_DisturbanceExists,
	kWSK_Idling,
	kWSK_MountedObject,
	kWSK_PositionIsValid,
	kWSK_RidingVehicle,
	kWSK_ReactedToWorldStateEvent,
	kWSK_SurveyedArea,
	kWSK_TargetIsAimingAtMe,
	kWSK_TargetIsLookingAtMe,
	kWSK_TargetIsDead,
	kWSK_TargetIsFlushedOut,
	kWSK_TargetIsSuppressed,
	kWSK_TraversedLink,
	kWSK_UsingObject,
	kWSK_WeaponArmed,
	kWSK_WeaponLoaded,

	// Count is always last.

	kWSK_Count,
};

// NOTE!!!
// This MUST be kept in sync with the ENUM_AIWORLDSTATE_PROP_KEY enum list.
// If this changes frequently, this should be moved to a auto-header to 
// avoid maintenance issues.
const char* const g_szAIWORLDSTATE_PROP_KEY[] = 
{
	"kWSK_AnimLooped",
	"kWSK_AnimPlayed",
	"kWSK_AtNode",
	"kWSK_AtNodeType",
	"kWSK_AtTargetPos",
	"kWSK_CoverStatus",
	"kWSK_DisturbanceExists",
	"kWSK_Idling",
	"kWSK_MountedObject",
	"kWSK_PositionIsValid",
	"kWSK_RidingVehicle",
	"kWSK_ReactedToWorldStateEvent",
	"kWSK_SurveyedArea",
	"kWSK_TargetIsAimingAtMe",
	"kWSK_TargetIsLookingAtMe",
	"kWSK_TargetIsDead",
	"kWSK_TargetIsFlushedOut",
	"kWSK_TargetIsSuppressed",
	"kWSK_TraversedLink",
	"kWSK_UsingObject",
	"kWSK_WeaponArmed",
	"kWSK_WeaponLoaded",
};

const char* const GetAIWorldStatePropName( ENUM_AIWORLDSTATE_PROP_KEY eProp );


//
// ENUM: WorldState Property Types.
//

enum ENUM_AIWORLDSTATE_PROP_TYPE
{
	kWST_InvalidType = -1,
	kWST_Unset,
	kWST_Variable,
	kWST_HOBJECT,
	kWST_int,
	kWST_bool,
	kWST_EnumAINodeType,
	kWST_EnumAnimProp,
	kWST_ENUM_NMLinkID,
	kWST_ENUM_AIWorldStateEvent
};


//
// STRUCT: WorldState Property
// 

struct SAIWORLDSTATE_PROP : public ILTObjRefReceiver
{
	SAIWORLDSTATE_PROP()
	{
		eWSKey = kWSK_InvalidKey;
		eWSType = kWST_InvalidType;
		nWSValue = (int)0;

		// Handle setting up the Receiver so that we get a callback when
		// hWSValue is destroyed.

		hNotifier.SetReceiver( *this );
	}

	SAIWORLDSTATE_PROP& operator=( const SAIWORLDSTATE_PROP& rOther )
	{
		// Perform a standard copy one most of the properties.

		eWSKey		= rOther.eWSKey;
		eWSType		= rOther.eWSType;
		nWSValue	= rOther.nWSValue;

		// Copy the HOBJECT only for the notifier, as we don't want to change
		// the receiver.

		hNotifier	= (HOBJECT)rOther.hNotifier;

		return *this;
	}

	// Clear the hWSValue if the object it points at is being removed.
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
	{
		LTASSERT( kWST_HOBJECT == eWSType, "HOBJECT type is expected.  Code failed to clear the Notifier when the union value/type changed." );
		if ( hObj == hWSValue )
		{
			hWSValue = NULL;
		}
	}

	ENUM_AIWORLDSTATE_PROP_KEY	eWSKey;
	ENUM_AIWORLDSTATE_PROP_TYPE eWSType;
	LTObjRefNotifier			hNotifier;

	// These should all be 4-byte data fields.
	// 
	// ALTERING THESE VALUES IS UNSAFE EXCEPT THROUGH THE 
	// CAIWorldState::SetWSProp CALLS WHICH SET THE OBJREFNOTIFIER.  DO NOT 
	// MANUALLY CHANGE THESE VALUES DIRECTLY OR CRASHES MAY OCCUR.
	// 

	union {
		ENUM_AIWORLDSTATE_PROP_KEY	eVariableWSKey;
		HOBJECT						hWSValue;
		int							nWSValue;
		bool						bWSValue;
		EnumAINodeType				eAINodeTypeWSValue;
		EnumAnimProp				eAnimPropWSValue;
		ENUM_NMLinkID				eNMLinkIDWSValue;
		ENUM_AIWorldStateEvent		eAIWorldStateEventWSValue;
	};
};

//
// VECTOR: List of WorldState Properties.
//

typedef std::vector< SAIWORLDSTATE_PROP, LTAllocator<SAIWORLDSTATE_PROP, LT_MEM_TYPE_OBJECTSHELL> > AIWORLDSTATE_PROP_LIST;
typedef std::bitset< kWSK_Count >	AIWORLDSTATE_PROP_SET_FLAGS;

// ----------------------------------------------------------------------- //

class CAIWorldState : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS( CAIWorldState );

		CAIWorldState();
		~CAIWorldState();

		// Serialization

		void				Save(ILTMessage_Write *pMsg);
		void				Load(ILTMessage_Read *pMsg);

		void				ResetWS() { m_maskPropsSet.reset(); }

		// Set.

		void				SetWSProp( SAIWORLDSTATE_PROP* pProp );
		void				SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, SAIWORLDSTATE_PROP* pProp );
		void				SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject, ENUM_AIWORLDSTATE_PROP_TYPE eType, int nValue );
		void				SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject, ENUM_AIWORLDSTATE_PROP_TYPE eType, HOBJECT hValue );
		void				SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject, ENUM_AIWORLDSTATE_PROP_TYPE eType, bool bValue );

		// Clear.

		void				ClearWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject );

		// Get.

		SAIWORLDSTATE_PROP* GetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject );
		SAIWORLDSTATE_PROP*	GetWSProp( unsigned int iProp );

		// Evaluate a prop.  This internally handles variable properties.

		SAIWORLDSTATE_PROP*	DereferenceWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey );

		// Query.

		unsigned int		GetNumWSProps() const { return m_maskPropsSet.count(); }
		bool				HasWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject );

		// Planning.

		void				CopyWorldState( CAIWorldState& wsWorldState );
		unsigned int		GetNumWorldStateDifferences( CAIWorldState& wsWorldStateB );
		unsigned int		GetNumUnsatisfiedWorldStateProps( CAIWorldState& wsWorldStateB );

		// Data Access.

		AIWORLDSTATE_PROP_SET_FLAGS*	GetWSPropSetFlags() { return &m_maskPropsSet; }

		// Debugging

		void GetDebugInfoString( std::string& sOut );

	protected:

		// Add.

		void				AddWSProp( SAIWORLDSTATE_PROP& prop );

	protected:

		SAIWORLDSTATE_PROP				m_aWSProps[kWSK_Count];
		AIWORLDSTATE_PROP_SET_FLAGS		m_maskPropsSet;
};

// ----------------------------------------------------------------------- //

#endif
