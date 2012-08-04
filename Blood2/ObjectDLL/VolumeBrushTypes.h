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

class Water : public VolumeBrush
{
	public:
		Water() : VolumeBrush() { m_dwFlags |= FLAG_RAYHIT; m_eContainerCode = CC_WATER; }
};


class Blood : public VolumeBrush
{
	public:
		Blood() : VolumeBrush() { m_dwFlags |= FLAG_RAYHIT; m_eContainerCode = CC_BLOOD; }
};


class Acid : public VolumeBrush
{
	public:
		Acid() : VolumeBrush() { m_dwFlags |= FLAG_RAYHIT; m_eContainerCode = CC_ACID; }
};


class Ladder : public VolumeBrush
{	
	public:
		Ladder() : VolumeBrush() { m_eContainerCode = CC_LADDER; }
	protected:
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
};


class Conveyor : public VolumeBrush
{	
	public:
		Conveyor() : VolumeBrush() { m_eContainerCode = CC_CONVEYOR; }
	protected:
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
};


// Level-specific items

class CraneControl : public VolumeBrush
{	
	public:	
		CraneControl() 
		{ 
			m_hCameraObj = DNULL; 
			m_hCrane = DNULL; 
			m_hBall		= DNULL;
			m_fCamYaw = 0.0f;
			m_eContainerCode = CC_CRANECONTROL;
		}
	protected:
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);

		HOBJECT m_hCameraObj;
		HOBJECT m_hCrane;
		HOBJECT m_hBall;
		DFLOAT	m_fCamYaw;
};


class Damage : public VolumeBrush
{	
	public:
		Damage() : VolumeBrush() { m_eContainerCode = CC_DAMAGE; }
};


class Minefield : public VolumeBrush
{	
	public:
		Minefield() : VolumeBrush() { m_eContainerCode = CC_MINEFIELD; }

		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void   Damage( HOBJECT hObj );

	protected:
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
		void	AddExplosion(DVector &vPos);
};




class FreeFall : public VolumeBrush
{	
	public:
		FreeFall() : VolumeBrush() { m_eContainerCode = CC_FREEFALL; m_fGravity = 0; }
};



#endif // __VOLUME_BRUSH_TYPES_H__
