// ----------------------------------------------------------------------- //
//
// MODULE  : PropsDB.h
//
// PURPOSE : Definition of the models database.
//
// CREATED : 03/09/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _PROPSDB_H__
#define _PROPSDB_H__


//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "resourceextensions.h"

//
// Defines...
//

class PropsDB;
extern PropsDB* g_pPropsDB;

class PropsDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( PropsDB );

public:

	typedef HRECORD HPROP;

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	// HPROP Accessors.
	HCATEGORY		GetPropsCategory( ) const { return m_hPropsCat; }
	uint32			GetNumProps() const;
	HPROP			GetProp( uint32 nIndex ) const;
	HPROP			GetPropByRecordName( char const* pszRecordName ) const;
	char const*		GetRecordNameOfHProp( HPROP hProp ) const;
	const char*		GetPropFilename(HPROP hProp) const;
	uint32			GetNumMaterials(HPROP hProp) const;
	const char*		GetMaterialFilename(HPROP hProp, uint8 iMaterial) const;
	void			CopyMaterialFilenames(HPROP hProp, char* paszDest, uint32 nNumValues, uint32 nStrLen) const;

private	:	// Members...

	HCATEGORY	m_hPropsCat;
};


#endif  // _PROPSDB_H__
