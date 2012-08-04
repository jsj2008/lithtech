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
#include "iltlightanim.h"


struct LIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
    LIGHTCREATESTRUCT();

    LTVector     vColor;
    LTVector     vOffset;
    uint32      dwLightFlags;
    LTFLOAT      fIntensityMin;
    LTFLOAT      fIntensityMax;
    uint8       nIntensityWaveform;
    LTFLOAT      fIntensityFreq;
    LTFLOAT      fIntensityPhase;
    LTFLOAT      fRadiusMin;
    LTFLOAT      fRadiusMax;
    uint8       nRadiusWaveform;
    LTFLOAT      fRadiusFreq;
    LTFLOAT      fRadiusPhase;
	HSTRING		hstrRampUpSound;
	HSTRING		hstrRampDownSound;
	HLIGHTANIM	m_hLightAnim;	// INVALID_LIGHT_ANIM if none..
};

inline LIGHTCREATESTRUCT::LIGHTCREATESTRUCT()
{
	vColor.Init();
	vOffset.Init();
	dwLightFlags		= 0;
	fIntensityMin		= 0.0f;
	fIntensityMax		= 0.0f;
	nIntensityWaveform	= 0;
	fIntensityFreq		= 0.0f;
	fIntensityPhase		= 0.0f;
	fRadiusMin			= 0.0f;
	fRadiusMax			= 0.0f;
	nRadiusWaveform		= 0;
	fRadiusFreq			= 0.0f;
	fRadiusPhase		= 0.0f;
    hstrRampUpSound     = LTNULL;
    hstrRampDownSound   = LTNULL;
    m_hLightAnim        = LTNULL;
}


class CLightFX : public CSpecialFX
{
	public :

		CLightFX();
		~CLightFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		virtual uint32 GetSFXID() { return SFX_LIGHT_ID; }

	protected:

        void SetRadius(LTFLOAT fRadius, LAInfo &info);
        void SetColor(LTFLOAT fRedValue, LTFLOAT fGreenValue, LTFLOAT fBlueValue, LAInfo &info);

        virtual void UpdateLightRadius(LAInfo &info);
        virtual void UpdateLightIntensity(LAInfo &info);
        virtual void PlayRampSound(int nDirection);

	private :

		// Member Variables

        LTVector m_vColor;                   // First color to use
        LTVector m_vOffset;                  // Offset relative to server obj

        uint32  m_dwLightFlags;

        LTFLOAT  m_fIntensityMin;            // How Dark light gets
        LTFLOAT  m_fIntensityMax;            // How Bright light gets
        uint8   m_nIntensityWaveform;
        LTFLOAT  m_fIntensityFreq;
        LTFLOAT  m_fIntensityPhase;

        uint32  m_nNumRadiusCycles;         // Number of times to cycle through
        LTFLOAT  m_fRadiusMin;               // How small light gets
        LTFLOAT  m_fRadiusMax;               // How large light gets
        uint8   m_nRadiusWaveform;
        LTFLOAT  m_fRadiusFreq;
        LTFLOAT  m_fRadiusPhase;

        LTFLOAT  m_fLifeTime;                // How long should this light stay around

        LTVector m_vCurrentColor;            // Color currently using
        LTFLOAT  m_fCurrentRadius;           // Radius currently using

        LTFLOAT  m_fIntensityTime;           // Intensity timer
        LTFLOAT  m_fRadiusTime;              // Radius timer
        LTFLOAT  m_fColorTime;               // Color timer

		int	    m_nCurIntensityState;	    // What intensity state are we in
		int	    m_nCurRadiusState;	        // What radius state are we in

        HSTRING m_hstrRampUpSound;           // Sounds for RampUp and RampDown
        HSTRING m_hstrRampDownSound;

        LTBOOL   m_bUseServerPos;            // Should we use the server pos?
        LTFLOAT  m_fStartTime;               // When did this light get created
        LTFLOAT  m_fRadius;

		HLIGHTANIM	m_hLightAnim;			// INVALID_LIGHT_ANIM if we're not using light animations.
};

#endif // __LIGHT_FX_H__