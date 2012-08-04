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

class LightFX : public BaseClass
{
	public :

 		LightFX();
		~LightFX();

		void TurnOn();
		void TurnOff();
        void ToggleLight()      { if (m_bOn) TurnOff();  else TurnOn(); }
        void RemoveFromWorld()  { m_bRemoveFromWorld = DTRUE; }

		DBOOL Init();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
        DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

        void    HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
        DBOOL   ReadProp(ObjectCreateStruct *pData);
        void    PostPropRead(ObjectCreateStruct *pStruct);
        DBOOL	InitialUpdate(DVector *pMovement);
        DBOOL   Update();

        void SetRadius(DFLOAT fRadius, DBOOL bForce = DFALSE);
        void SetColor(DFLOAT fRedValue, DFLOAT fGreenValue, DFLOAT fBlueValue);
        
// The following are Light Color related methods //////////////////////////////
        virtual void UpdateLightColor();
        virtual void DecNumColorCycles();
// The following are Light Radius related methods /////////////////////////////
        virtual void UpdateLightRadius();
        virtual void UpdateRadiusMin();
        virtual void UpdateRadiusMax();
        virtual void UpdateRadiusRampUp();
        virtual void UpdateRadiusRampDown();
        virtual void DecNumRadiusCycles();
        virtual void InitRadiusTransition(int NextState);
// The following are Light Intensity related methods //////////////////////////
        virtual void UpdateLightIntensity();
        virtual void UpdateIntensityMin();
        virtual void UpdateIntensityMax();
        virtual void UpdateIntensityRampUp();
        virtual void UpdateIntensityRampDown();
        virtual void DecNumIntensityCycles();
        virtual void InitIntensityTransition(int NextState);
        virtual void PlayRampSound(int nDirection);

	public : 

		// Member Variables

		DBOOL   m_bOn;				        // Are we on?
		DBOOL	m_bRemoveFromWorld;	        // Are we still in the world

		DDWORD  m_nNumColorCycles;		    // Number of times to cycle through
		DVector m_vColor1;				    // First color to use
		DFLOAT  m_fColor1Time;			    // How long we stay on color 1
		DVector m_vColor2;				    // Second color to use
		DFLOAT  m_fColor2Time;			    // How long we stay on color 2
		DVector m_vColor3;				    // Third  color to use
		DFLOAT  m_fColor3Time;			    // How long we stay on color 3

		DDWORD  m_nNumIntensityCycles;	    // Number of times to cycle through
		DFLOAT  m_fIntensityMin;			// How Dark light gets
		DFLOAT  m_fIntensityMax;			// How Bright light gets
		DFLOAT  m_fIntensityMinTime;		// How long light stays darkest
		DFLOAT  m_fIntensityMaxTime;		// How long light stays brightest
		DFLOAT  m_fIntensityRampUpTime;     // How long from dark to light
		DFLOAT  m_fIntensityRampDownTime;   // How long from light to dark

		DDWORD  m_nNumRadiusCycles;		    // Number of times to cycle through
		DFLOAT  m_fRadiusMin;				// How small light gets
		DFLOAT  m_fRadiusMax;				// How large light gets
		DFLOAT  m_fRadiusMinTime;			// How long light stays smallest
		DFLOAT  m_fRadiusMaxTime;			// How long light stays largest
		DFLOAT  m_fRadiusRampUpTime;		// How long from small to large
		DFLOAT  m_fRadiusRampDownTime;	    // How long from large to small

		DFLOAT  m_fLifeTime;				// How long should this light stay around

		DVector m_vCurrentColor;			// Color currently using
		DFLOAT	m_fCurrentRadius;		    // Radius currently using

		DFLOAT	m_fIntensityTime;		    // Intensity timer
		DFLOAT	m_fRadiusTime;			    // Radius timer
		DFLOAT	m_fColorTime;			    // Color timer

		int	    m_nCurIntensityState;	    // What intensity state are we in
		int	    m_nCurRadiusState;	        // What radius state are we in
		int	    m_nCurColorUsed;		    // Which of the colors are we currently using (0 = Init, 1 = Color1, 2 = Color2, 3 = Color3)
        
        HSTRING m_hstrRampUpSound;           // Sounds for RampUp and RampDown
        HSTRING m_hstrRampDownSound;         

		DBOOL	m_bDynamic;					// Was this object dynamically create

	private :
    
		DFLOAT	m_fStartTime;			    // When did this light get created

        DFLOAT  m_fRedValue;
        DFLOAT  m_fGreenValue;
        DFLOAT  m_fBlueValue;
        DFLOAT  m_fRadius;

		DFLOAT	m_fHitPts;
		CDestructable m_damage;
};

#endif // __LIGHT_FX_H__


