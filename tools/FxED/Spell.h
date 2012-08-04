//------------------------------------------------------------------
//
//   MODULE  : SPELL.H
//
//   PURPOSE : Defines class CSpell
//
//   CREATED : On 10/29/98 At 10:26:26 AM
//
//------------------------------------------------------------------

#ifndef __SPELL__H_
	#define __SPELL__H_

	// Includes....

	#include "LinkList.h"
	#include "Phase.h"

	// Defines....

	#define ST_POINT						0
	#define ST_PROJECTILE					1

	#define STT_SINGLE						0
	#define STT_AREA						1
	#define STT_SINGLEAREA					2

	#define CPT_FLUX						0
	#define CPT_SANITY						1
	#define CPT_HITPOINTS					2

	#define CHO_ONCE						0
	#define CHO_EVERYSECOND					1
	#define CHO_EVERY5SECONDS				2
	#define CHO_EVERY10SECONDS				3
	#define CHO_EVERY15SECONDS				4
	#define CHO_EVERY20SECONDS				5

	#define SAL_FAST						0
	#define SAL_MEDIUM						1
	#define SAL_SLOW						2

	struct TOTEM_REQUIREMENT
	{
		CString									m_sRequirement;
	};

	struct PHASE
	{
//		CLinkList<CBaseEffect *>				m_collEffects;
	};

	class CSpell
	{
		public :

			// Constuctor

												CSpell();

			// Destructor

										       ~CSpell();

			// Member Functions

			BOOL								Init();
			void								Term();

			// Accessors

			char*								GetGuid() { return m_sGuid; }
			char*								GetName() { return m_sName; }
			char*								GetDesc() { return m_sDesc; }
			char**								GetDescPtr() { return (char **)&m_sDesc; }
			CLinkList<TOTEM_REQUIREMENT>*		GetTotemRequirements() { return &m_collTotemRequirements; }
			CString&							GetCastCost() { return m_sCastCost; }
			CString&							GetRadius() { return m_sRadius; }
			int									GetCastPtType() { return m_nCastPtType; }
			int									GetHowOften() { return m_nHowOften; }
			int									GetType() { return m_nType; }
			int									GetTargetType() { return m_nTargetType; }
//			CLinkList<CBaseEffect *>*			GetEffects() { return &m_collEffects; }
			CPhase*								GetPhase(int nPhase) { return m_collPhases + nPhase; }
			CPhase*								GetCastPhase() { return m_collPhases; }
			CPhase*								GetActivePhase() { return m_collPhases + 1; }
			CPhase*								GetResolutionPhase() { return m_collPhases + 2; }
			int									GetCastSpeed() { return m_nCastSpeed; }
											
			void								SetGuid(char *sGuid) { strcpy(m_sGuid, sGuid); }
			void								SetName(char *sName) { strcpy(m_sName, sName); }
			void								SetDesc(char *sDesc) { strcpy(m_sDesc, sDesc); }
			void								SetCastCost(CString sCost) { m_sCastCost = sCost; }
			void								SetRadius(CString sRadius) { m_sRadius = sRadius; }
			void								SetCastPtType(int nPtType) { m_nCastPtType = nPtType; }
			void								SetHowOften(int nHowOften) { m_nHowOften = nHowOften; }
			void								SetType(int nType) { m_nType = nType; }
			void								SetTargetType(int nType) { m_nTargetType = nType; }
			void								SetCastSpeed(int nSpeed) { m_nCastSpeed = nSpeed; }
											
		protected :								
												
			// Member Variables					
												
			char								m_sGuid[128];
			char								m_sName[128];
			char								m_sDesc[128];
			CLinkList<TOTEM_REQUIREMENT>		m_collTotemRequirements;
			CString								m_sCastCost;
			CString								m_sRadius;
			int									m_nCastPtType;
			int									m_nHowOften;
			int									m_nType;
			int									m_nTargetType;
			int									m_nCastSpeed;
			CPhase								m_collPhases[3];
	};

#endif