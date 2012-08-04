// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionMgr.h
//
// PURPOSE : The TransitionMgr object
//
// CREATED : 11/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __TRANSITIONMGR_H__
#define __TRANSITIONMGR_H__

//
// Globals...
//
	
	class CTransitionMgr;
	extern CTransitionMgr *g_pTransMgr;


class CTransitionMgr 
{
	public : // Methods...

		CTransitionMgr( );
		~CTransitionMgr( );

		void	TransitionFromArea( HOBJECT hTransArea );

		bool	PreTransitionLoad( char const* pSaveLevelFile );
		bool	PostTransitionLoad( char const* pRestoreLevelFile );

		HOBJECT	GetTransitionArea( ) const { return m_hTransArea; }

	
	protected : // Methods...

		bool	SaveTransitionObjects( );
		bool	LoadTransitionObjects( );

		bool	RemoveTransitionObjects( );

		bool	SaveNonTransitionObjects( char const* pSaveLevelFile );
		bool	LoadNonTransitionObjects( char const* pRestoreLevelFile );


	private : // Members...

		typedef ObjRefList	TransitionObjectList;
		TransitionObjectList	m_TransitionObjectList;

		char		m_szTransAreaName[32];	// The name of the transition from/to areas need to be the same
		LTVector	m_vTransAreaDims;		// The diminsions of the transition from/to areas need to be the same
		LTObjRef	m_hTransArea;			// The objcet we are transitioning from/to


	private : // Methods...

		bool		IsTransitionObject( HOBJECT hObj );
		
};

#endif // __TRANSITIONMGR_H__
