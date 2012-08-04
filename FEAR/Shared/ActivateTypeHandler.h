// ----------------------------------------------------------------------- //
//
// MODULE  : ActivateTypeHandler.h
//
// PURPOSE : Provide activation type to an existing game object class.
// For stand-alone activation objects, see ActivateObjectFX.h
//
// CREATED : 7/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACTIVATE_TYPE_HANDLER_H__
#define __ACTIVATE_TYPE_HANDLER_H__

//
// Includes...
//

#include "ActivateDB.h"
#include "GameBase.h"

//
// Classes...
//

class CActivateTypeHandler : public ILTObjRefReceiver
{
	public: // Methods...

		CActivateTypeHandler();
		~CActivateTypeHandler();

		// CActivateTypeHandlers are not valid until their type is set.
		void Init( HOBJECT hInObject, const char *pInitActivateTypeStr = NULL );
		void Term();

		void	SetActivateType( const char *pName );
		uint8	GetActivateType( ) const { return m_nId; }

		void	CreateActivateTypeMsg();						// Use this when there is no other specialfx message related to the object
		void	WriteActivateTypeMsg( ILTMessage_Write *pMsg ); // Use this to write the data to an existing specialfx message
		
		void	SetDisabled( bool bDis, bool bSendToClient = true, HCLIENT hClient = NULL );
		void	SetState( ACTIVATETYPE::State eState, bool bSendToClient = true, HCLIENT hClient = NULL );
		void	SetCanActivate( bool bCanActivate );

		void	Save( ILTMessage_Write *pMsg );
		void	Load( ILTMessage_Read *pMsg );

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		void	InheritObject( HOBJECT hObj );
		void	DisownObject( HOBJECT hObj );

	protected: // Members...

		void	SetParent( HOBJECT hObject );
		bool	IsValid( bool bVerifyTypeInDB = false );

		LTObjRef				m_hParent;				// The object we belong to

		uint8					m_nId;					// Id of the ActivateType
		bool					m_bDisabled;			// Is the object disabled
		ACTIVATETYPE::State		m_eState;				// Which state are we in

		ObjRefNotifierList		m_lstInheritedObjs;		// List of objects that inherit the ActivateType from m_pBase

		EExecutionShellContext	m_eExecutionShell;
};

#endif // __ACTIVATE_TYPE_HANDLER_H__
