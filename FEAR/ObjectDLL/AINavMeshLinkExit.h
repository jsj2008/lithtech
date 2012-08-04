// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkExit.h
//
// PURPOSE : 
//
// CREATED : 4/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AINAVMESHLINKEXIT_H_
#define __AINAVMESHLINKEXIT_H_

#include "AINavMeshLinkAbstract.h"

class AINodeSmartObject;

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINavMeshLinkExit
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINavMeshLinkExit : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

	enum Const
	{
		kMaxActionNodes = 4
	};

public:
	// Constructors

	AINavMeshLinkExit();
	virtual ~AINavMeshLinkExit();

	// Engine

	virtual uint32	EngineMessageFn(uint32 messageID, void *pvData, float fData);
	virtual void	ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// AINavMeshLinkAbstract overrides

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Exit; }
	virtual bool					GetAllowDirectionalMovement() { return true; }
	bool							IsLinkExitRelevant( CAI* pAI );
	
	AINodeSmartObject* GetActionSmartObjectNode( CAI* pAI );

private:
	AINavMeshLinkExit(const AINavMeshLinkExit& src);				// Not implemented
	const AINavMeshLinkExit& operator=(const AINavMeshLinkExit&);	// Not implemented

	float			m_flThreatRadiusSqr;
	float			m_flBoundaryRadiusSqr;

	LTObjRef		m_hActionSmartObjectNode[kMaxActionNodes];
	std::string		m_aszActionSmartObjectName[kMaxActionNodes];
};



#endif // __AINAVMESHLINKEXIT_H_
