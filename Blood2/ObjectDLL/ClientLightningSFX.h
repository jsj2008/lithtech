// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLightningSFX.h
//
// PURPOSE : CClientLightningSFX - Definition
//
// CREATED : 8-1-98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_LIGHTNING_SFX_H__
#define __CLIENT_LIGHTNING_SFX_H__

// ----------------------------------------------------------------------- //

#include "ClientSFX.h"

// ----------------------------------------------------------------------- //

struct CLIENTLIGHTNINGSFX
{
	CLIENTLIGHTNINGSFX::CLIENTLIGHTNINGSFX();

	DVector		vSource;
	DVector		vDest;
	DBYTE		nShape;
	DBYTE		nForm;
	DBYTE		nType;
};

// ----------------------------------------------------------------------- //

inline CLIENTLIGHTNINGSFX::CLIENTLIGHTNINGSFX()
{
	memset(this, 0, sizeof(CLIENTLIGHTNINGSFX));
}

// ----------------------------------------------------------------------- //

class CClientLightningSFX : public CClientSFX
{
	public :
		CClientLightningSFX()	{ bTriggered = 0; hstrSound = DNULL; }
		~CClientLightningSFX()	{ g_pServerDE->FreeString( hstrSound ); }
		void SetupFX(CLIENTLIGHTNINGSFX *clFX, DFLOAT damage, DFLOAT damageRadius, 
					char *szSound, DFLOAT soundRadius);

		void CreateBolt();
		void Fire(HOBJECT hSender);

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void	HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		CLIENTLIGHTNINGSFX	cl;

		DFLOAT		fDamage;
		DFLOAT		fDamageRadius;

		HSTRING		hstrSound;
		DFLOAT		fSoundRadius;

		DBOOL		bTriggered;
};

#endif // __CLIENT_LIGHTNING_SFX_H__