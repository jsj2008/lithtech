// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushFX.h
//
// PURPOSE : VolumeBrush special fx class - Definition
//
// CREATED : 4/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_FX_H__
#define __VOLUME_BRUSH_FX_H__

#include "SpecialFX.h"

struct VBCREATESTRUCT : public SFXCREATESTRUCT
{
	VBCREATESTRUCT::VBCREATESTRUCT();

	DBOOL		bFogEnable;
	DFLOAT		fFogFarZ;
	DFLOAT		fFogNearZ;
	DVector		vFogColor;
};

inline VBCREATESTRUCT::VBCREATESTRUCT()
{
	memset(this, 0, sizeof(VBCREATESTRUCT));
}


class CVolumeBrushFX : public CSpecialFX
{
	public :

		CVolumeBrushFX() : CSpecialFX() 
		{
			VEC_INIT(m_vFogColor);
			m_bFogEnable	= DFALSE;
			m_fFogFarZ		= 0.0f;	
			m_fFogNearZ		= 0.0f;	
		}

		virtual DBOOL Update() { return DTRUE; }

		DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
			if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

			VBCREATESTRUCT* pVB = (VBCREATESTRUCT*)psfxCreateStruct;

			m_bFogEnable	= pVB->bFogEnable;
			m_fFogFarZ		= pVB->fFogFarZ;	
			m_fFogNearZ		= pVB->fFogNearZ;	
			VEC_COPY(m_vFogColor, pVB->vFogColor);

			return DTRUE;
		}

		DBOOL   IsFogEnable()  const { return m_bFogEnable; }
		DVector GetFogColor()  const { return m_vFogColor; }
		DFLOAT	GetFogFarZ()   const { return m_fFogFarZ; }
		DFLOAT	GetFogNearZ()  const { return m_fFogNearZ; }

	protected :

		DBOOL		m_bFogEnable;
		DFLOAT		m_fFogFarZ;
		DFLOAT		m_fFogNearZ;
		DVector		m_vFogColor;
};

#endif // __VOLUME_BRUSH_FX_H__