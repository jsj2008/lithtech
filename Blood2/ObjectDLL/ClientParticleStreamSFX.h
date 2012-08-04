// ----------------------------------------------------------------------- //
//
// MODULE  : ClientParticleStreamSFX.h
//
// PURPOSE : CClientParticleStreamSFX - Definition
//
// CREATED : 8-1-98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_PARTICLESTREAM_SFX_H__
#define __CLIENT_PARTICLESTREAM_SFX_H__

// ----------------------------------------------------------------------- //

#include "ClientSFX.h"

// ----------------------------------------------------------------------- //

class CClientParticleStreamSFX : public CClientSFX
{
	public :
		CClientParticleStreamSFX()	
		{
			hstrTexture = DNULL;
			hstrSound1	= DNULL;
			hstrSound2	= DNULL;
			hstrSound3	= DNULL;
		}
		~CClientParticleStreamSFX()	
		{
			g_pServerDE->FreeString( hstrTexture );
			g_pServerDE->FreeString( hstrSound1 );
			g_pServerDE->FreeString( hstrSound2 );
			g_pServerDE->FreeString( hstrSound3 );
		}
		void SetupFX();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void	HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void	CacheFiles();

		DFLOAT		fRadius;
		DFLOAT		fPosRadius;
		DFLOAT		fMinVel;
		DFLOAT		fMaxVel;
		DDWORD		nNumParticles;
		DFLOAT		fSpread;
		DVector		vColor1;
		DVector		vColor2;
		DFLOAT		fAlpha;
		DFLOAT		fMinLife;
		DFLOAT		fMaxLife;
		DFLOAT		fRampTime;
		DFLOAT		fDelay;
		DFLOAT		fGravity;
		DBYTE		bRampFlags;
		DBOOL		bOn;
		HSTRING		hstrTexture;
		DFLOAT		fSoundRadius;
		HSTRING		hstrSound1;
		HSTRING		hstrSound2;
		HSTRING		hstrSound3;
};

#endif // __CLIENT_PARTICLESTREAM_SFX_H__