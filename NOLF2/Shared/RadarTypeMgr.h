// ----------------------------------------------------------------------- //
//
// MODULE  : RadarTypeMgr.h
//
// PURPOSE : The RadarTypeMgr object
//
// CREATED : 6/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __RADAR_TYPE_MGR_H__
#define __RADAR_TYPE_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"
	#include "TemplateList.h"

//
// Forwards...
//

	class CRadarTypeMgr;
	extern CRadarTypeMgr *g_pRadarTypeMgr;

//
// Defines...
//

	#define	RTMGR_DEFAULT_FILE		"Attributes\\RadarTypes.txt"
	#define RTMGR_INVALID_ID		-1
	#define RTMGR_MAX_NAME_LENGTH	32
	#define RTMGR_MAX_FILE_PATH		64


struct RADARTYPE
{
	RADARTYPE();
	~RADARTYPE();

	bool	Init( CButeMgr &ButeMgr, char *aTagName );
	
	uint8			nId;

	char			*szName;
	char			*szIcon;
	int				nDrawOrder;
};


typedef CTList<RADARTYPE*> RadarTypeList;

class CRadarTypeMgr : public CGameButeMgr
{
	public :	// Methods...
		
		CRadarTypeMgr( );
		~CRadarTypeMgr( );
		
		// Call this to get the singleton instance of the radar type mgr.
		static CRadarTypeMgr& Instance( );

		void			Reload() { Term(); m_buteMgr.Term(); Init(); }

		LTBOOL			Init( const char *szAttributeFile = RTMGR_DEFAULT_FILE );
		void			Term( );

		RADARTYPE		*GetRadarType( uint32 nID );
		RADARTYPE		*GetRadarType( const char *pName );

		int				GetNumRadarTypes( ) const { return m_RadarTypeList.GetLength(); }


	private :	// Members...

		RadarTypeList	m_RadarTypeList;
};

////////////////////////////////////////////////////////////////////////////
//
// CRadarTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use RadarTypeMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CRadarTypeMgrPlugin : public IObjectPlugin
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

#endif // __RADAR_TYPE_MGR_H__