// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushTypes.h
//
// PURPOSE : VolumeBrushTypes definition
//
// CREATED : 2/16/98
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_TYPES_H__
#define __VOLUME_BRUSH_TYPES_H__

#include "VolumeBrush.h"

DBOOL LiquidFilterFn(HOBJECT hObj, void *pUserData);

class BlueWater : public VolumeBrush
{
	public:
		BlueWater() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_BLUE_WATER;
			m_dwFlags |= FLAG_RAYHIT; 
		}
};

class DirtyWater : public VolumeBrush
{
	public:
		DirtyWater() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_DIRTY_WATER;
			m_dwFlags |= FLAG_RAYHIT; 
		}
};

class ClearWater : public VolumeBrush
{
	public:
		ClearWater() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_CLEAR_WATER;
			m_dwFlags |= FLAG_RAYHIT; 
		}
};

class CorrosiveFluid : public VolumeBrush
{
	public:
		CorrosiveFluid() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_CORROSIVE_FLUID;
			m_dwFlags |= FLAG_RAYHIT; 
		}
};

class Kato : public VolumeBrush
{
	public:
		Kato() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_KATO;
			m_dwFlags |= FLAG_RAYHIT; 
		}
};

class LiquidNitrogen : public VolumeBrush
{
	public:
		LiquidNitrogen() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_LIQUID_NITROGEN;
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

class Smoke : public VolumeBrush
{
	public:
		Smoke() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_SMOKE;
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

class ZeroGravity : public VolumeBrush
{
	public:
		ZeroGravity() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_ZERO_GRAVITY;
		}
};

class Vacuum : public VolumeBrush
{
	public:
		Vacuum() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_VACUUM;
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

class TotalRed : public VolumeBrush
{
	public:
		TotalRed() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_TOTAL_RED;
		}
};

class TintScreen : public VolumeBrush
{
	public:
		TintScreen() : VolumeBrush() 
		{ 
			m_eContainerCode = CC_TINT_SCREEN;
		}

	protected:
	
		void UpdatePhysics(ContainerPhysics* pCPStruct) {}
		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private:
		DVector m_vColor;
};


#endif // __VOLUME_BRUSH_TYPES_H__
