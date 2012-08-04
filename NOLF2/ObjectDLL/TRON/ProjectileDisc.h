// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileDisc.h
//
// PURPOSE : TRON Disc class - definition
//
// CREATED : 12/10/01
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef  _ProjectileDisc_INCLUDED_
#define  _ProjectileDisc_INCLUDED_

//
// Includes
//
#include "Projectile.h"


// forward define to reduce header dependancies
struct WeaponFireInfo;


///
// class CDisc
//
class CDisc : public CProjectile
{
public:
	CDisc();
	~CDisc();

protected:
	// setup the disc (called externally)
	virtual LTBOOL      Setup( CWeapon const*, WeaponFireInfo const& );

	// message functions
	virtual uint32      EngineMessageFn( uint32, void*, LTFLOAT );
	virtual uint32      ObjectMessageFn( HOBJECT, ILTMessage_Read* );

	virtual void        HandleImpact( HOBJECT );
	virtual void        HandleTouch( HOBJECT );
	virtual void		HandleOwnerDeath();

	virtual void RemoveObject();

	virtual bool        UpdateDisc();

	virtual void        Detonate( HOBJECT );

	LTVector			GetReturnToPosition(const LTVector& vOffset);

	// specific update routines
	bool                UpdateStateFlying();
	bool                UpdateStateDeflecting();
	bool                UpdateStateReturning();

	// State query methods:
	bool				IsDiscReturning();

	// code to handle state changing information on ReturnMode
	virtual void        EnterReturnMode( bool bImpact );

	// handles everything necessary to send request to the owner
	// to stop or start sending projectile messages
	void                RequestStartSendingMessages();
	void                RequestStopSendingMessages();

	// current state...what is the disc doing?
	// NOTE: these are enumerated states, the enumerations
	// defined in the .cpp file
	int                 m_eDiscState;
	int                 m_eTrackingMode;
	int                 m_eTrackingStyle;

	// TRUE between the time we receive a new target point
	// and the disc recomputes its flight based on the new info.
	bool                m_bNewTarget;

	// The target the disc is trying to hit (it will use one or the
	// other, not both, depending on the tracking state).
	LTVector            m_vPointTarget;
	LTObjRef            m_hObjectTarget;

	// variables for the control line method of tracking
	LTVector            m_vControlDirection;
	LTVector            m_vControlPosition;

	// Socket the Disc should return to, valid if the valid
	// is not INVALID_MODEL_SOCKET
	HMODELSOCKET		m_hReturnToSocket;

	// keep track if we've requested messages or not
	// (to cut down no message traffic)
	bool                m_bReceivingMessages;

	// Store the ID of the Catch enabling stimulus so that it can
	// be turned off when the disc is caught.  Currently set if the
	// value is not kStimID_Unset
	EnumAIStimulusID	m_eDiscCatchableStimID;

	// Store the ID of the Block enabling stimulus so that it can
	// be turned off when the disc enters return mode.  Currently 
	// set if the value is not kStimID_Unset
	EnumAIStimulusID	m_eDiscBlockableStimID;

	// last time the projectile was updated
	LTFLOAT             m_fLastUpdateTime;

	// the disc's return velocity
	LTFLOAT             m_fReturnVelocity;

	// if the disc is not returning to a node, it will return
	// to the owner's position plus this offset in the Y
	// direction
	LTFLOAT             m_fReturnHeightOffset;

	// rate (in RADIANS/SEC) at which the disc can turn
	LTFLOAT             m_fTurnRate;

	// angle (in RADIANS) the disc will attempt in aim for the control line
	LTFLOAT             m_fIncidentAngleToControlLine;

	// the decay (in RADIANS/SEC) of the previously mentioned angle
	LTFLOAT             m_fIncidentAngleToControlLineDecay;

	//	Temp demo hack!!  This is here because the discs RemoveObject
	//	can be called from TestInsideObject, meaning that the thrower
	//	never recieves the RETURNED message, and is then stuck
	//	discless.  Remove this once a true solution has been
	//	implemented
	LTBOOL				m_bSentReturnedMsg;

	// time at which the deflection started
	int                 m_nDeflectServerTimeStart;
};


#endif //_ProjectileDisc_INCLUDED_
