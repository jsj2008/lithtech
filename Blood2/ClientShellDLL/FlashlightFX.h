// ----------------------------------------------------------------------- //
//
// MODULE  : FlashlightFX.h
//
// PURPOSE : Flashlight effects
//
// CREATED : 10/12/98
//
// ----------------------------------------------------------------------- //

#ifndef __FLASHLIGHTFX_H__
#define __FLASHLIGHTFX_H__

#include "SpecialFX.h"


struct FLASHLIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	FLASHLIGHTCREATESTRUCT::FLASHLIGHTCREATESTRUCT();
};

inline FLASHLIGHTCREATESTRUCT::FLASHLIGHTCREATESTRUCT()
{
	memset(this, 0, sizeof(FLASHLIGHTCREATESTRUCT));
}


class CFlashlightFX : public CSpecialFX
{
	public :

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();
};

#endif // __FLASHLIGHTFX_H__