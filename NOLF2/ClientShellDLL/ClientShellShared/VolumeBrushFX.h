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
#include "SurfaceDefs.h"

struct VBCREATESTRUCT : public SFXCREATESTRUCT
{
    VBCREATESTRUCT();

	virtual void Read(ILTMessage_Read *pMsg);

    LTBOOL      bFogEnable;
    LTFLOAT     fFogFarZ;
    LTFLOAT     fFogNearZ;
    LTVector    vFogColor;
    LTVector    vTintColor;
    LTVector    vLightAdd;
	uint8		nSoundFilterId;
	LTBOOL		bCanPlayMoveSnds;
	SurfaceType	eSurfaceOverrideType;

	LTVector	vCurrent;
	float		fGravity;
	float		fViscosity;
	float		fFriction;
	PlayerPhysicsModel ePPhysicsModel;
};

inline VBCREATESTRUCT::VBCREATESTRUCT()
{
    bFogEnable				= LTFALSE;
	fFogFarZ				= 0.0f;
	fFogNearZ				= 0.0f;
	nSoundFilterId			= 0;
	bCanPlayMoveSnds		= LTTRUE;
	eSurfaceOverrideType	= ST_UNKNOWN;

	vFogColor.Init();
	vTintColor.Init();
	vLightAdd.Init();

	vCurrent.Init();
	fGravity				= 0.0f;
	fViscosity				= 0.0f;
	fFriction				= 0.0f;
	ePPhysicsModel			= PPM_NORMAL;
}

inline void VBCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	bFogEnable				= pMsg->Readbool();
	fFogFarZ				= pMsg->Readfloat();
	fFogNearZ				= pMsg->Readfloat();
	vFogColor				= pMsg->ReadLTVector();
	vTintColor				= pMsg->ReadLTVector();
	vLightAdd				= pMsg->ReadLTVector();
	nSoundFilterId			= pMsg->Readuint8();
	bCanPlayMoveSnds		= pMsg->Readbool();
	eSurfaceOverrideType	= (SurfaceType)pMsg->Readuint8();

	vCurrent				= pMsg->ReadLTVector();
	fGravity				= pMsg->Readfloat();
	fViscosity				= pMsg->Readfloat();
	fFriction				= pMsg->Readfloat();
	ePPhysicsModel			= (PlayerPhysicsModel)pMsg->Readuint8();
}

class CVolumeBrushFX : public CSpecialFX
{
	public :

		CVolumeBrushFX() : CSpecialFX()
		{
			m_vFogColor.Init();
			m_vTintColor.Init(255.0f, 255.0f, 255.0f);
			m_vLightAdd.Init();

            m_bFogEnable			= LTFALSE;
			m_fFogFarZ				= 0.0f;
			m_fFogNearZ				= 0.0f;
			m_nSoundFilterId		= 0;
			m_bCanPlayMoveSnds		= LTTRUE;
			m_eSurfaceOverrideType	= ST_UNKNOWN;
		}

        virtual LTBOOL Update() { return !m_bWantRemove; }

        LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
            if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

			VBCREATESTRUCT* pVB = (VBCREATESTRUCT*)psfxCreateStruct;

			m_bFogEnable			= pVB->bFogEnable;
			m_fFogFarZ				= pVB->fFogFarZ;
			m_fFogNearZ				= pVB->fFogNearZ;
			m_vFogColor				= pVB->vFogColor;
			m_vTintColor			= pVB->vTintColor;
			m_vLightAdd				= pVB->vLightAdd;
			m_nSoundFilterId		= pVB->nSoundFilterId;
			m_bCanPlayMoveSnds		= pVB->bCanPlayMoveSnds;
			m_eSurfaceOverrideType	= pVB->eSurfaceOverrideType;

			m_vCurrent				= pVB->vCurrent;
			m_fGravity				= pVB->fGravity;
			m_fViscosity			= pVB->fViscosity;
			m_fFriction				= pVB->fFriction;
			m_ePPhysicsModel		= pVB->ePPhysicsModel;

			uint16 nCode;
			g_pLTClient->GetContainerCode(m_hServerObject, &nCode);
			m_eContainerCode		= (ContainerCode)nCode;

            return LTTRUE;
		}

        LTBOOL CreateObject(ILTClient* pClientDE)
		{
            if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

			// Special case for liquid volume brushes, set the FLAG_RAYHIT
			// flag...

			if (m_hServerObject && IsLiquid(GetCode()))
			{
				g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);
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
		SurfaceType GetSurfaceOverride()	const { return m_eSurfaceOverrideType; }

        LTFLOAT			GetGravity()    const { return m_fGravity; }
        LTFLOAT			GetFriction()   const { return m_fFriction; }
        LTFLOAT			GetViscosity()  const { return m_fViscosity; }
		ContainerCode	GetCode()		const { return m_eContainerCode; }
        LTVector		GetCurrent()    const { return m_vCurrent; }
		PlayerPhysicsModel GetPhysicsModel() const { return m_ePPhysicsModel; }

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
		SurfaceType	m_eSurfaceOverrideType;

		LTVector	m_vCurrent;
		float		m_fGravity;
		float		m_fViscosity;
		float		m_fFriction;
		PlayerPhysicsModel m_ePPhysicsModel;

		ContainerCode	m_eContainerCode;
};

#endif // __VOLUME_BRUSH_FX_H__