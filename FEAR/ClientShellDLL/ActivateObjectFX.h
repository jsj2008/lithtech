// ----------------------------------------------------------------------- //
//
// MODULE  : ActivateObjectFX.h
//
// PURPOSE : ActivateObject special fx class - Definition
//
// CREATED : 7/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACTIVATE_OBJECT_FX_H__
#define __ACTIVATE_OBJECT_FX_H__

//
// Includes...
//
	
#include "SpecialFX.h"
#include "ActivateDB.h"

//
// Defines...
//

#define		MAX_ACTIVATEOBJECTFX_OBJECTS		400


class CActivateObjectHandler : public ILTObjRefReceiver
{
	public: // Methods...

		CActivateObjectHandler();
		~CActivateObjectHandler();

		bool	Init( HOBJECT hObject );
		
		static const CActivateObjectHandler* FindActivateObject( HOBJECT hHandledObject );

		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	public: // Members... 
		
		uint8					m_nId;			// Id of the ActivateType
		bool					m_bDisabled;	// Is the object disabled
		ACTIVATETYPE::State		m_eState;		// Which state are we in


	protected: // Members...
	
		typedef std::vector<CActivateObjectHandler*> ActivateObjList;

		LTObjRefNotifier				m_hObject;		// Object we are handling

		static ActivateObjList	m_lstActivateObjs;
};


class CActivateObjectFX : public CSpecialFX
{
	public: // Methods...

		CActivateObjectFX();
		~CActivateObjectFX();

		virtual bool	Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
		virtual bool	OnServerMessage( ILTMessage_Read *pMsg );
		
		virtual uint32	GetSFXID() { return SFX_ACTIVATEOBJECT_ID; }
		virtual bool	Update() { return !m_bWantRemove; }


	private: // Members...

		CActivateObjectHandler	m_ActivateObjectHandler;
};

#endif // __ACTIVATE_OBJECT_FX_H__
