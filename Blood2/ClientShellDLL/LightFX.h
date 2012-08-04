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

	DVector		vColor;
	DDWORD		dwLightFlags;
	DFLOAT		fIntensityMin;
	DFLOAT		fIntensityMax;
	DBYTE		nIntensityWaveform;
	DFLOAT		fIntensityFreq;
	DFLOAT		fIntensityPhase;
	DFLOAT		fRadiusMin;
	DFLOAT		fRadiusMax;
	DBYTE		nRadiusWaveform;
	DFLOAT		fRadiusFreq;
	DFLOAT		fRadiusPhase;
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

			m_bDynamic				 = DTRUE;

			m_hstrRampUpSound		 = DNULL;
			m_hstrRampDownSound		 = DNULL;

			m_fHitPts				 = 1.0f;
		}

		~CLightFX()
		{
			g_pClientDE->FreeString( m_hstrRampUpSound );
			g_pClientDE->FreeString( m_hstrRampDownSound );
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	protected:

        void SetRadius(DFLOAT fRadius);
        void SetColor(DFLOAT fRedValue, DFLOAT fGreenValue, DFLOAT fBlueValue);
        
        virtual void UpdateLightRadius();
        virtual void UpdateLightIntensity();
        virtual void PlayRampSound(int nDirection);

	private :
    
		// Member Variables

		DVector m_vColor;				    // First color to use

		DDWORD	m_dwLightFlags;

		DFLOAT  m_fIntensityMin;			// How Dark light gets
		DFLOAT  m_fIntensityMax;			// How Bright light gets
		DBYTE	m_nIntensityWaveform;
		DFLOAT	m_fIntensityFreq;
		DFLOAT	m_fIntensityPhase;

		DDWORD  m_nNumRadiusCycles;		    // Number of times to cycle through
		DFLOAT  m_fRadiusMin;				// How small light gets
		DFLOAT  m_fRadiusMax;				// How large light gets
		DBYTE	m_nRadiusWaveform;
		DFLOAT	m_fRadiusFreq;
		DFLOAT	m_fRadiusPhase;

		DFLOAT  m_fLifeTime;				// How long should this light stay around

		DVector m_vCurrentColor;			// Color currently using
		DFLOAT	m_fCurrentRadius;		    // Radius currently using

		DFLOAT	m_fIntensityTime;		    // Intensity timer
		DFLOAT	m_fRadiusTime;			    // Radius timer
		DFLOAT	m_fColorTime;			    // Color timer

		int	    m_nCurIntensityState;	    // What intensity state are we in
		int	    m_nCurRadiusState;	        // What radius state are we in
        
        HSTRING m_hstrRampUpSound;           // Sounds for RampUp and RampDown
        HSTRING m_hstrRampDownSound;         

		DBOOL	m_bDynamic;					// Was this object dynamically create

		DFLOAT	m_fStartTime;			    // When did this light get created

        DFLOAT  m_fRadius;

		DFLOAT	m_fHitPts;
//		CDestructable m_damage;
};

#endif // __LIGHT_FX_H__


