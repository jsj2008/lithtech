/****************************************************************************
;
;	 MODULE:		SOCCEROBJECTS (.H)
;
;	PURPOSE:		Soccer objects for zombie-head soccer
;
;	HISTORY:		1/26/99 [bp] This file was created
;
;	COMMENT:		Copyright (c) 1999, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _SOCCEROBJECTS_H_
#define _SOCCEROBJECTS_H_


// Includes...

#include "B2BaseClass.h"


// Classes...

class SoccerGoal : public B2BaseClass
{
	// Member functions...

	public:

		SoccerGoal();
		~SoccerGoal();

		DDWORD			EngineMessageFn( DDWORD messageID, void* pData, DFLOAT fData );
//		DDWORD			ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead );

		int				GetTeamID() { return( m_nTeamID ); }

		void			SendTrigger( );
		static DList *	GetGoalList( ) { return &m_GoalList; }
		void			SpawnBall( );

	protected:

		void			ReadProp( ObjectCreateStruct* pStruct );
		void			PostPropRead( ObjectCreateStruct* pStruct );
		void			OnTouchNotify( HOBJECT hObj );
		void			Update( );

		void			OnInitialUpdate( void* pData, DFLOAT fData );

	// Member variables...

	private:

		static DList	m_GoalList;
		DLink			m_Link;

		int				m_nTeamID;
		DBOOL			m_bDirectional;
		DVector			m_vGoalDirection;
		DBOOL			m_bBoxPhysics;
		HSTRING			m_hstrScoreSound;
		float			m_fRadius;
		HSTRING			m_hstrScoreTarget;
		HSTRING			m_hstrScoreMsg;
		HCLASS			m_hSoccerBall;
		DBYTE			m_nNumBallsToMake;
		DBOOL			m_bWaitOneFrame;
};


class SoccerBall : public B2BaseClass
{
	// Member functions...

	public:

		SoccerBall();
		~SoccerBall();

		HOBJECT			GetLastPlayerTouched( ) const { return m_hLastPlayer; }

		DDWORD			EngineMessageFn( DDWORD messageID, void* pData, DFLOAT fData );
		DDWORD			ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead );

		DBOOL			IsMadeGoal( ) { return m_bMadeGoal; }

	protected:

		void			ReadProp( ObjectCreateStruct* pStruct );
		void			PostPropRead( ObjectCreateStruct* pStruct );

		void			OnInitialUpdate( void* pData, DFLOAT fData );
		void			OnTouchNotify( HOBJECT hObj, float fForce );
		void			Update( );
		void			CreateLight( );

	// Member variables...

	private:

		DBOOL			m_bOnGround;
		float			m_fLastTimeOnGround;
		float			m_fLastBounceTime;
		DBOOL			m_bBounced;
		float			m_fRadius;
		float			m_fRespawnTime;

		DVector			m_vLastNormal;
		DVector			m_vLastPos;

		HOBJECT			m_hLastPlayer;

//		HOBJECT			m_hLight;
//		HATTACHMENT		m_hLightAttachment;

		DBOOL			m_bMadeGoal;

};

#endif // _SOCCEROBJECTS_H_


