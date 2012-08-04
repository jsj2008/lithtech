// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWarpGateSFX.h
//
// PURPOSE : CClientWarpGateSFX - Definition
//
// CREATED : 8-15-98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_WARPGATE_SFX_H__
#define __CLIENT_WARPGATE_SFX_H__

// ----------------------------------------------------------------------- //

#include "ClientSFX.h"

// ----------------------------------------------------------------------- //

struct CLIENTWARPSPRITE
{
	DFLOAT	fMinScale;
	DFLOAT	fMaxScale;
	DFLOAT	fAlpha;
	DDWORD	nRampUpType;
	DDWORD	nRampDownType;
	DBOOL	bAlign;
	HSTRING	hstrSprite;
};

struct CLIENTWARPPARTICLE
{
	DFLOAT	fSystemRadius;
	DFLOAT	fPosRadius;
	DVector	vOffset;
	DVector	vRotations;
	DFLOAT	fMinVelocity;
	DFLOAT	fMaxVelocity;
	DDWORD	nNumParticles;
	DDWORD	nEmitType;
	DVector	vMinColor;
	DVector	vMaxColor;
	DFLOAT	fAlpha;
	DFLOAT	fMinLifetime;
	DFLOAT	fMaxLifetime;
	DFLOAT	fAddDelay;
	DFLOAT	fGravity;
	DDWORD	nRampUpType;
	DDWORD	nRampDownType;
	DBOOL	bAlign;
	HSTRING	hstrParticle;
};

// ----------------------------------------------------------------------- //

class CClientWarpGateSFX : public CClientSFX
{
	public :
		CClientWarpGateSFX();
		~CClientWarpGateSFX();
		void SetupFX();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void	HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		DFLOAT		fRampUpTime;
		DFLOAT		fRampDownTime;
		DBOOL		bInitiallyOn;

		CLIENTWARPSPRITE	wSpr;
		CLIENTWARPPARTICLE	wPS1;
		CLIENTWARPPARTICLE	wPS2;

		DFLOAT		fSoundRadius;
		HSTRING		hstrSound1;
		HSTRING		hstrSound2;
		HSTRING		hstrSound3;

		DBOOL		bOn;
		DBOOL		bFirstUpdate;
};

#endif // __CLIENT_WARPGATE_SFX_H__