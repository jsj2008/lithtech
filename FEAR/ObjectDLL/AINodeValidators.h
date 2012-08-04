// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeValidators.h
//
// PURPOSE : This file contains common tests used during in IsNodeValid 
//			 calls.  Under the current system, these tests are duplicated
//			 many times, making maintenance more difficult.  This is an 
//			 attempt at avoiding such duplication.
//
// CREATED : 9/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODEVALIDATORS_H_
#define _AINODEVALIDATORS_H_

#include "AIWorkingMemory.h"
#include "AINodeTypes.h"

// Forward declarations.

class AINode;

// ----------------------------------------------------------------------- //

// The boundary radius represents the 'outer' radius for a node.  The threat
// must be inside this radius for this test to succeed.

#define BOUNDARYRADIUS_PROPS( radius ) \
		ADD_REALPROP_FLAG(BoundaryRadius,			radius,		0|PF_RADIUS|PF_FOVFARZ, "If the AI's threat exits this radius, it will invalidate the node. [WorldEdit units]") \
		ADD_STRINGPROP_FLAG(BoundaryRegion,			"",			0|PF_OBJECTLINK, "Alternative to boundary radius. If an AIRegion is specified, the radius is ignored.")

class AINodeValidatorBoundaryRadius
{
public:
	AINodeValidatorBoundaryRadius();

	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	void InitNodeValidator( AINode* pNode );

	bool Evaluate( AINode* pNode, uint32 dwFilteredStatusFlags, const LTVector& vThreatPos, ENUM_NMPolyID eThreatNMPoly ) const;

	float GetBoundaryRadiusSqr() const { return m_fBoundaryRadiusSqr; }
	ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_eBoundaryAIRegion; }

private:
	float			m_fBoundaryRadiusSqr;
	ENUM_AIRegionID	m_eBoundaryAIRegion;
	std::string		m_strBoundaryAIRegion;
};

// ----------------------------------------------------------------------- //

// This test represents a simple FOV.  The targets position must be inside 
// this FOV for this test to succeed.

#define THREATFOV_PROPS() \
	ADD_REALPROP_FLAG(Fov,						60.0f,			0|PF_CONEFOV, "The AI's threat must be within this FOV of the forward vector of the node for it to be valid. [Degrees]")

class AINodeValidatorThreatFOV
{
public:
	AINodeValidatorThreatFOV();

	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, const LTVector& vThreatPos, const LTVector& vNodePos, const LTVector& vNodeForward, bool bIgnoreDir ) const;

private:
	float m_fFovDp;
};

// ----------------------------------------------------------------------- //

// This test represents a simple FOV.  The AIs position must be inside 
// this FOV for this test to succeed.

#define AIOUTSIDEFOV_PROPS() \
	ADD_REALPROP_FLAG(Fov,						60.0f,			0|PF_CONEFOV, "The AI must be within this FOV of the forward vector of the node for it to be valid. [Degrees]")

class AINodeValidatorAIOutsideFOV
{
public:
	AINodeValidatorAIOutsideFOV();

	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, const LTVector& vAIPos, const LTVector& vNodePos, const LTVector& vNodeForward, bool bIgnoreDir ) const;

private:
	float m_fFovDp;
};

// ----------------------------------------------------------------------- //

// This test represents represents the 'enabled state' of the node.  For 
// this test to succeed, the node must not be disabled.

#define ENABLED_PROPS() \
	ADD_BOOLPROP_FLAG(StartDisabled,	false,				0, "If true the AINode will begin disabled.")

class AINodeValidatorEnabled
{
public:
	AINodeValidatorEnabled();

	bool IsEnabled() const;
	void SetEnabled( bool bEnabled );

	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags ) const;

private:
	bool m_bNodeEnabled;
};

// ----------------------------------------------------------------------- //

// This test verifies that the AI believes that it hasn't been seen for 
// some period of time.  If the AI knows its enemy saw it recently, this
// test will fail.

#define THREATUNSEEN_PROPS()

class AINodeValidatorThreatUnseen
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hThreat, HOBJECT hNodeObject ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that the AI believes its target is not aiming at it,
// while it is at the node.  If the AI is at the node AND if AI knows that
// its target is aiming at it, this test will fail.

#define THREATAIMINATATNODE_PROPS()

class AINodeValidatorThreatAimingAtNode
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject, bool bCoverBehindAlly ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that the enemy is outside of nodes threat radius.
// If the threat position is too close to the node, this test will fail.

#define THREATRADIUS_PROPS( radius ) \
	ADD_REALPROP_FLAG(ThreatRadius,				radius,			0|PF_RADIUS, "If the AI's threat comes within this radius of the node, it will invalidate the node. [WorldEdit units]") \
	ADD_STRINGPROP_FLAG(ThreatRegion,			"",				0|PF_OBJECTLINK, "Alternative to threat radius. If an AIRegion is specified, the radius is ignored.")

class AINodeValidatorThreatRadius
{
public:
	AINodeValidatorThreatRadius();

	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	void InitNodeValidator( AINode* pNode );

	bool Evaluate( AINode* pNode, uint32 dwFilteredStatusFlags, const LTVector& vThreatPos, ENUM_NMPolyID eThreatNMPoly ) const;
	
	float GetThreatRadiusSqr() const { return m_fThreatRadiusSqr; }

private:
	float			m_fThreatRadiusSqr;
	ENUM_AIRegionID	m_eThreatAIRegion;
	std::string		m_strThreatAIRegion;
};

// ----------------------------------------------------------------------- //

// This test verifies that the node and (optionally) its dependency are 
// locked by other AIs.  This test will fail if:
// 1) The node is locked by another AI
// 2) The node has a destination dependency, and the dependency is locked
// 3) The node has a occupation dependency, and another AI isn't at the 
// node

#define LOCKEDBYOTHER_PROPS()

class AINodeValidatorLockedByOther
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, EnumAINodeClusterID eNodeClusterID, HOBJECT hLockingAI, HOBJECT hDependency, bool* pbOutCoverBehindAlly ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that if the node has an owner, the AI has the owner locked.  
// This test will fail if the node has an owner, and the owner is unlocked,
// or locked by someone else.

#define OWNERNOTLOCKED_PROPS()

class AINodeValidatorOwnerNotLocked
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( AINode* pNode, uint32 dwFilteredStatusFlags, CAI* pAI ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that no AI has been damaged at this node recently.
// If an AI has been damaged, this test fails.

#define DAMAGED_PROPS()

class AINodeValidatorDamaged
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that no AI wants to avoid this node.
// If an AI wants to avoid this node, this test fails.

#define AVOID_PROPS()

class AINodeValidatorAvoid
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that the AIs path to the node is unobstructed by the 
// AIs current target.  If a direction to the enemy and to the node is close
// this test fails.

#define PATHBLOCKED_PROPS()

class AINodeValidatorPathBlocked
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, const LTVector& vNodePos, const LTVector& vAIPos, const LTVector& vThreatPos ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that the AI is behind the node.
// If the node is behind the AI, this test fails.

#define AIBACKTONODE_PROPS()

class AINodeValidatorAIBackToNode
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, const LTVector& vNodePos ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that the Player is not standing on the node.
// If the Player is standing on the node, this test fails.

#define PLAYERONNODE_PROPS()

class AINodeValidatorPlayerOnNode
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, const LTVector& vNodePos ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that the node is valid while following.
// If the node is not valid to use while following someone, this test fails.

#define VALIDFORFOLLOW_PROPS()

class AINodeValidatorValidForFollow
{
public:
	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject ) const;
};

// ----------------------------------------------------------------------- //

// This test verifies that the AI has not been at the node beyond the 
// level designer specified time.

#define EXPIRATION_PROPS( DefaultMinTime, DefaultMaxTime ) \
	ADD_REALPROP_FLAG(MinExpiration,			DefaultMinTime,	0, "Minimum time AI stays at this node. [Seconds]") \
	ADD_REALPROP_FLAG(MaxExpiration,			DefaultMaxTime,	0, "Maximum time AI stays at this node. [Seconds]")

class AINodeValidatorExpiration
{
public:
	AINodeValidatorExpiration();

	void ReadProps( const GenericPropList *pProps );
	void Load( ILTMessage_Read *pMsg );
	void Save( ILTMessage_Write *pMsg );

	void SetNewExpirationTime();
	bool Evaluate( uint32 dwFilteredStatusFlags, CAI* pAI, HOBJECT hNodeObject, ENUM_AIWMTASK_TYPE eScriptingTaskType ) const;
	double GetExpirationTime() const;

private:
	float  m_fMinExpiration;
	float  m_fMaxExpiration;
	double m_fExpirationTime;

};

#endif // _AINODEVALIDATORS_H_
