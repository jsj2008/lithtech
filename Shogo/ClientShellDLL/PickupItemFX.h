// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.h
//
// PURPOSE : PickupItem - Definition
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPITEM_FX_H__
#define __PICKUPITEM_FX_H__

#include "SpecialFX.h"
#include "ModelFuncs.h"

struct PICKUPITEMCREATESTRUCT : public SFXCREATESTRUCT
{
	PICKUPITEMCREATESTRUCT::PICKUPITEMCREATESTRUCT();
};

inline PICKUPITEMCREATESTRUCT::PICKUPITEMCREATESTRUCT()
{
	memset(this, 0, sizeof(PICKUPITEMCREATESTRUCT));
}

class CPickupItemFX : public CSpecialFX
{
	public :

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update();

};

#endif // __PICKUPITEM_FX_H__