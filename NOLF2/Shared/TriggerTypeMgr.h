// ----------------------------------------------------------------------- //
//
// MODULE  : TriggerTypeMgr.h
//
// PURPOSE : The TriggerTypeMgr object
//
// CREATED : 7/22/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRIGGER_TYPE_MGR_H__
#define __TRIGGER_TYPE_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"
	#include "TemplateList.h"

//
// Forwards...
//

	class CTriggerTypeMgr;
	extern CTriggerTypeMgr *g_pTriggerTypeMgr;

//
// Defines...
//

	#define TTMGR_DEFAULT_FILE		"Attributes\\TriggerTypes.txt"
	#define TTMGR_INVALID_ID		-1
	#define	TTMGR_MAX_NAME_LENGTH	32
	#define TTMGR_MAX_FILE_PATH		128


struct TRIGGERTYPE
{
	TRIGGERTYPE();
	~TRIGGERTYPE();

	bool	Init( CButeMgr &ButeMgr, char *aTagName );

	uint8		nId;
	
	char	*szName;
	char	*szIcon;

};

typedef CTList<TRIGGERTYPE*> TriggerTypeList;

class CTriggerTypeMgr : public CGameButeMgr
{
	public: // Methods...

		CTriggerTypeMgr();
		~CTriggerTypeMgr();

		// Call this to get the singleton instance of the activate type mgr.
		static CTriggerTypeMgr& Instance( );

		void			Reload() { Term(); m_buteMgr.Term(); Init(); }

		LTBOOL			Init( const char *szAttributeFile = TTMGR_DEFAULT_FILE );
		void			Term( );

		TRIGGERTYPE*	GetTriggerType( uint32 nID );
		TRIGGERTYPE*	GetTriggerType( const char *pName );

		int				GetNumTriggerTypes( ) const { return m_TriggerTypeList.GetLength(); }


	private :	// Members...

		TriggerTypeList	m_TriggerTypeList;	
};

#ifndef _CLIENTBUILD

////////////////////////////////////////////////////////////////////////////
//
// CTriggerTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use TriggerTypeMgr
//
////////////////////////////////////////////////////////////////////////////

#include "iobjectplugin.h"

class CTriggerTypeMgrPlugin : public IObjectPlugin
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

#endif // _CLIENTBUILD

#endif // __TRIGGER_TYPE_MGR_H__