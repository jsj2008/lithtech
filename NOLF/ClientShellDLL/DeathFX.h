// ----------------------------------------------------------------------- //
//
// MODULE  : DeathFX.h
//
// PURPOSE : Death special fx class - Definition
//
// CREATED : 6/14/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEATH_FX_H__
#define __DEATH_FX_H__

#include "SpecialFX.h"
#include "ContainerCodes.h"
#include "CharacterAlignment.h"
#include "GibFX.h"

struct DEATHCREATESTRUCT : public SFXCREATESTRUCT
{
    DEATHCREATESTRUCT();

	ModelId		eModelId;
	ModelStyle	eModelStyle;
    uint8       nDeathType;
    LTVector     vPos;
    LTVector     vDir;
};

inline DEATHCREATESTRUCT::DEATHCREATESTRUCT()
{
	eModelId		= eModelIdInvalid;
	eModelStyle		= eModelStyleInvalid;
	nDeathType		= 0;
	vPos.Init();
	vDir.Init();
}


class CDeathFX : public CSpecialFX
{
	public :

		CDeathFX() : CSpecialFX()
		{
			m_eCode				= CC_NO_CONTAINER;
			m_nDeathType		= 0;
			m_eModelId			= eModelIdInvalid;
			m_eModelStyle		= eModelStyleInvalid;
			VEC_INIT(m_vPos);
			VEC_INIT(m_vDir);
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update() { return LTFALSE; }

		virtual uint32 GetSFXID() { return SFX_DEATH_ID; }

	protected :

		ContainerCode	m_eCode;			// Container effect is in
        LTVector         m_vPos;             // Effect position
        LTVector         m_vDir;             // Direction damage came from
        uint8           m_nDeathType;       // Type of death
		ModelId			m_eModelId;			// Model
		ModelStyle		m_eModelStyle;		// Style of model

		void CreateDeathFX();
		void CreateVehicleDeathFX();
		void CreateHumanDeathFX();

		void SetupGibTypes(GIBCREATESTRUCT & gib);
};

#endif // __DEATH_FX_H__