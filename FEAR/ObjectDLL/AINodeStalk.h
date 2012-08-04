// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeStalk.h
//
// PURPOSE : 
//
// CREATED : 5/17/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AINODESTALK_H_
#define __AINODESTALK_H_

#include "AINode.h"

LINKTO_MODULE( AINodeStalk );

class AINodeStalk : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

public :

	AINodeStalk();
	~AINodeStalk();
	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// TEMP
	virtual uint32		EngineMessageFn(uint32 messageID, void *pv, float fData);
	void Update();
	// \TEMP

	virtual EnumAINodeType GetType() const { return kNode_Stalk; }
	virtual void		ReadProp(const GenericPropList *pProps);
	virtual bool		GetDestinationPosition(CAI* pAI, const LTVector& vThreatPosition, LTVector& vOutPosition) const;
	virtual float		GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
	virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }
	virtual bool		IsNodeValid( CAI* /*pAI*/, const LTVector& /*vPosAI*/, HOBJECT /*hThreat*/, EnumAIThreatPosition eThreatPos, uint32 /*dwStatusFlags*/ );

	// Arrival / Departure.

	virtual void HandleAIArrival( CAI* pAI );

	// Debugging visualization functionality

	virtual bool		AllowOutsideNavMesh() { return true; }
	virtual int			DrawSelf();
	virtual int			HideSelf();

private:
	void				LoadBlindObjectData();
	void				ReadData(uint8*);

	bool				GetSafeStalkOBB(float flAIRadius, float flHeight, const LTVector& vThreat, LTOBB& rOutOBB) const;
	void				DrawProbes(const LTOBB& SafeOBB, LTVector* paProbes, int nProbes, DebugLineSystem& system) const;
	void				GetSafeOBBProbes(LTOBB& rOBB, float flMinDistanceBetweenProbes, 
							int kSubDivisionWidthMax, int kSubDivisionDepthMax,  
							int nMaxProbes, LTVector* avOutProbePoints, int& nOutProbeCount) const;


	uint32				m_nStalkDataIndex;

	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;

	// Don't save:

	VECTOR_LIST			m_lstStalkVerts;
	LTVector			m_vStalkCenter;
	bool				m_bDrawing;
};

#endif // __AINODESTALK_H_

