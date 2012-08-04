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
    VBCREATESTRUCT();

	virtual void Read(HMESSAGEREAD hMessage);

    LTBOOL      bFogEnable;
    LTFLOAT     fFogFarZ;
    LTFLOAT     fFogNearZ;
    LTVector    vFogColor;
    LTVector    vTintColor;
    LTVector    vLightAdd;
	uint8		nSoundFilterId;
	LTBOOL		bCanPlayMoveSnds;
};

inline VBCREATESTRUCT::VBCREATESTRUCT()
{
    bFogEnable		= LTFALSE;
	fFogFarZ		= 0.0f;
	fFogNearZ		= 0.0f;
	nSoundFilterId	= 0;
	vFogColor.Init();
	vTintColor.Init();
	vLightAdd.Init();
	bCanPlayMoveSnds = LTTRUE;
}

inline void VBCREATESTRUCT::Read(HMESSAGEREAD hMessage)
{
	bFogEnable  = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
	fFogFarZ	= g_pLTClient->ReadFromMessageFloat(hMessage);
	fFogNearZ	= g_pLTClient->ReadFromMessageFloat(hMessage);
	g_pLTClient->ReadFromMessageVector(hMessage, &vFogColor);
	g_pLTClient->ReadFromMessageVector(hMessage, &vTintColor);
	g_pLTClient->ReadFromMessageVector(hMessage, &vLightAdd);
	nSoundFilterId	 = g_pLTClient->ReadFromMessageByte(hMessage);
	bCanPlayMoveSnds = g_pLTClient->ReadFromMessageByte(hMessage);
}

class CVolumeBrushFX : public CSpecialFX
{
	public :

		CVolumeBrushFX() : CSpecialFX()
		{
			m_vFogColor.Init();
			m_vTintColor.Init(255.0f, 255.0f, 255.0f);
			m_vLightAdd.Init();

            m_bFogEnable		= LTFALSE;
			m_fFogFarZ			= 0.0f;
			m_fFogNearZ			= 0.0f;
			m_nSoundFilterId	= 0;
			m_bCanPlayMoveSnds	= LTTRUE;
		}

        virtual LTBOOL Update() { return !m_bWantRemove; }

        LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
            if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

			VBCREATESTRUCT* pVB = (VBCREATESTRUCT*)psfxCreateStruct;

			m_bFogEnable		= pVB->bFogEnable;
			m_fFogFarZ			= pVB->fFogFarZ;
			m_fFogNearZ			= pVB->fFogNearZ;
			m_vFogColor			= pVB->vFogColor;
			m_vTintColor		= pVB->vTintColor;
			m_vLightAdd			= pVB->vLightAdd;
			m_nSoundFilterId	= pVB->nSoundFilterId;
			m_bCanPlayMoveSnds	= pVB->bCanPlayMoveSnds;

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
                    uint32 dwFlags = pClientDE->GetObjectFlags(m_hServerObject);
					pClientDE->SetObjectFlags(m_hServerObject, dwFlags | FLAG_RAYHIT);
				}
			}

            return LTTRUE;
		}

        LTBOOL		IsFogEnable()			const { return m_bFogEnable; }
        LTVector	GetFogColor()			const { return m_vFogColor; }
        LTFLOAT		GetFogFarZ()			const { return m_fFogFarZ; }
        LTFLOAT		GetFogNearZ()			const { return m_fFogNearZ; }
        LTVector	GetTintColor()			const { return m_vTintColor; }
        LTVector	GetLightAdd()			const { return m_vLightAdd; }
		uint8		GetSoundFilterId()		const { return m_nSoundFilterId; }
		LTBOOL		CanPlayMovementSounds() const { return m_bCanPlayMoveSnds; }

		virtual uint32 GetSFXID() { return SFX_VOLUMEBRUSH_ID; }

	protected :

        LTBOOL      m_bFogEnable;
        LTFLOAT     m_fFogFarZ;
        LTFLOAT     m_fFogNearZ;
        LTVector    m_vFogColor;
        LTVector    m_vTintColor;
        LTVector    m_vLightAdd;
		uint8		m_nSoundFilterId;
		LTBOOL		m_bCanPlayMoveSnds;
};

#endif // __VOLUME_BRUSH_FX_H__