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

#include "DestructableWorldModel.h"
#include "cpp_engineobjects_de.h"

class RotatingWorldModel : public BaseClass
{
	public :

		RotatingWorldModel();
		virtual ~RotatingWorldModel();

	protected :

		enum RWMState { RWM_OFF, RWM_NORMAL, RWM_SPINUP, RWM_SPINDOWN };

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		DVector m_vVelocity;		// Current Rotation velocity
		DVector m_vSaveVelocity;	// Normal Rotation velocity
		DVector m_vSign;			// Direction of rotation
		DVector m_vSpinUpTime;		// Time to go from zero to full velocity
		DVector m_vSpinDownTime;	// Time to go from full to zero velocity
		DVector m_vSpinTimeLeft;	// How much time left to spin up/down

		DFLOAT	m_fLastTime;		// Last update time
		DFLOAT	m_fStartTime;		// When did we start the current state

		DFLOAT	m_fPitch;			// Object pitch (X)
		DFLOAT	m_fYaw;				// Object yaw (Y)
		DFLOAT	m_fRoll;			// Object roll (Z)

		RWMState m_eState;			// What state are we in.

		HSTRING	m_hstrBusySound;	// Sound played while rotating
		HSTRING	m_hstrSpinUpSound;	// Sound played when spinning up
		HSTRING	m_hstrSpinDownSound;// Sound played when spinning down

		HSOUNDDE m_sndLastSound;	// Handle of last sound playing
		DFLOAT	 m_fSoundRadius;	// Radius of sound

		DBOOL	 m_bBoxPhysics;		// Use box physics
		SurfaceType m_eSurfaceType;

		void UpdateNormalRotation();
		void UpdateSpinUp();
		void UpdateSpinDown();

		void SetNormalRotation();
		void SetSpinUp();
		void SetSpinDown();
		void SetOff();

		void StartSound(HSTRING hstrSoundName, DBOOL bLoop);

	private :

		CDestructableWorldModel m_damage;

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags);
		void	CacheFiles();

		DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		void	InitialUpdate();
		void	Update();

		void	HandleTrigger(HOBJECT hSender, HSTRING hMsg);

};

#endif // __ROTATING_WORLD_MODEL_H__
