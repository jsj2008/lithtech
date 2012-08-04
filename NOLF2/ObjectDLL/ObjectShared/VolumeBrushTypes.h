// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushTypes.h
//
// PURPOSE : VolumeBrushTypes definition
//
// CREATED : 2/16/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_TYPES_H__
#define __VOLUME_BRUSH_TYPES_H__

#include "VolumeBrush.h"

LINKTO_MODULE( VolumeBrushTypes );

bool LiquidFilterFn(HOBJECT hObj, void *pUserData);

class Water : public VolumeBrush
{
	public:

		Water() : VolumeBrush()
		{
			m_eContainerCode = CC_WATER;
			m_eSurfaceOverrideType = ST_LIQUID;
			m_dwFlags |= FLAG_RAYHIT;
		}
};

class Ice : public VolumeBrush
{
	public:

		Ice() : VolumeBrush()
		{
			m_eContainerCode = CC_ICE;
			m_dwFlags |= FLAG_RAYHIT;
		}
};

class CorrosiveFluid : public VolumeBrush
{
	public:

		CorrosiveFluid() : VolumeBrush()
		{
			m_eContainerCode = CC_CORROSIVE_FLUID;
			m_eSurfaceOverrideType = ST_LIQUID;
			m_dwFlags |= FLAG_RAYHIT;
		}
};

class FreezingWater : public VolumeBrush
{
	public:

		FreezingWater() : VolumeBrush()
		{
			m_eContainerCode = CC_FREEZING_WATER;
			m_eSurfaceOverrideType = ST_LIQUID;
			m_dwFlags |= FLAG_RAYHIT;
		}
};

class PoisonGas : public VolumeBrush
{
	public:

		PoisonGas() : VolumeBrush()
		{
			m_eContainerCode = CC_POISON_GAS;
		}
};

class Electricity : public VolumeBrush
{
	public:

		Electricity() : VolumeBrush()
		{
			m_eContainerCode = CC_ELECTRICITY;
		}
};

class EndlessFall : public VolumeBrush
{
	public:

		EndlessFall() : VolumeBrush()
		{
			m_eContainerCode = CC_ENDLESS_FALL;
		}
};

class Wind : public VolumeBrush
{
	public:

		Wind() : VolumeBrush()
		{
			m_eContainerCode = CC_WIND;
		}
};

class ColdAir : public VolumeBrush
{
	public:

		ColdAir() : VolumeBrush()
		{
			m_eContainerCode = CC_COLDAIR;
		}
};

class Burn : public VolumeBrush
{
	public:

		Burn() : VolumeBrush()
		{
			m_eContainerCode = CC_BURN;
		}
};

class Filter : public VolumeBrush
{
	public:

		Filter() : VolumeBrush()
		{
			m_eContainerCode = CC_FILTER;
		}
};

class SafteyNet : public VolumeBrush
{
	public:

		SafteyNet() : VolumeBrush()
		{
			m_eContainerCode = CC_SAFTEY_NET;
		}
};

class Ladder : public VolumeBrush
{
	public:

		Ladder() : VolumeBrush()
		{
			m_eContainerCode = CC_LADDER;
		}

	protected:

		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
};


class Weather : public VolumeBrush
{
	public:

		Weather() : VolumeBrush()
		{
			m_eContainerCode = CC_WEATHER;
			m_dwWeatherFlags = 0;
			m_fViewDist		 = 1000.0f;
		}

	protected:

		virtual void UpdatePhysics(ContainerPhysics* pCPStruct) {}

		virtual void ReadProp(ObjectCreateStruct *pStruct);
		virtual void WriteSFXMsg(ILTMessage_Write *pMsg);

	private:

        uint32  m_dwWeatherFlags;
        LTFLOAT  m_fViewDist;
};

class Gravity : public VolumeBrush
{
	public:

		Gravity() : VolumeBrush()
		{
			m_eContainerCode = CC_GRAVITY;
		}


	protected:

		virtual void ReadProp( ObjectCreateStruct *pStruct );
};

#endif // __VOLUME_BRUSH_TYPES_H__