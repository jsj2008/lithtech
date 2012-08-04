// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLaserBeamSFX.h
//
// PURPOSE : CClientLaserBeamSFX - Definition
//
// CREATED : 8-10-98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_LASERBEAM_SFX_H__
#define __CLIENT_LASERBEAM_SFX_H__

// ----------------------------------------------------------------------- //

#include "ClientSFX.h"

// ----------------------------------------------------------------------- //

struct CLIENTLASERBEAMSFX
{
	CLIENTLASERBEAMSFX::CLIENTLASERBEAMSFX();

	DVector		vSource;
	DVector		vDest;
	DBYTE		nType;
};

// ----------------------------------------------------------------------- //

inline CLIENTLASERBEAMSFX::CLIENTLASERBEAMSFX()
{
	memset(this, 0, sizeof(CLIENTLASERBEAMSFX));
}

// ----------------------------------------------------------------------- //

class CClientLaserBeamSFX : public CClientSFX
{
	public :
		CClientLaserBeamSFX()	{ fDamage = 0.0f; fDamageRadius = 0.0f; hstrSound = 0; }
		~CClientLaserBeamSFX( ) { if( hstrSound ) g_pServerDE->FreeString( hstrSound ); }

		void CreateBeam();
		void Fire(HOBJECT hSender);

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void	HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		CLIENTLASERBEAMSFX	clb;

		DFLOAT		fDamage;
		DFLOAT		fDamageRadius;

		HSTRING		hstrSound;
		DFLOAT		fSoundRadius;
};

#endif // __CLIENT_LASERBEAM_SFX_H__