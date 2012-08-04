// ----------------------------------------------------------------------- //
//
// MODULE  : CameraFX.h
//
// PURPOSE : Camera special fx class - Definition
//
// CREATED : 5/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_FX_H__
#define __CAMERA_FX_H__

#include "SpecialFX.h"

struct CAMCREATESTRUCT : public SFXCREATESTRUCT
{
    CAMCREATESTRUCT();

    LTBOOL   bAllowPlayerMovement;
    uint8    nCameraType;
    LTBOOL   bIsListener;
	LTFLOAT	 fFovX;
	LTFLOAT	 fFovY;
};

inline CAMCREATESTRUCT::CAMCREATESTRUCT()
{
    bAllowPlayerMovement    = LTFALSE;
	nCameraType				= 0;
    bIsListener             = LTFALSE;
	fFovX					= 0.0f;
	fFovY					= 0.0f;
}

class CCameraFX : public CSpecialFX
{
	public :

		CCameraFX() : CSpecialFX()
		{
            m_bAllowPlayerMovement  = LTFALSE;
			m_nCameraType			= 0;
            m_bIsListener           = LTFALSE;
			m_fFovX					= 0.0f;
			m_fFovY					= 0.0f;
			m_fPrevFovX				= 0.0f;
			m_fPrevFovY				= 0.0f;
			m_fFovTime				= 0.0f;
			m_fFovCurrTime			= 0.0f;
		}

        virtual LTBOOL Update() { return !m_bWantRemove; }
		
		virtual LTBOOL OnServerMessage(ILTMessage_Read *pMsg)
		{
			if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

			uint8 nMsgId = pMsg->Readuint8();

			switch(nMsgId)
			{
				case CAMFX_FOV:
				{
					m_fPrevFovX				= m_fFovX;
					m_fPrevFovY				= m_fFovY;

					m_fFovX					= pMsg->Readfloat();
					m_fFovY					= pMsg->Readfloat();
					m_fFovTime				= pMsg->Readfloat();

					m_fFovCurrTime			= 0.0f;
				}
			}

			return LTTRUE;
		}

        LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
            if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

			CAMCREATESTRUCT* pCAM = (CAMCREATESTRUCT*)psfxCreateStruct;

			m_bAllowPlayerMovement	= pCAM->bAllowPlayerMovement;
			m_nCameraType			= pCAM->nCameraType;
			m_bIsListener			= pCAM->bIsListener;
			m_fFovX					= pCAM->fFovX;
			m_fFovY					= pCAM->fFovY;
			m_fPrevFovX				= m_fFovX;
			m_fPrevFovY				= m_fFovY;
			m_fFovTime				= 0.0f;
			m_fFovCurrTime			= 0.0f;

            return LTTRUE;
		}

        LTBOOL   AllowPlayerMovement()   const { return m_bAllowPlayerMovement; }
        uint8    GetType()               const { return m_nCameraType; }
        LTBOOL   IsListener()            const { return m_bIsListener; }
		
		void UpdateFOV()
		{
			m_fFovCurrTime += g_pGameClientShell->GetFrameTime();
		}

		LTFLOAT  GetFovX() const
		{
			if( m_fFovCurrTime < m_fFovTime)
			{
				float t = m_fFovCurrTime / m_fFovTime;
				return LTLERP( m_fPrevFovX, m_fFovX, t);
			}

			return m_fFovX; 
		}
		
		LTFLOAT  GetFovY() const
		{
			if( m_fFovCurrTime < m_fFovTime)
			{
				float t = m_fFovCurrTime / m_fFovTime;
				return LTLERP( m_fPrevFovY, m_fFovY, t);
			}

			return m_fFovY; 
		}

		virtual uint32 GetSFXID() { return SFX_CAMERA_ID; }

	protected :

        LTBOOL   m_bAllowPlayerMovement;
        uint8    m_nCameraType;
        LTBOOL   m_bIsListener;
		LTFLOAT	 m_fFovX;
		LTFLOAT	 m_fFovY;
		LTFLOAT  m_fPrevFovX;
		LTFLOAT  m_fPrevFovY;
		LTFLOAT	 m_fFovTime;
		LTFLOAT	 m_fFovCurrTime;
};

#endif // __CAMERA_FX_H__