// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeLead.h
//
// PURPOSE : This module defines AINodeLead.  This node is a location the 
//			 AI attempts to lead an object towards.  In addition, this node 
//			 specifies sounds to play if the follower gets too far behind.
//
// CREATED : 4/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODELEAD_H_
#define _AINODELEAD_H_

LINKTO_MODULE(AINodeLead);

#include "AINode.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeLead
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeLead : public AINode
{
	typedef AINode super;
	
	enum kConst
	{
		// Number of thresholds (this can be altered as needed; the current
		// value represents the count requested by level design).
		kFollowerDistanceThreshold_Count = 1
	};

public:
	enum
	{
		kThreshold_Invalid = -1
	};

	DEFINE_CAST( AINodeLead );

	// Ctor/Dtor

	AINodeLead();
	virtual ~AINodeLead();

	// Engine 

	virtual void ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Type

	EnumAINodeType GetType() const { return kNode_Lead; }

	// 

	float GetStartWaitingRadiusSqr() const;
	float GetResumePathingRadiusSqr() const;

	EnumAISoundType GetAISoundType_LeadWait() const;
	EnumAISoundType GetAISoundType_LeadResume() const;

private:
	PREVENT_OBJECT_COPYING(AINodeLead);

	EnumAISoundType		m_eLeadWait;
	EnumAISoundType		m_eLeadResume;
	float				m_flStartWaitingRadiusSqr;
	float				m_flResumePathingRadiusSqr;
};

class AINodeLeadPlugin : public AINodePlugin
{
private:
	LTRESULT PreHook_EditStringList( const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength );
};

#endif // _AINODELEAD_H_
