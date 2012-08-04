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
#include "ContainerCodes.h"

struct VBCREATESTRUCT : public SFXCREATESTRUCT
{
	VBCREATESTRUCT::VBCREATESTRUCT();

	LTBOOL		bFogEnable;
	LTFLOAT		fFogFarZ;
	LTFLOAT		fFogNearZ;
	LTVector		vFogColor;
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
			m_bFogEnable	= LTFALSE;
			m_fFogFarZ		= 0.0f;	
			m_fFogNearZ		= 0.0f;	
		}

		virtual LTBOOL Update() { return LTTRUE; }

		LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
			if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

			VBCREATESTRUCT* pVB = (VBCREATESTRUCT*)psfxCreateStruct;

			m_bFogEnable	= pVB->bFogEnable;
			m_fFogFarZ		= pVB->fFogFarZ;	
			m_fFogNearZ		= pVB->fFogNearZ;	
			VEC_COPY(m_vFogColor, pVB->vFogColor);

			return LTTRUE;
		}

		LTBOOL CreateObject(ILTClient* pClientDE) 
		{
			if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

			// Special case for liquid volume brushes, set the FLAG_RAYHIT
			// flag...

			uint16 code;
			if (m_hServerObject && pClientDE->GetContainerCode(m_hServerObject, &code))
			{
				if (IsLiquid((ContainerCode)code))
				{
					uint32 dwFlags;
					pClientDE->Common()->GetObjectFlags(m_hServerObject, OFT_Flags, dwFlags);
					dwFlags |= FLAG_RAYHIT;
					pClientDE->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
				}
			}

			return LTTRUE;
		}

		LTBOOL   IsFogEnable()  const { return m_bFogEnable; }
		LTVector GetFogColor()  const { return m_vFogColor; }
		LTFLOAT	GetFogFarZ()   const { return m_fFogFarZ; }
		LTFLOAT	GetFogNearZ()  const { return m_fFogNearZ; }

	protected :

		LTBOOL		m_bFogEnable;
		LTFLOAT		m_fFogFarZ;
		LTFLOAT		m_fFogNearZ;
		LTVector		m_vFogColor;
};

#endif // __VOLUME_BRUSH_FX_H__