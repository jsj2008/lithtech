// ----------------------------------------------------------------------- //
//
// MODULE  : AINavigationMgr.h
//
// PURPOSE : AINavigationMgr abstract class definition
//
// CREATED : 2/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AINAVIGATION_MGR_H__
#define __AINAVIGATION_MGR_H__

#include "AnimationProp.h"

// Forward declarations.

class	CAI;
class	CAIPathNavMesh;
class	CAnimationProps;
struct	SPATH_NODE;

enum ENUM_AI_NAV_STATUS
{
	kNav_Unset,
	kNav_Set,
	kNav_Failed,
	kNav_Done,
};


// ----------------------------------------------------------------------- //

class CAINavigationMgr
{
	public:

		CAINavigationMgr();
		~CAINavigationMgr();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		void	InitNavigationMgr( CAI* pAI );

		void	UpdateNavigation();
		void	UpdateNavigationAnimation();

		void	HandleAIDeath();

		// This is a hack to support the ghost's FlyThru links.
		// The path should really be moved to the Blackboard.

		CAIPathNavMesh* GetNMPath() { return m_pNMPath; }
		bool			IsNavSet() const { return m_bNavSet; }

	protected:

		// Path manipulation

		void	AdvancePath();
		void	MoveToPathNode(SPATH_NODE* pPathNode);
		bool	SetPath( const LTVector& vDest );
		void	SetNavDone();

		bool	IsPathValid(const LTVector& vDest) const;

		// Animation selection.

		void			SelectMovementAnimProps( CAnimationProps* pProps );
		EnumAnimProp	SelectMovementDirection();

		// Debug rendering.

		void	UpdateDebugRendering( float fVarTrack );
		void	DrawPath();
		void	HidePath();

	protected:

		CAI*				m_pAI;
		LTVector			m_vDest;
		bool				m_bNavSet;

		CAIPathNavMesh*		m_pNMPath;
		bool				m_bDrawingPath;

		uint32				m_nPathMgrKnowledgeIndex;
};

// ----------------------------------------------------------------------- //

#endif
