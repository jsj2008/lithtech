// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDoor.h
//
// PURPOSE : AI NavMesh Link Door class definition
//
// CREATED : 07/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_DOOR_H_
#define _AI_NAVMESH_LINK_DOOR_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLinkDoor );


// Forward declarations.

class	Door;


class AINavMeshLinkDoor : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

	enum kConst
	{
		// Max number of doors used by the link
		kMaxDoors = 2,
	};

	enum ENUM_Channels
	{
		// Channel identifiers
		kChannel_Invalid	= -1,
		kChannel_Door1,
		kChannel_Door2,
		kChannel_DoorDouble,
		kChannel_Count		= 3,
	};

public:
	struct Channel
	{
		Channel();
		void		Save(ILTMessage_Write *pMsg);
		void		Load(ILTMessage_Read *pMsg);

		LTVector	GetCenter() const;
		bool		GetRequiresAction(CAI* pAI) const;
		bool		GetIsOpen(CAI* pAI) const;
		bool		GetIsOpening(CAI* pAI) const;
		bool		GetIsRotating() const;
		bool		GetCanTraverse(CAI* pAI) const;
		bool		GetBlocked(CAI* pAI) const;

		void		SetBlockedToAI(CAI* pAI, bool bJammed) const;
		bool		GetBlockedToAI(CAI* pAI) const;
		void		Open(CAI* pAI) const;
		
		LTObjRef	m_hDoor[2];
		int			m_iDoorCount;

		LTVector	m_v0;
		LTVector	m_v1;
	};


	AINavMeshLinkDoor();

	virtual void	ReadProp(const GenericPropList *pProps);
	virtual void	InitialUpdate();
	virtual void	Save(ILTMessage_Write *pMsg);
	virtual void	Load(ILTMessage_Read *pMsg);

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Door; }
	virtual float					GetNMLinkPathingWeight(CAI* pAI);
	virtual bool					IsLinkValidDest() { return false; }
	virtual bool					IsLinkPassable( CAI* pAI, ENUM_NMPolyID ePolyTo );
	virtual bool					IsLinkRelevant( CAI* pAI );
	virtual bool					IsLinkValid(CAI* pAI, EnumAIActionType eActionType, bool bTraversalInProgress);
	virtual bool					IsTraversalComplete( CAI* pAI );
	virtual bool					PullStrings( const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos );
	virtual bool					IsPullStringsModifying( CAI* pAI );
	virtual void					ApplyMovementAnimation( CAI* pAI );
	virtual void					ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject );
	virtual float					GetNMLinkOffsetEntryDistA() const;
	virtual float					GetNMLinkOffsetEntryDistB() const;

	// Expose ability to request a channel be opened.

	void							OpenDoor(CAI* pAI);
	void							HandleDoorClosed( Door* pDoor );
	void							HandleDoorOpened( Door* pDoor );
	bool							IsTraversalDoorBlockedToAI( CAI* pAI );
	bool							IsTraversalDoorBlocked( CAI* pAI );
	void							SetDoorBlockedIsJammed( CAI* pAI );
	bool							IsDoorRotating(CAI* pAI);
	bool							DoDoorsExist() const;

protected:
	void							SetupDoorChannels();
	ENUM_Channels					SelectChannelForStringPull(const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos) const;
	bool							ChannelPullString(ENUM_Channels eChannel, const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos);
	bool							GetChannel(CAI* pAI, const Channel** pOutChannel) const;
	void							SetChannel(CAI* pAI, ENUM_Channels eChannel) const;
	float							GetNMLinkOffsetEntryDist( float fDefaultOffset ) const;

	std::string		m_strDoor;

	LTObjRef		m_hDoor[kMaxDoors];
	Channel			m_Channel[kChannel_Count];

	int				m_iValidDoors;
	int				m_iValidChannels;
};

//-----------------------------------------------------------------

class AINavMeshLinkDoorPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkDoor; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_DOOR_H_
