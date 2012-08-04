// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.h
//
// PURPOSE : Explosion special fx class - Definition
//
// CREATED : 5/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_FX_H__
#define __EXPLOSION_FX_H__

#include "BaseScaleFX.h"


struct EXCREATESTRUCT : public BSCREATESTRUCT
{
};

class CExplosionFX : public CBaseScaleFX
{
	public :

		CExplosionFX() : CBaseScaleFX() 
		{
			m_nType = OT_MODEL;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	private :
};

#endif // __EXPLOSION_FX_H__