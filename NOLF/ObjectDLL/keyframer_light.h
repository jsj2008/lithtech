
#ifndef __KEYFRAMER_LIGHT_H__
#define __KEYFRAMER_LIGHT_H__


    #include "iltlightanim.h"
    #include "iltprelight.h"


	#define KEYFRAMERLIGHT_CLASSNAME	"KeyframerLight"


	// Used by preprocessor hooks.
	class KLProps
	{
	public:
        LTBOOL   m_bDirLight;
		float	m_fRadius;
		float	m_fFOV;
        LTVector m_vInnerColor;
        LTVector m_vOuterColor;
        LTBOOL   m_bUseShadowMaps;

		// Object position.
        LTVector m_vPos;

		// Object's forward vector.. the keyframer ignores this.
        LTVector m_vForwardVec;
	};


	class KeyframerLight : public BaseClass
	{
	public:

						KeyframerLight();

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		void			PreCreate(ObjectCreateStruct *pStruct);
		void			InitialUpdate();


	public:

		HLIGHTANIM	m_hLightAnim;		// INVALID_LIGHT_ANIM if none.
        uint32      m_nStepsPerSegment; // Number of steps in between KeyFramer keys.

		// Used for shadow-mapped lights.
        LTVector     m_vLightColor;
		float		m_fLightRadius;
	};


	// Get a KeyframerLights's light animation name (based on the object name).
	void GetKLLightAnimName(char *pDest, const char *pObjectName);

	// Read in all the properties (for preprocessor callbacks).
    void ReadKLProps(ILTPreLight *pLightLT, HPREOBJECT hObject, KLProps *pProps);

	// Returns TRUE if hObj is a KeyframerLight.
    LTBOOL IsKeyframerLight(ILTServer *pServerDE, HOBJECT hObj);

	// Sets up m_iFrames and m_fPercentBetween to interpolate through the animation's frames.
    void SetupLightAnimPosition(LAInfo &info, uint32 nTotalFrames, float fPercent);


#endif
