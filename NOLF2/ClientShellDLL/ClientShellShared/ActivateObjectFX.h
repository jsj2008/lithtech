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
	#include "ActivateTypeMgr.h"


class CActivateObjectHandler
{
	public: // Methods...

		CActivateObjectHandler();
		~CActivateObjectHandler();

		typedef std::vector<CActivateObjectHandler*> ActivateObjList;
		static ActivateObjList const& GetActivateObjectList() { return m_lstActivateObjs; }

		bool	Init( HOBJECT hObject );
		
		HOBJECT	GetHOBJECT() const { return m_hObject; }
		

	public: // Members... 
		
		uint8					m_nId;			// Id of the ActivateType
		bool					m_bDisabled;	// Is the object disabled
		ACTIVATETYPE::State		m_eState;		// Which state are we in


	protected: // Members...
	
		HOBJECT					m_hObject;		// Object we are handling

		static ActivateObjList	m_lstActivateObjs;
};


class CActivateObjectFX : public CSpecialFX
{
	public: // Methods...

		CActivateObjectFX();
		~CActivateObjectFX();

		virtual LTBOOL	Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
		virtual LTBOOL	OnServerMessage( ILTMessage_Read *pMsg );
		
		virtual uint32	GetSFXID() { return SFX_ACTIVATEOBJECT_ID; }
		virtual LTBOOL	Update() { return !m_bWantRemove; }


	private: // Members...

		CActivateObjectHandler	m_ActivateObjectHandler;
};

#endif // __ACTIVATE_OBJECT_FX_H__
