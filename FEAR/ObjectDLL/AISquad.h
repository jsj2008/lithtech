// ----------------------------------------------------------------------- //
//
// MODULE  : AISquad.h
//
// PURPOSE : AISquad class definition
//
// CREATED : 6/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISQUAD_H__
#define __AISQUAD_H__

#include "AIClassFactory.h"
#include "AIActivityAbstract.h"
#include "CharacterAlignment.h"

// Forward declarations.

class	CAIActivityAbstract;

#define MAX_AI_SQUAD_SIZE	20

// ----------------------------------------------------------------------- //

enum ENUM_AI_SQUAD_ID
{
	kSquad_Invalid = -1,
};

// ----------------------------------------------------------------------- //

class CAISquad : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS( CAISquad );

		 CAISquad();
		~CAISquad();

		void		Save(ILTMessage_Write *pMsg);
		void		Load(ILTMessage_Read *pMsg);

		// Initialization.

		void				InitSquad( ENUM_AI_SQUAD_ID eSquadID, EnumCharacterAlignment eAlignment );
		void				SetSquadID( ENUM_AI_SQUAD_ID eSquadID ) { m_eSquadID = eSquadID; }
		void				InitActivities();

		// Squad generation.

		void				AddSquadMember( HOBJECT hMember, const LTRect3f& AABB );
		void				MergeSquad( CAISquad* pSquad );
		bool				OverlapsSquad( const LTRect3f& AABB ) const;

		// Update.

		bool				UpdateSquad();

		// Targeting.

		bool				HasTarget( unsigned int dwTargetFlags );

		// Query.

		ENUM_AI_SQUAD_ID		GetSquadID() const { return m_eSquadID; }
		EnumCharacterAlignment	GetSquadAlignment() const { return m_eAlignment; }
		bool					IsSquadEngaged() const;
		LTObjRef*				GetSquadMembers() { return &( m_aSquadMembers[0] ); }
		int						GetNumSquadMembers() const { return m_cSquadMembers; }
		bool					IsSquadMember( HOBJECT hAI ) const;
		bool					SquadCompletedUpdate() const { return m_bTestedAllActivities; }
		bool					IsSquadMemberDead() const { return m_bSquadMemberDied; }
		CAIActivityAbstract*	GetCurrentActivity() { return m_pCurActivity; }
		LTRect3f&				GetSquadAABB() { return m_SquadAABB; }
		bool					SquadCanSeeTarget( ENUM_AI_TARGET_TYPE eTargetType ) const;

	protected:

		// AIActivities.

		CAIActivityAbstract*	AI_FACTORY_NEW_Activity( EnumAIActivityType eActivityType );

		// Squad deaths.

		void					HandleSquadDeaths();

	protected:

		ENUM_AI_SQUAD_ID		m_eSquadID;
		EnumCharacterAlignment	m_eAlignment;

		LTRect3f				m_SquadAABB;
		LTObjRef				m_aSquadMembers[MAX_AI_SQUAD_SIZE];
		int						m_cSquadMembers;

		bool					m_bSquadMemberDied;

		CAIActivityAbstract*	m_pCurActivity;
		CAIActivityAbstract*	m_apAIActivities[kActivity_Count];
		int						m_iActivityToTest;
		bool					m_bTestedAllActivities;
};

typedef std::vector<CAISquad*, LTAllocator<CAISquad*, LT_MEM_TYPE_OBJECTSHELL> > AI_SQUAD_LIST;

// ----------------------------------------------------------------------- //

#endif
