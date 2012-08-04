// ----------------------------------------------------------------------- //
//
// MODULE  : ActivateTypeMgr.h
//
// PURPOSE : The ActivateTypeMgr object
//
// CREATED : 7/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACTIVATE_TYPE_MGR_H__
#define __ACTIVATE_TYPE_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"
	#include "TemplateList.h"

//
// Forwards...
//

	class CActivateTypeMgr;
	extern CActivateTypeMgr *g_pActivateTypeMgr;

//
// Defines...
//

	#define ATMGR_DEFAULT_FILE		"Attributes\\ActivateTypes.txt"
	#define ATMGR_INVALID_ID		-1
	#define ATMGR_MAX_NAME_LENGTH	32


struct ACTIVATETYPE
{
	ACTIVATETYPE();
	~ACTIVATETYPE();

	enum State
	{
		eOn	= 0,
		eOff,

		eMaxStates
	};

	bool	Init( CButeMgr &ButeMgr, char *aTagName );

	uint8	nId;
	
	char	*szName;
	uint32	dwStateID[eMaxStates];
};

typedef CTList<ACTIVATETYPE*> ActivateTypeList;

class CActivateTypeMgr : public CGameButeMgr
{
	public: // Methods...

		CActivateTypeMgr();
		~CActivateTypeMgr();

		// Call this to get the singleton instance of the activate type mgr.
		static CActivateTypeMgr& Instance( );

		void			Reload() { Term(); m_buteMgr.Term(); Init(); }

		LTBOOL			Init( const char *szAttributeFile = ATMGR_DEFAULT_FILE );
		void			Term( );

		ACTIVATETYPE	*GetActivateType( uint32 nID );
		ACTIVATETYPE	*GetActivateType( const char *pName );

		int				GetNumActivateTypes( ) const { return m_ActivateTypeList.GetLength(); }


	private :	// Members...

		ActivateTypeList	m_ActivateTypeList;	
};

#ifndef _CLIENTBUILD

////////////////////////////////////////////////////////////////////////////
//
// CActivateTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use ActivateTypeMgr
//
////////////////////////////////////////////////////////////////////////////

#include "iobjectplugin.h"

class CActivateTypeMgrPlugin : public IObjectPlugin
{
	public:
		
		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

		bool PopulateStringList(char** aszStrings, uint32* pcStrings,
			const uint32 cMaxStrings, const uint32 cMaxStringLength);

	
	protected :

		static bool		sm_bInitted;
};

////////////////////////////////////////////////////////////////////////////
//
// CActivateTypeHandler is used to keep the functionality for handling an
// ActivateType within a single class.
//
////////////////////////////////////////////////////////////////////////////

class CActivateTypeHandler : public ILTObjRefReceiver
{
	public: // Methods...

		CActivateTypeHandler();
		~CActivateTypeHandler();

		void	Init( LPBASECLASS pObj );

		void	SetActivateType( uint8 nId );
		void	SetActivateType( const char *pName );

		void	CreateActivateTypeMsg();						// Use this when there is no other specialfx message related to the object
		void	WriteActivateTypeMsg( ILTMessage_Write *pMsg ); // Use this to write the data to an existing specialfx message
		
		void	SetDisabled( bool bDis, bool bSendToClient = true, HCLIENT hClient = LTNULL );
		void	SetState( ACTIVATETYPE::State eState, bool bSendToClient = true, HCLIENT hClient = LTNULL );

		void	Save( ILTMessage_Write *pMsg );
		void	Load( ILTMessage_Read *pMsg );

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		void	InheritObject( HOBJECT hObj );
		void	DisownObject( HOBJECT hObj );

		
	protected: // Members...

		LPBASECLASS				m_pBase;				// The BaseClass object we belong to

		uint8					m_nId;					// Id of the ActivateType
		bool					m_bDisabled;			// Is the object disabled
		ACTIVATETYPE::State		m_eState;				// Which state are we in

		ObjRefNotifierList		m_lstInheritedObjs;		// List of objects that inherit the ActivateType from m_pBase
};

#endif // _CLIENTBUILD

#endif // __ACTIVATE_TYPE_MGR_H__