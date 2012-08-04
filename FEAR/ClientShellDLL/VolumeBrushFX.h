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
#include "SoundFilterDB.h"
#include "ForceVolume.h"

struct VBCREATESTRUCT : public SFXCREATESTRUCT
{
    VBCREATESTRUCT();

	virtual void Read(ILTMessage_Read *pMsg);

    bool      bFogEnable;
    float     fFogFarZ;
    float     fFogNearZ;
    LTVector    vFogColor;
    LTVector    vTintColor;
    LTVector    vLightAdd;
	HRECORD		hSoundFilterRecord;
	bool		bCanPlayMoveSnds;
	SurfaceType	eSurfaceOverrideType;
	bool		bAllowSwimming;

	LTVector	vCurrent;
	float		fGravity;
	float		fViscosity;
	float		fFriction;
	PlayerPhysicsModel ePPhysicsModel;

	bool		bForceVolume;
	LTVector	vForceDir;	
	float		fForceMag;
	float		fWaveAmplitude;
	float		fWaveFrequency;
	float		fWaveBaseOffset;
	float		fDensity;
	float		fLinearDrag;
	float		fAngularDrag;
};

inline VBCREATESTRUCT::VBCREATESTRUCT()
{
    bFogEnable				= false;
	fFogFarZ				= 0.0f;
	fFogNearZ				= 0.0f;
	hSoundFilterRecord		= NULL;
	bCanPlayMoveSnds		= true;
	eSurfaceOverrideType	= ST_UNKNOWN;
	bAllowSwimming			= true;

	vFogColor.Init();
	vTintColor.Init();
	vLightAdd.Init();

	vCurrent.Init();
	fGravity				= 0.0f;
	fViscosity				= 0.0f;
	fFriction				= 0.0f;
	ePPhysicsModel			= PPM_NORMAL;

	bForceVolume			= false;
	vForceDir.Init(0.0f, 1.0f, 0.0f);	
	fForceMag				= 0.0f;
	fWaveAmplitude			= 0.0f;
	fWaveFrequency			= 1.0f;
	fWaveBaseOffset			= 1.0f;
	fDensity				= 0.0f;
	fLinearDrag				= 0.01f;
	fAngularDrag			= 1.0f;
}

inline void VBCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	bFogEnable				= pMsg->Readbool();
	fFogFarZ				= pMsg->Readfloat();
	fFogNearZ				= pMsg->Readfloat();
	vFogColor				= pMsg->ReadLTVector();
	vTintColor				= pMsg->ReadLTVector();
	vLightAdd				= pMsg->ReadLTVector();
	hSoundFilterRecord		= pMsg->ReadDatabaseRecord(g_pLTDatabase, SoundFilterDB::Instance().GetSoundFilterCategory());
	bCanPlayMoveSnds		= pMsg->Readbool();
	bAllowSwimming			= pMsg->Readbool( );
	eSurfaceOverrideType	= (SurfaceType)pMsg->Readuint8();

	vCurrent				= pMsg->ReadLTVector();
	fGravity				= pMsg->Readfloat();
	fViscosity				= pMsg->Readfloat();
	fFriction				= pMsg->Readfloat();
	ePPhysicsModel			= (PlayerPhysicsModel)pMsg->Readuint8();

	bForceVolume			= pMsg->Readbool();
	if(bForceVolume)
	{
		vForceDir			= pMsg->ReadLTVector();
		fForceMag			= pMsg->Readfloat();
		fWaveAmplitude		= pMsg->Readfloat();
		fWaveFrequency		= pMsg->Readfloat();
		fWaveBaseOffset		= pMsg->Readfloat();
		fDensity			= pMsg->Readfloat();
		fLinearDrag			= pMsg->Readfloat();
		fAngularDrag		= pMsg->Readfloat();
	}
}

class CVolumeBrushFX : public CSpecialFX
{
	public :

		CVolumeBrushFX() : CSpecialFX()
		{
			m_vFogColor.Init();
			m_vTintColor.Init(255.0f, 255.0f, 255.0f);
			m_vLightAdd.Init();

            m_bFogEnable			= false;
			m_fFogFarZ				= 0.0f;
			m_fFogNearZ				= 0.0f;
			m_hSoundFilterRecord	= 0;
			m_bCanPlayMoveSnds		= true;
			m_eSurfaceOverrideType	= ST_UNKNOWN;
		}

        virtual bool Update() 
		{
			//don't update if we aren't visible (i.e. are deactivated on the server)
			uint32 nUserFlags;
			g_pLTClient->Common()->GetObjectFlags(m_hServerObject, OFT_User, nUserFlags);

			if(nUserFlags & USRFLG_VISIBLE)
				m_ForceVolume.Update(SimulationTimer::Instance().GetTimerElapsedS());

			return !m_bWantRemove; 
		}

        bool Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
            if (!CSpecialFX::Init(psfxCreateStruct)) return false;

			VBCREATESTRUCT* pVB = (VBCREATESTRUCT*)psfxCreateStruct;

			m_bFogEnable			= pVB->bFogEnable;
			m_fFogFarZ				= pVB->fFogFarZ;
			m_fFogNearZ				= pVB->fFogNearZ;
			m_vFogColor				= pVB->vFogColor;
			m_vTintColor			= pVB->vTintColor;
			m_vLightAdd				= pVB->vLightAdd;
			m_hSoundFilterRecord	= pVB->hSoundFilterRecord;
			m_bCanPlayMoveSnds		= pVB->bCanPlayMoveSnds;
			m_bAllowSwimming		= pVB->bAllowSwimming;
			m_eSurfaceOverrideType	= pVB->eSurfaceOverrideType;

			m_vCurrent				= pVB->vCurrent;
			m_fGravity				= pVB->fGravity;
			m_fViscosity			= pVB->fViscosity;
			m_fFriction				= pVB->fFriction;
			m_ePPhysicsModel		= pVB->ePPhysicsModel;

			m_bForceVolume			= pVB->bForceVolume;
			m_vForceDir				= pVB->vForceDir;
			m_fForceMag				= pVB->fForceMag;
			m_fWaveAmplitude		= pVB->fWaveAmplitude;
			m_fWaveFrequency		= pVB->fWaveFrequency;
			m_fWaveBaseOffset		= pVB->fWaveBaseOffset;
			m_fDensity				= pVB->fDensity;
			m_fLinearDrag			= pVB->fLinearDrag;
			m_fAngularDrag			= pVB->fAngularDrag;

			uint16 nCode;
			g_pLTClient->GetContainerCode(m_hServerObject, &nCode);
			m_eContainerCode		= (ContainerCode)nCode;

            return true;
		}

        bool CreateObject(ILTClient* pClientDE)
		{
            if (!CSpecialFX::CreateObject(pClientDE)) return false;

			// Special case for liquid volume brushes, set the FLAG_RAYHIT
			// flag...

			if (m_hServerObject && IsLiquid(GetCode()))
			{
				g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);
			}

			if(m_bForceVolume)
			{
				m_ForceVolume.Init(m_hServerObject, m_fDensity, m_vForceDir, m_fForceMag, m_fWaveFrequency, 
									m_fWaveAmplitude, m_fWaveBaseOffset, m_fLinearDrag, m_fAngularDrag);
				m_ForceVolume.SetActive(true);
			}

            return true;
		}

		bool OnServerMessage( ILTMessage_Read *pMsg )
		{ 
			if( !CSpecialFX::OnServerMessage( pMsg ))
				return false;

			uint8 nMsgId = pMsg->ReadBits( FNumBitsExclusive<kVolumeBrush_NumMsgs>::k_nValue );

			switch( nMsgId )
			{
				case kVolumeBrush_AllowSwimming:
				{
					m_bAllowSwimming = pMsg->Readbool( );
				}
				break;

				default:
				break;
			}

			return true;
		}

        bool		IsFogEnable()			const { return m_bFogEnable; }
        LTVector	GetFogColor()			const { return m_vFogColor; }
        float		GetFogFarZ()			const { return m_fFogFarZ; }
        float		GetFogNearZ()			const { return m_fFogNearZ; }
        LTVector	GetTintColor()			const { return m_vTintColor; }
        LTVector	GetLightAdd()			const { return m_vLightAdd; }
		HRECORD		GetSoundFilterRecord()		const { return m_hSoundFilterRecord; }
		bool		CanPlayMovementSounds() const { return m_bCanPlayMoveSnds; }
		SurfaceType GetSurfaceOverride()	const { return m_eSurfaceOverrideType; }
		bool		IsSwimmingAllowed( )	const { return m_bAllowSwimming; }

        float			GetGravity()    const { return m_fGravity; }
        float			GetFriction()   const { return m_fFriction; }
        float			GetViscosity()  const { return m_fViscosity; }
		ContainerCode	GetCode()		const { return m_eContainerCode; }
        LTVector		GetCurrent()    const { return m_vCurrent; }
		PlayerPhysicsModel GetPhysicsModel() const { return m_ePPhysicsModel; }

		virtual uint32 GetSFXID() { return SFX_VOLUMEBRUSH_ID; }

	protected :

        bool      m_bFogEnable;
        float     m_fFogFarZ;
        float     m_fFogNearZ;
        LTVector    m_vFogColor;
        LTVector    m_vTintColor;
        LTVector    m_vLightAdd;
		HRECORD		m_hSoundFilterRecord;
		bool		m_bCanPlayMoveSnds;
		bool		m_bAllowSwimming;
		SurfaceType	m_eSurfaceOverrideType;

		LTVector	m_vCurrent;
		float		m_fGravity;
		float		m_fViscosity;
		float		m_fFriction;
		PlayerPhysicsModel m_ePPhysicsModel;

		bool		m_bForceVolume;
		LTVector	m_vForceDir;	
		float		m_fForceMag;
		float		m_fWaveAmplitude;
		float		m_fWaveFrequency;
		float		m_fWaveBaseOffset;
		float		m_fDensity;
		float		m_fLinearDrag;
		float		m_fAngularDrag;

		CForceVolume	m_ForceVolume;

		ContainerCode	m_eContainerCode;
};

#endif // __VOLUME_BRUSH_FX_H__