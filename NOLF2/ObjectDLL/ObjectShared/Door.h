// ----------------------------------------------------------------------- //
//
// MODULE  : DOOR.h
//
// PURPOSE : A Door object
//
// CREATED : 8/5/97 5:07:00 PM
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DOOR_H__
#define __DOOR_H__

//
// Includes...
//

	#include "ActiveWorldModel.h"

//
// Defines...
//

	// Assign these to the equivelant AactiveWorldModel states...	

	#define DOORSTATE_CLOSED		AWM_STATE_OFF
	#define DOORSTATE_CLOSING		AWM_STATE_POWEROFF
	#define DOORSTATE_OPEN			AWM_STATE_ON
	#define DOORSTATE_OPENING		AWM_STATE_POWERON

LINKTO_MODULE( Door );


//
// Structs...
//

class Door : public ActiveWorldModel
{
	public: // Methods...

		Door();
		virtual ~Door();

		LTBOOL	IsLockedForCharacter( HOBJECT hChar ) const;
		uint8	GetState() const { return m_nCurState; }
        LTBOOL	IsAITriggerable() const { return ( m_dwPropFlags & AWM_PROP_AIACTIVATE ); }
		LTFLOAT GetYaw() const { return m_fYaw; }
		void	SetAIUser(HOBJECT hAIUser) { m_hAIUser = hAIUser; }
		HOBJECT	GetAIUser() const { return m_hAIUser; }
		HOBJECT GetDoorLink() const { return m_hDoorLink; }

	protected: // Members...

		HSTRING		m_hstrDoorLink;			// Name of other door we are linked to for synched motion
		LTObjRef	m_hDoorLink;			// Door Object that we are linked to 
		LTObjRef	m_hAIUser;				// Handle to AI currently using the door.

		LTBOOL	IsLocked() const { return ( m_dwPropFlags & AWM_PROP_LOCKED ); }

	protected: // Methods...

		// Engine message handlers...
		
		virtual uint32	OnAllObjectsCreated( );
		virtual bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );
		virtual void	HandleTriggerMsg( );
		virtual void	HandleLock(LTBOOL bLock);
		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( ObjectCreateStruct *pOCS );

		// State methods...

		virtual void	SetPowerOn( );
		virtual void	SetPowerOff( );

		// Activation...

		virtual void	Activate( HOBJECT hObj );

	private: // Methods...

		// Trigger handlers...

		void	HandelLinkTriggerMsg( LTBOOL bTriggerLink );
		void	TriggerLink( HOBJECT hActivateObj );
		void	TriggerClose( );

		void	PlayDoorKnobAni( char* pAniName );
};

#endif // __DOOR_H__