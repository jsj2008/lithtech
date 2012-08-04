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
#include "ClientUtilities.h"

struct CAMCREATESTRUCT : public SFXCREATESTRUCT
{
    CAMCREATESTRUCT();

    bool	bAllowPlayerMovement;
    uint8   nCameraType;
    bool	bIsListener;
	float	fFovAspectScale;
	float	fFovY;
};

inline CAMCREATESTRUCT::CAMCREATESTRUCT()
{
    bAllowPlayerMovement    = false;
	nCameraType				= 0;
    bIsListener             = false;
	fFovAspectScale			= 1.0f;
	fFovY					= 0.0f;
}

class CCameraFX : public CSpecialFX
{
	public :

		CCameraFX() : CSpecialFX()
		{
            m_bAllowPlayerMovement  = false;
			m_nCameraType			= 0;
            m_bIsListener           = false;
			m_fFovY					= 0.0f;
			m_fFovAspectScale		= 1.0f;
			m_fPrevFovY				= 0.0f;
			m_fPrevFovAspectScale	= 1.0f;
			m_fFovTime				= 0.0f;
			m_fFovCurrTime			= 0.0f;
		}

        virtual bool Update() { return !m_bWantRemove; }
		
		virtual bool OnServerMessage(ILTMessage_Read *pMsg)
		{
			if (!CSpecialFX::OnServerMessage(pMsg)) return false;

			uint8 nMsgId = pMsg->Readuint8();

			switch(nMsgId)
			{
				case CAMFX_FOV:
				{
					m_fFovCurrTime			= 0.0f;
					m_fPrevFovY				= m_fFovY;
					m_fPrevFovAspectScale	= m_fFovAspectScale;

					m_fFovY					= pMsg->Readfloat();
					m_fFovAspectScale		= pMsg->Readfloat();

					m_fFovTime				= pMsg->Readfloat();
				}
			}

			return true;
		}

        bool Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
            if (!CSpecialFX::Init(psfxCreateStruct)) return false;

			CAMCREATESTRUCT* pCAM = (CAMCREATESTRUCT*)psfxCreateStruct;

			m_bAllowPlayerMovement	= pCAM->bAllowPlayerMovement;
			m_nCameraType			= pCAM->nCameraType;
			m_bIsListener			= pCAM->bIsListener;
			m_fFovY					= pCAM->fFovY;
			m_fFovAspectScale		= pCAM->fFovAspectScale;
			m_fPrevFovY				= m_fFovY;
			m_fPrevFovAspectScale	= m_fFovAspectScale;
			m_fFovTime				= 0.0f;
			m_fFovCurrTime			= 0.0f;

            return true;
		}

        bool   AllowPlayerMovement()   const { return m_bAllowPlayerMovement; }
        uint8    GetType()               const { return m_nCameraType; }
        bool   IsListener()            const { return m_bIsListener; }
		
		void UpdateFOV()
		{
			m_fFovCurrTime += RealTimeTimer::Instance().GetTimerElapsedS( );
		}

		float  GetFovAspectScale() const
		{
			if( m_fFovCurrTime < m_fFovTime)
			{
				float t = m_fFovCurrTime / m_fFovTime;
				return LTLERP( m_fPrevFovAspectScale, m_fFovAspectScale, t);
			}

			return m_fFovAspectScale; 
		}
		
		float  GetFovY() const
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

		uint8   m_nCameraType;
        bool	m_bAllowPlayerMovement;
        bool	m_bIsListener;
		float	m_fFovAspectScale;
		float	m_fFovY;
		float	m_fPrevFovAspectScale;
		float	m_fPrevFovY;
		float	m_fFovTime;
		float	m_fFovCurrTime;
};

#endif // __CAMERA_FX_H__