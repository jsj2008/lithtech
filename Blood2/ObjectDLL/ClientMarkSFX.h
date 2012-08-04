// ----------------------------------------------------------------------- //
//
// MODULE  : CClientMarkSFX.h
//
// PURPOSE : CClientMarkSFX - Definition
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTMARKSFX_H__
#define __CLIENTMARKSFX_H__

#include "ClientSFX.h"
#include "SharedDefs.h"


class CClientMarkSFX : public CClientSFX
{
	public:

		CClientMarkSFX() : CClientSFX()
		{
			m_hAttachment = DNULL;
		}

	public :
		
//		void Setup( DVector *pvPos, DVector *pvDir, DFLOAT fScale, HSTRING hstrSprite,
//					DVector vColor, DBOOL bScaleY = DFALSE, DFLOAT fScaleCount = 0.0f);
		void Setup( DVector *pvPos, DVector *pvDir, DBYTE nWeaponType, DBYTE nAmmoType, SurfaceType eType, HOBJECT hLinkObj = DNULL);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private:

		char* GetMarkSprite(DBYTE nWeaponType, DBYTE nAmmoType, SurfaceType eSurfType);
		HATTACHMENT m_hAttachment;

};

#endif // __CLIENTMARKSFX_H__
