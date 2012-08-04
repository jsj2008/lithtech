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

#include "cpp_engineobjects_de.h"
#include "Destructable.h"
#include "ClientSFX.h"

class ClientLightFX : public CClientSFX
{
	public :

 		ClientLightFX();
		~ClientLightFX();

		DBOOL Init();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
        DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

        void    HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
        DBOOL   ReadProp(ObjectCreateStruct *pData);
        void    PostPropRead(ObjectCreateStruct *pStruct);
        DBOOL	InitialUpdate(DVector *pMovement);
        DBOOL   Update();

		void	SendEffectMessage();

	public : 

		// Member Variables

		DBOOL   m_bOn;				        // Are we on?

		DVector m_vColor;				    // First color to use

		DFLOAT  m_fIntensityMin;			// How Dark light gets
		DFLOAT  m_fIntensityMax;			// How Bright light gets
		DFLOAT	m_fIntensityPhase;
		DFLOAT	m_fIntensityFreq;
		DBYTE	m_nIntensityWaveform;

		DFLOAT  m_fRadiusMin;				// How small light gets
		DFLOAT  m_fRadiusMax;				// How large light gets
		DFLOAT	m_fRadiusPhase;
		DFLOAT	m_fRadiusFreq;
		DBYTE	m_nRadiusWaveform;

		DFLOAT  m_fLifeTime;				// How long should this light stay around
       
        HSTRING m_hstrRampUpSound;           // Sounds for RampUp and RampDown
        HSTRING m_hstrRampDownSound;         

		DBOOL	m_bDynamic;					// Was this object dynamically create

	private :
    
		DFLOAT	m_fStartTime;			    // When did this light get created

		DDWORD	m_dwLightFlags;

		DFLOAT	m_fHitPts;
		CDestructable m_damage;

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();
};


// Additional light wave classes
class SquareWaveLightFX : public ClientLightFX
{
};


class SawWaveLightFX : public ClientLightFX
{
};


class RampUpWaveLightFX : public ClientLightFX
{
};


class RampDownWaveLightFX : public ClientLightFX
{
};


class SineWaveLightFX : public ClientLightFX
{
};


class Flicker1WaveLightFX : public ClientLightFX
{
};


class Flicker2WaveLightFX : public ClientLightFX
{
};


class Flicker3WaveLightFX : public ClientLightFX
{
};


class Flicker4WaveLightFX : public ClientLightFX
{
};


class StrobeWaveLightFX : public ClientLightFX
{
};


class SearchWaveLightFX : public ClientLightFX
{
};


class FlickerLight : public ClientLightFX
{
};


class GlowingLight : public ClientLightFX
{
};




#endif // __LIGHT_FX_H__


