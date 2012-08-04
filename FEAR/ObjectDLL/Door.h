// ----------------------------------------------------------------------- //
//
// MODULE  : DOOR.h
//
// PURPOSE : A Door object
//
// CREATED : 8/5/97 5:07:00 PM
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DOOR_H__
#define __DOOR_H__

//
// Includes...
//

	#include "ActiveWorldModel.h"
	#include "AIEnumNavMeshLinkTypes.h"

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
	typedef ActiveWorldModel Super;

	public: // Methods...

		Door();
		virtual ~Door();

		bool		IsLockedForCharacter( HOBJECT hChar ) const;
		uint8		GetState() const { return m_nCurState; }
		bool		IsAITriggerable() const { return !!(m_dwPropFlags & AWM_PROP_AIACTIVATE); }
		float 		GetYaw() const { return m_fYaw; }
		const std::string& 	GetDoorLinkName() const { return m_sDoorLink; }
		void		SetNMLinkID( ENUM_NMLinkID eNMLinkID ) { m_eNMLinkID = eNMLinkID; }

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected: // Members...

		std::string		m_sDoorLink;		// Name of other door we are linked to for synched motion
		LTObjRef		m_hDoorLink;		// Door Object that we are linked to 

		enum { knNumSectors = 2 };

		std::string		m_sSectorName[knNumSectors];	// Name of the sector controlled by this door
		uint32			m_nSectorID[knNumSectors];		// ID of the door's	sectors																			or

		ENUM_NMLinkID	m_eNMLinkID;		// Associated NavMeshLink.

		bool	IsLocked() const { return !!(m_dwPropFlags & AWM_PROP_LOCKED); }

	protected: // Methods...

		// Engine message handlers...
		
		virtual uint32	OnAllObjectsCreated( );
		virtual void	Lock( bool bLock );
		virtual void	OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		virtual void	OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		virtual void	ReadProps( const GenericPropList *pProps );

		// State methods...

		virtual void	SetPowerOn( double fTime, uint8 nWaveform );
		virtual void	SetPowerOff( double fTime, uint8 nWaveform );

		virtual void	SetOn( bool bInitialState );
		virtual void	SetOff( bool bInitialState );
		virtual void	ToggleState( float fTime = -1.0f, uint8 nWaveform = (uint8)-1 );

		// Activation...

		virtual void	Activate( HOBJECT hObj );

		virtual void	SetStartingAngles( const LTVector& vStartingAngles );

		// Dynamic sector handling
		virtual void	UpdateSector( bool bSectorActive );

	private: // Methods...

		// Trigger handlers...

		void	HandelLinkToggleState( bool bTriggerLink, float fTime = -1.0f, uint8 nWaveform = (uint8)-1 );
		void	ToggleLink( HOBJECT hActivateObj, float fTime = -1.0f, uint8 nWaveform = (uint8)-1 );

		bool			m_bSectorsActive;


		// Message Handlers...

		DECLARE_MSG_HANDLER( Door, HandleCloseMsg );

	private:

		PREVENT_OBJECT_COPYING( Door );

};

#endif // __DOOR_H__
