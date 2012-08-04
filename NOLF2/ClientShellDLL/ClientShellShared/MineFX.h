// ----------------------------------------------------------------------- //
//
// MODULE  : MineFX.h
//
// PURPOSE : Mine special fx class - Definition
//
// CREATED : 2/26/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MINE_FX_H__
#define __MINE_FX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"
#include "BaseScaleFX.h"

class CMineFX : public CSpecialFX
{
	public :

		CMineFX() : CSpecialFX()
		{
		}

		~CMineFX()
		{
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_MINE_ID; }

	protected :

		MINECREATESTRUCT	m_cs;
		BSCREATESTRUCT		m_scalecs;

		CBaseScaleFX		m_RadiusModel;
};

#endif // __MINE_FX_H__