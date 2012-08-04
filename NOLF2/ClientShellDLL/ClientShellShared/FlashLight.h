// ----------------------------------------------------------------------- //
//
// MODULE  : FlashLight.h
//
// PURPOSE : FlashLight class - Definition
//
// CREATED : 07/21/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FLASH_LIGHT_H__
#define __FLASH_LIGHT_H__

#include "ltbasedefs.h"
#include "PolyLineFX.h"

class CFlashLight
{
	public :

		CFlashLight();
		virtual ~CFlashLight();

		virtual void Update();

		virtual void Toggle()	{ (m_bOn ? TurnOff() : TurnOn());}
		virtual void TurnOn();
		virtual void TurnOff();

		virtual LTBOOL IsOn() const { return m_bOn; }

	protected :

		virtual void CreateLight();

		virtual void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset) = 0;

		virtual LTBOOL UpdateServer() { return LTTRUE; }

	private :

        LTBOOL   m_bOn;
		HOBJECT	 m_hLight;
};

class CFlashLightPlayer : public CFlashLight
{
	protected :

		void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset);
};

class CFlashLight3rdPerson : public CFlashLight
{
	public :

		CFlashLight3rdPerson();
		~CFlashLight3rdPerson();

		void Init(HOBJECT hObj);

	protected :

		virtual void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset);

		virtual LTBOOL UpdateServer() { return LTFALSE; }

	protected :

		HOBJECT			m_hObj;
};

#endif // __FLASH_LIGHT_H__