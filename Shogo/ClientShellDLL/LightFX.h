// ----------------------------------------------------------------------- //
//
// MODULE  : LightFX.h
//
// PURPOSE : LightFX Inventory Item
//
// CREATED : 02/03/98
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHT_FX_H__
#define __LIGHT_FX_H__

#include "SpecialFX.h"
#include "ClientServerShared.h"


struct LIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	LIGHTCREATESTRUCT::LIGHTCREATESTRUCT();

	LTVector		vColor;
	uint32		dwLightFlags;
	LTFLOAT		fIntensityMin;
	LTFLOAT		fIntensityMax;
	uint8		nIntensityWaveform;
	LTFLOAT		fIntensityFreq;
	LTFLOAT		fIntensityPhase;
	LTFLOAT		fRadiusMin;
	LTFLOAT		fRadiusMax;
	uint8		nRadiusWaveform;
	LTFLOAT		fRadiusFreq;
	LTFLOAT		fRadiusPhase;
	HSTRING		hstrRampUpSound;
	HSTRING		hstrRampDownSound;
};

inline LIGHTCREATESTRUCT::LIGHTCREATESTRUCT()
{
	memset(this, 0, sizeof(LIGHTCREATESTRUCT));
}


class CLightFX : public CSpecialFX
{
	public :

		CLightFX() : CSpecialFX() 
		{
			m_fStartTime	= -1.0f;

			VEC_INIT(m_vColor);

			m_fIntensityMin			    = 0.5f;
			m_fIntensityMax			    = 1.0f;
    
			m_fRadiusMin			    = 500.0f;   // default Radius
			m_fRadiusMax			    = 0.0f;

			m_fLifeTime			        = -1.0f;

			m_vColor.x = m_vColor.y = m_vColor.z = 255.0f;

			m_fCurrentRadius         = 0.0f;
    
			m_fIntensityTime         = 0.0f;
			m_fRadiusTime            = 0.0f;
    
			m_fStartTime             = 0.0f;

			m_bDynamic				 = LTTRUE;

			m_hstrRampUpSound		 = LTNULL;
			m_hstrRampDownSound		 = LTNULL;

			m_fHitPts				 = 1.0f;
		}

		~CLightFX()
		{
			if (m_pClientDE)
			{
				if (m_hstrRampUpSound)
				{
					m_pClientDE->FreeString(m_hstrRampUpSound);
				}
				if (m_hstrRampDownSound)
				{
					m_pClientDE->FreeString(m_hstrRampDownSound);
				}
			}
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected:

        void SetRadius(LTFLOAT fRadius);
        void SetColor(LTFLOAT fRedValue, LTFLOAT fGreenValue, LTFLOAT fBlueValue);
        
        virtual void UpdateLightRadius();
        virtual void UpdateLightIntensity();
        virtual void PlayRampSound(int nDirection);

	private :
    
		// Member Variables

		LTVector m_vColor;				    // First color to use

		uint32	m_dwLightFlags;

		LTFLOAT  m_fIntensityMin;			// How Dark light gets
		LTFLOAT  m_fIntensityMax;			// How Bright light gets
		uint8	m_nIntensityWaveform;
		LTFLOAT	m_fIntensityFreq;
		LTFLOAT	m_fIntensityPhase;

		uint32  m_nNumRadiusCycles;		    // Number of times to cycle through
		LTFLOAT  m_fRadiusMin;				// How small light gets
		LTFLOAT  m_fRadiusMax;				// How large light gets
		uint8	m_nRadiusWaveform;
		LTFLOAT	m_fRadiusFreq;
		LTFLOAT	m_fRadiusPhase;

		LTFLOAT  m_fLifeTime;				// How long should this light stay around

		LTVector m_vCurrentColor;			// Color currently using
		LTFLOAT	m_fCurrentRadius;		    // Radius currently using

		LTFLOAT	m_fIntensityTime;		    // Intensity timer
		LTFLOAT	m_fRadiusTime;			    // Radius timer
		LTFLOAT	m_fColorTime;			    // Color timer

		int	    m_nCurIntensityState;	    // What intensity state are we in
		int	    m_nCurRadiusState;	        // What radius state are we in
        
        HSTRING m_hstrRampUpSound;           // Sounds for RampUp and RampDown
        HSTRING m_hstrRampDownSound;         

		LTBOOL	m_bDynamic;					// Was this object dynamically create

		LTFLOAT	m_fStartTime;			    // When did this light get created

        LTFLOAT  m_fRadius;

		LTFLOAT	m_fHitPts;
//		CDestructable m_damage;
};

#endif // __LIGHT_FX_H__


