// ----------------------------------------------------------------------- //
//
// MODULE  : ClientExplosionSFX.h
//
// PURPOSE : CClientExplosionSFX - Definition
//
// CREATED : 7-1-98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_EXPLOSION_SFX_H__
#define __CLIENT_EXPLOSION_SFX_H__

// ----------------------------------------------------------------------- //

#include "ClientSFX.h"

// ----------------------------------------------------------------------- //

#define CEXP_FADE_NONE				0
#define CEXP_FADE_ALPHA2CLEAR		1
#define CEXP_FADE_SOLID2ALPHA		2
#define CEXP_FADE_CLEAR2ALPHA		3
#define CEXP_FADE_ALPHA2SOLID		4

// ----------------------------------------------------------------------- //

#define CEXP_WAVETYPE_NONE		0
#define CEXP_WAVETYPE_INC		1
#define	CEXP_WAVETYPE_DEC		2

// ----------------------------------------------------------------------- //

class CClientExplosionSFX : public CClientSFX
{
	public :
		CClientExplosionSFX()
		{	
			VEC_INIT(m_vPos); 
			VEC_INIT(m_vNormal); 
			m_fOffset = 0.0f; 
			m_nType = 0; 
			m_bTriggered = 0;	
			m_hstrSound = DNULL;
		}

		~CClientExplosionSFX()
		{
			if (!g_pServerDE) return;
			if (m_hstrSound)
				g_pServerDE->FreeString(m_hstrSound);
		}

		void Setup(DVector *vPos, DVector *vNormal, DFLOAT fOffset);

		void SetupModel(DVector *vScale1, DVector *vScale2, DFLOAT fDuration, DFLOAT fInitAlpha,
					DBYTE bWaveForm, DBYTE bFade, DBOOL bRandRot, char *szModel, char *szSkin);

		void SetupSprite(DVector *vScale1, DVector *vScale2, DFLOAT fDuration, DFLOAT fInitAlpha,
					DBYTE bWaveForm, DBYTE bFade, DBOOL bAlign, char *szSprite);

		void SetupRing(DVector *vColor, DFLOAT fRadius, DFLOAT fPosRadius, DFLOAT fVelocity,
					DFLOAT fGravity, DDWORD nParticles, DFLOAT fDuration, DFLOAT fInitAlpha,
					DFLOAT fDelay, DBYTE bFade, DBOOL bAlign, char *szParticle);

		void SetupWave(DVector *vScale1, DVector *vScale2, DFLOAT fDuration, DFLOAT fInitAlpha,
					DFLOAT fDelay, DBYTE bWaveForm, DBYTE bFade, DBOOL bAlign, char *szWave);

		void SetupLight(DVector *vColor1, DVector *vColor2, DFLOAT fDuration, DFLOAT fDelay,
					DFLOAT fRadius1, DFLOAT fRadius2);

		void SetupFrag(DVector *vScale, DVector *vRotateMax, DFLOAT fSpread, DFLOAT fDuration,
					DFLOAT fVelocity, DFLOAT fBounceMod, DFLOAT fGravity, DFLOAT fFadeTime,
					DBOOL bRandDir, char *szModel, char *szSkin);

		void SetupFX(DVector *vPos, DVector *vNormal, DDWORD nType);
		void SetupFX();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void	HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);

		//*****	GENERAL VARIABLES *****
		DVector		m_vPos;
		DVector		m_vNormal;
		DFLOAT		m_fOffset;
		DFLOAT		m_fDamage;
		DFLOAT		m_fDamageRadius;
		DDWORD		m_nType;
		HSTRING		m_hstrSound;
//		char		m_szSound[MAX_CS_FILENAME_LEN];
		DBOOL		m_bTriggered;
};

#endif // __CLIENT_EXPLOSION_SFX_H__