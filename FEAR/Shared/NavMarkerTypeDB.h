// ----------------------------------------------------------------------- //
//
// MODULE  : NavMarkerTypeDB.h
//
// PURPOSE : NavMarkerType database definition
//
// CREATED : 11/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __NMT_DATABASE_H__
#define __NMT_DATABASE_H__

//
// Includes...
//

#include "GameDatabaseMgr.h"


//
// Defines...
//
	


class CNavMarkerTypeDB;
extern CNavMarkerTypeDB* g_pNavMarkerTypeDB;

typedef HRECORD	HNAVMARKERTYPE;

class CNavMarkerTypeDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CNavMarkerTypeDB );

	public :	// Methods...

		bool	Init( const char *szDatabaseFile = DB_Default_File );
		void	Term() {};

		using		CGameDatabaseMgr::GetRecord;
		HNAVMARKERTYPE	GetRecord( const char *pszType ) const;
		HNAVMARKERTYPE	GetRecord( uint16 nIndex ) const
		{ 
			return g_pLTDatabase->GetRecordByIndex( m_hCatTypes, nIndex );
		}
		
		uint16		GetNumTypes( )	const	{ return m_nNumTypes; }
		
		HCATEGORY	GetCategory( )		const	{ return m_hCatTypes; }

		LTVector	 GetWorldOffset(HNAVMARKERTYPE hRecord) const;

		const char * GetIconTexture(HNAVMARKERTYPE hRecord) const;
		LTVector2	 GetIconSize(HNAVMARKERTYPE hRecord) const;
		LTVector2	 GetIconOffset(HNAVMARKERTYPE hRecord) const;
		uint32		 GetIconColor(HNAVMARKERTYPE hRecord) const;

		bool		 UseRange(HNAVMARKERTYPE hRecord) const;
		uint8		 GetRangeSize() const; //global, equals text size of "default" type
		LTVector2	 GetRangeOffset(HNAVMARKERTYPE hRecord) const;
		const char*	 GetTextFont(HNAVMARKERTYPE hRecord) const;
		uint8		 GetTextSize(HNAVMARKERTYPE hRecord) const;
		LTVector2	 GetTextOffset(HNAVMARKERTYPE hRecord) const;
		uint32		 GetTextColor(HNAVMARKERTYPE hRecord) const;

		const char * GetArrowTexture(HNAVMARKERTYPE hRecord) const;
		LTVector2	 GetArrowSize(HNAVMARKERTYPE hRecord) const;
		uint32		 GetArrowColor(HNAVMARKERTYPE hRecord) const;

		uint8		 GetPriority(HNAVMARKERTYPE hRecord) const;

		const char * GetClientFX(HNAVMARKERTYPE hRecord) const;

		float		 GetLifetime(HNAVMARKERTYPE hRecord) const;
		float		 GetMultiplayerFadeAngle(HNAVMARKERTYPE hRecord) const;

		LTVector2	 GetFadeRange(HNAVMARKERTYPE hRecord) const;
		float		 GetMinimumFadeAlpha(HNAVMARKERTYPE hRecord) const;


	private:

		HRECORD		GetLayoutRecord(HNAVMARKERTYPE hRecord) const;

	private	:	// Members...

		HCATEGORY	m_hCatTypes;
		uint16		m_nNumTypes;
};

class CNavMarkerLayoutDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CNavMarkerLayoutDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_Localized_File );
	void	Term() {};

	using		CGameDatabaseMgr::GetRecord;
	HRECORD		GetRecord( char const* pszRecordName ) { return g_pLTDatabase->GetRecord( m_hLayoutCat, pszRecordName ); }

	const char*	 GetTextFont(HRECORD hRecord) const;
	uint8		 GetTextSize(HRECORD hRecord) const;
	LTVector2	 GetTextOffset(HRECORD hRecord) const;
	HCATEGORY	 GetCategory( ) { return m_hLayoutCat; }

private	:	// Members...

	HCATEGORY	m_hLayoutCat;
};


////////////////////////////////////////////////////////////////////////////
//
// CNavMarkerTypeDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use NavMarkerTypeDB
//
////////////////////////////////////////////////////////////////////////////
#ifdef _SERVERBUILD

#include "iobjectplugin.h"

class CNavMarkerTypeDBPlugin : public IObjectPlugin
{
	private:

		CNavMarkerTypeDBPlugin();
		CNavMarkerTypeDBPlugin( const CNavMarkerTypeDBPlugin &other );
		CNavMarkerTypeDBPlugin& operator=( const CNavMarkerTypeDBPlugin &other );
		~CNavMarkerTypeDBPlugin();


	public:

		NO_INLINE static CNavMarkerTypeDBPlugin& Instance() { static CNavMarkerTypeDBPlugin sPlugin; return sPlugin; }

		static void PopulateStringList(char** aszStrings, uint32* pcStrings,
			const uint32 cMaxStrings, const uint32 cMaxStringLength);

};

#endif // _SERVERBUILD


#endif // __WEAPON_DATABASE_H__
