// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingWorldModel.h
//
// PURPOSE : RotatingWorldModel definition
//
// CREATED : 10/27/97
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATING_WORLD_MODEL_H__
#define __ROTATING_WORLD_MODEL_H__

#include "DestructibleModel.h"
#include "GameBase.h"
#include "keyframer_light.h"

#define MAX_RWM_LIGHT_STEPS	128
#define MAX_RWM_LIGHT_ANIMS 16


class CRotatingWorldModelPlugin : public IObjectPlugin
{
    virtual LTRESULT PreHook_Light(
        ILTPreLight *pInterface,
		HPREOBJECT hObject);

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected :
	  CDestructibleModelPlugin m_DestructibleModelPlugin;
};


class RotatingWorldModel : public GameBase
{
	public :

		RotatingWorldModel();
		virtual ~RotatingWorldModel();

	protected :

		enum RWMState { RWM_OFF, RWM_NORMAL, RWM_SPINUP, RWM_SPINDOWN };

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        LTVector m_vVelocity;        // Current Rotation velocity
        LTVector m_vSaveVelocity;    // Normal Rotation velocity
        LTVector m_vSign;            // Direction of rotation
        LTVector m_vSpinUpTime;      // Time to go from zero to full velocity
        LTVector m_vSpinDownTime;    // Time to go from full to zero velocity
        LTVector m_vSpinTimeLeft;    // How much time left to spin up/down

        LTFLOAT  m_fLastTime;        // Last update time
        LTFLOAT  m_fStartTime;       // When did we start the current state

        LTFLOAT  m_fPitch;           // Object pitch (X)
        LTFLOAT  m_fYaw;             // Object yaw (Y)
        LTFLOAT  m_fRoll;            // Object roll (Z)

		RWMState m_eState;			// What state are we in.

		HSTRING	m_hstrBusySound;	// Sound played while rotating
		HSTRING	m_hstrSpinUpSound;	// Sound played when spinning up
		HSTRING	m_hstrSpinDownSound;// Sound played when spinning down

        HLTSOUND m_sndLastSound;    // Handle of last sound playing
        LTFLOAT   m_fSoundRadius;    // Radius of sound

        LTBOOL    m_bBoxPhysics;     // Use box physics

		HSTRING		m_hShadowLightsString;	// List of shadow lights
		HLIGHTANIM	m_hLightAnims[MAX_RWM_LIGHT_ANIMS];
											// Light animations
        uint32      m_nLightAnims;          // Number of light animations
        uint32      m_nLightFrames;         // Number of frames per animation
        uint32      m_nShadowAxis;          // Which axis of rotation for shadows
        LTBOOL       m_bFirstUpdate;

		void UpdateNormalRotation();
		void UpdateSpinUp();
		void UpdateSpinDown();

		void SetNormalRotation();
		void SetSpinUp();
		void SetSpinDown();
		void SetOff();

        void StartSound(HSTRING hstrSoundName, LTBOOL bLoop);

	private :

		CDestructibleModel m_damage;

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);
		void	CacheFiles();

        LTBOOL   ReadProp(ObjectCreateStruct *pInfo);
		void	InitialUpdate();
		void	Update();
		void	FirstUpdate();
		void	SetLightAnimPercent(float fPercent);

		void	HandleTrigger(HOBJECT hSender, const char* szMsg);

        void    SetupLoopedLightAnimPosition(LAInfo &info, uint32 nTotalFrames, float fPercent);
		void	SetLightAnimRemoved();
};

void RWMPreprocessorCB(HPREOBJECT hObject, ILTPreLight *pInterface);

#endif // __ROTATING_WORLD_MODEL_H__