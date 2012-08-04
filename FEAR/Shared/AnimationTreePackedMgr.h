// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationTreePackedMgr.h
//
// PURPOSE : AnimationTreePackedMgr class implementation.
//           Manager for creating and accessing packed AnimTrees.
//
// CREATED : 6/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __ANIMATION_TREE_PACKED_MGR_H__
#define __ANIMATION_TREE_PACKED_MGR_H__

#include "AnimationTreePackedTypes.h"


//
// Forward declarations.
//

class	CAnimationTreePacked;
class	CAnimationTreePackedMgr;
struct	ANIM_TREE_INDEX;
struct	TRANS_QUERY_RESULTS;


// 
// Global.
//

extern CAnimationTreePackedMgr* g_pAnimationTreePackedMgr;


//
// Structs.
//

struct GLOBAL_ANIM_TREE_TRANSITION
{
	const char*				pszName;
	AT_GLOBAL_TRANSITION_ID	eGlobalID;
};
typedef std::vector<GLOBAL_ANIM_TREE_TRANSITION> GLOBAL_ANIM_TREE_TRANSITION_LIST;


//
// CAnimationTreePackedMgr
//

class CAnimationTreePackedMgr
{
	public:
	
		 CAnimationTreePackedMgr();
		~CAnimationTreePackedMgr();

		// Must turn off inlining on the Instance method because of compiler bug which 
		// ends up calling the constructor ever time Instance is called.
		// See: http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&oe=UTF-8&threadm=u0nRp5Qr%23GA.233%40cppssbbsa03&rnum=2&prev=/groups%3Fhl%3Den%26lr%3D%26ie%3DUTF-8%26oe%3DUTF-8%26q%3Dsingleton%2Bstatic%2Binline%2Bgroup%253Amicrosoft.public.vc.*%26btnG%3DSearch%26meta%3Dgroup%253Dmicrosoft.public.*
		NO_INLINE static CAnimationTreePackedMgr& CAnimationTreePackedMgr::Instance()
		{
			static CAnimationTreePackedMgr inst;
			return inst;
		}

		bool	InitAnimationTreePackedMgr();
		void	TermAnimationTreePackedMgr();

		CAnimationTreePacked*	GetAnimationTreePacked( const char* pszFilename );

		bool					FindTransition( const ANIM_TREE_INDEX &IndexAnimationFrom, 
												const ANIM_TREE_INDEX &IndexAnimationTo, 
												const ANIM_TREE_PACKED_LIST &lstAnimTreePacked, 
												TRANS_QUERY_RESULTS &rTransResults ) const;

	private:

		CAnimationTreePacked*	FindAnimationTreePacked( const char* pszFilename );
		CAnimationTreePacked*	CreateAnimationTreePacked( const char* pszFilename );

		void					AssignGlobalTransitionIDs( CAnimationTreePacked* pTree );
		AT_GLOBAL_TRANSITION_ID	GetGlobalTransitionID( const char* pszName );

	private:

		ANIM_TREE_PACKED_LIST					m_lstAnimTrees;
	
		uint32									m_iNextTreeID;

		GLOBAL_ANIM_TREE_TRANSITION_LIST		m_lstGlobalTransitions;
		uint32									m_iNextTransitionID;
};

#endif
