// ----------------------------------------------------------------------- //
//
// MODULE  : Rotating.h
//
// PURPOSE : Rotating definition
//
// CREATED : 4/29/99
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATING_H__
#define __ROTATING_H__

#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"


#define ADD_ROTATING_AGGREGATE() \
	PROP_DEFINEGROUP(RotatingStuff, PF_GROUP1) \
		ADD_BOOLPROP_FLAG(StartOn, DTRUE, PF_GROUP1) \
		ADD_STRINGPROP_FLAG(SpinUpSound, "", PF_GROUP1) \
		ADD_STRINGPROP_FLAG(BusySound, "", PF_GROUP1) \
		ADD_STRINGPROP_FLAG(SpinDownSound, "", PF_GROUP1) \
		ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_GROUP1) \
		ADD_REALPROP_FLAG(XAxisRevTime, 0.0f, PF_GROUP1) \
		ADD_REALPROP_FLAG(XAxisSpinUpTime,   0.0f, PF_GROUP1) \
		ADD_REALPROP_FLAG(XAxisSpinDownTime, 0.0f, PF_GROUP1) \
		ADD_BOOLPROP_FLAG(XRotateForward, DTRUE, PF_GROUP1) \
		ADD_REALPROP_FLAG(YAxisRevTime, 0.0f, PF_GROUP1) \
		ADD_REALPROP_FLAG(YAxisSpinUpTime,   0.0f, PF_GROUP1) \
		ADD_REALPROP_FLAG(YAxisSpinDownTime, 0.0f, PF_GROUP1) \
		ADD_BOOLPROP_FLAG(YRotateForward, DTRUE, PF_GROUP1) \
		ADD_REALPROP_FLAG(ZAxisRevTime, 0.0f, PF_GROUP1) \
		ADD_REALPROP_FLAG(ZAxisSpinUpTime,   0.0f, PF_GROUP1) \
		ADD_REALPROP_FLAG(ZAxisSpinDownTime, 0.0f, PF_GROUP1) \
		ADD_BOOLPROP_FLAG(ZRotateForward, DTRUE, PF_GROUP1) \


class Rotating : public Aggregate
{
	public :

		Rotating();
		virtual ~Rotating();
		DBOOL	Init(HOBJECT hObject);
		void	SetNormalRotation();
		void	SetSpinUp();
		void	SetSpinDown();
		void	SetOff();

	protected :

		enum	RWMState { RWM_OFF, RWM_NORMAL, RWM_SPINUP, RWM_SPINDOWN };

		DDWORD	EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		HOBJECT	m_hObject; 

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

		void UpdateNormalRotation();
		void UpdateSpinUp();
		void UpdateSpinDown();

		void StartSound(HSTRING hstrSoundName, DBOOL bLoop);

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		void	InitialUpdate();
		void	Update();
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		void	HandleTrigger(HOBJECT hSender, HSTRING hMsg);

};

#endif // __ROTATING_H__
