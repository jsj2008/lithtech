// ----------------------------------------------------------------------- //
//
// MODULE  : PickupObject.h
//
// PURPOSE : PickupObject - Definition
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPOBJECTFX_H__
#define __PICKUPOBJECTFX_H__

#include "SpecialFX.h"


struct PICKUPOBJCREATESTRUCT : public SFXCREATESTRUCT
{
	PICKUPOBJCREATESTRUCT::PICKUPOBJCREATESTRUCT();
};

inline PICKUPOBJCREATESTRUCT::PICKUPOBJCREATESTRUCT()
{
	memset(this, 0, sizeof(PICKUPOBJCREATESTRUCT));
}


class CPickupObjectFX : public CSpecialFX
{
	public :

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

};

#endif // __PICKUPOBJECTFX_H__