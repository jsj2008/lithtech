// ----------------------------------------------------------------------- //
//
// MODULE  : Searcher.h
//
// PURPOSE : The Searcher object
//
// CREATED : 12/20/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCHER_H__
#define __SEARCHER_H__

class CSearcher 
{
	public :	// Methods...

		CSearcher( )
			:	m_hTargetObj	( LTNULL ),
				m_hTargetHitBox	( LTNULL ),
				m_bShowTimeBar	( true ),
				m_bSearching	( false ),
				m_fTotalTime	( 0.0f ),
				m_fRemainingTime	( 0.0f ),
				m_fTimer	( 0.0f )
		{

		}

		~CSearcher( ) {}

		void	Update( );
		void	OnSearchMessage( ILTMessage_Read *pMsg );

		bool	IsSearching() {return (m_bSearching &&  m_hTargetObj);}
		
		uint8	GetMaxProgress();

	protected : // Members...

		HOBJECT		m_hTargetObj;
		HOBJECT		m_hTargetHitBox;
		bool		m_bShowTimeBar;
		bool		m_bSearching;
		float		m_fTotalTime;
		float		m_fRemainingTime;
		float		m_fTimer;
};

#endif // __SEARCHER_H__