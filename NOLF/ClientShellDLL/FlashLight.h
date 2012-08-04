// ----------------------------------------------------------------------- //
//
// MODULE  : FlashLight.h
//
// PURPOSE : FlashLight class - Definition
//
// CREATED : 07/21/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
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

        LTBOOL           m_bOn;
		HOBJECT			 m_hLight;
		CPolyLineFX		 m_LightBeam;

        LTFLOAT          m_fMinLightRadius;
        LTFLOAT          m_fMaxLightRadius;
        LTFLOAT          m_fMaxLightDist;

        LTFLOAT          m_fServerUpdateTimer;
};

class CFlashLightPlayer : public CFlashLight
{
	protected :

		void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset);
};

class CFlashLightAI : public CFlashLight
{
	public :

		CFlashLightAI();
		~CFlashLightAI();

		void Init(HOBJECT hAI);

		void Update();

	protected :

		void GetLightPositions(LTVector & vStartPos, LTVector & vEndPos, LTVector & vUOffset, LTVector & vROffset);

		LTBOOL UpdateServer() { return LTFALSE; }

	protected :

		HOBJECT			m_hAI;
};

#endif // __FLASH_LIGHT_H__