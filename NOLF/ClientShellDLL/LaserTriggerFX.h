// ----------------------------------------------------------------------- //
//
// MODULE  : LaserTriggerFX.h
//
// PURPOSE : LaserTrigger special fx class - Definition
//
// CREATED : 2/11/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LASER_TRIGGER_FX_H__
#define __LASER_TRIGGER_FX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"
#include "PolyLineFX.h"
#include "BaseScaleFX.h"

class CLaserTriggerFX : public CSpecialFX
{
	public :

		CLaserTriggerFX() : CSpecialFX()
		{
		}

		~CLaserTriggerFX()
		{
            if(m_cs.hstrSpriteFilename && g_pLTClient)
            {
                g_pLTClient->FreeString(m_cs.hstrSpriteFilename);
            }
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_LASERTRIGGER_ID; }

	protected :

		LTCREATESTRUCT		m_cs;			// Our data
		PLFXCREATESTRUCT	m_pls;			// Data for creating the beam
		CPolyLineFX			m_Beam;			// Laser Beam
		CBaseScaleFX		m_StartSprite;	// Glow at start of beam
		CBaseScaleFX		m_EndSprite;	// Glow at end of beam

        LTBOOL CalcBeamCoords();
};

#endif // __LASER_TRIGGER_FX_H__