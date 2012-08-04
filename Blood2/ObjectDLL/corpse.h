

#ifndef __CORPSE_H__
#define	__CORPSE_H__

#include "basedefs_de.h"

#include "destructablemodel.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "ai_mgr.h"
#include "B2BaseClass.h"

class CCorpse : public B2BaseClass
{
	public:
		
		CCorpse() : B2BaseClass(OT_MODEL)
		{
			AddAggregate(&m_damage);

			m_pServerDE		= DNULL;
			m_fMass			= 200.0f;
			m_fHitPoints	= 50.0f;
			m_bDead			= DFALSE;
			m_bSetShutdown	= DFALSE;

			m_hFireSource	= DNULL;
			m_fBurnStart	= 0.0f;
			m_hAttach		= DNULL;

			m_hSmokeSource	= DNULL;
			m_fSmokeStart	= 0.0f;

			m_fRemoveTime	= 0.0f;
			m_bLimbLoss		= DTRUE;
			m_hLoopSound	= DNULL;

			m_szSoundDir[0] = '\0';
		}

		~CCorpse()
		{
			if( m_hLoopSound )
			{
				g_pServerDE->KillSound( m_hCurSound );
			}
			if( m_hLoopSound )
			{
				g_pServerDE->KillSound( m_hCurSound );
			}

		}

		void	Setup(HOBJECT hFireObj = DNULL, DBOOL bLimbLoss = DTRUE);

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private:

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	PostPropRead(ObjectCreateStruct *pStruct);
		void	InitialUpdate(DVector *pMovement);
		DBOOL	Update();
		DBOOL	HandleDamage(DVector vDir, DFLOAT fDamage, int nType, DVector vPos);
		int		CalculateHitLimb(DVector vDir, DVector vPos);
		int		SetProperNode(int nNode);
		DBOOL	CreateGibs(DVector vDir, int nNumGibs, int nType, DFLOAT fDamage);
		void	GenerateHitSpheres();
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void	OnStringKey(ArgList* pArgList);
		DBOOL	PlayAISound(char* szSound, DFLOAT fRadius, DDWORD dwFlags, int nVol);
		void	CreateBloodPool();

		CDestructable	m_damage;
		CServerDE*		m_pServerDE;
        AI_Shared       AIShared;       // Shared functions

		DFLOAT			m_fHitSpheres[NUM_STD_NODES];

		DFLOAT			m_fMass;
		DFLOAT			m_fHitPoints;
		DBOOL			m_bDead;
		char			m_szSoundDir[256];
		HSOUNDDE		m_hCurSound;
		DBOOL			m_bSetShutdown;

		HOBJECT			m_hFireSource;
		DFLOAT			m_fBurnStart;
		HATTACHMENT		m_hAttach;

		HOBJECT			m_hSmokeSource;
		DFLOAT			m_fSmokeStart;

		DFLOAT			m_fRemoveTime;
		DBOOL			m_bLimbLoss;
		HSOUNDDE		m_hLoopSound;
};

#endif