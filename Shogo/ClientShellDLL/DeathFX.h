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
#include "ModelFuncs.h"
#include "GibFX.h"

struct DEATHCREATESTRUCT : public SFXCREATESTRUCT
{
	DEATHCREATESTRUCT::DEATHCREATESTRUCT();

	uint8	nModelId;
	uint8	nDeathType;
	uint8	nSize;
	uint8	nCharacterClass;
	LTVector vPos;
	LTVector vDir;
};

inline DEATHCREATESTRUCT::DEATHCREATESTRUCT()
{
	memset(this, 0, sizeof(DEATHCREATESTRUCT));
}


class CDeathFX : public CSpecialFX
{
	public :

		CDeathFX() : CSpecialFX() 
		{
			m_eCode				= CC_NONE;
			m_nDeathType		= 0;
			m_nModelId			= MI_UNDEFINED;
			m_nSize				= MS_NORMAL;
			m_nCharacterClass	= UNKNOWN;
			VEC_INIT(m_vPos);
			VEC_INIT(m_vDir);
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update() { return LTFALSE; }

	protected :
	
		ContainerCode	m_eCode;			// Container effect is in
		LTVector			m_vPos;				// Effect position
		LTVector			m_vDir;				// Direction damage came from
		uint8			m_nDeathType;		// Type of death
		uint8			m_nModelId;			// Type of model
		uint8			m_nSize;			// Size of model
		uint8			m_nCharacterClass;	// Character class

		void CreateDeathFX();
		void CreateVehicleDeathFX();
		void CreateMechaDeathFX();
		void CreateHumanDeathFX();

		void SetupGibTypes(GIBCREATESTRUCT & gib);
};

#endif // __DEATH_FX_H__