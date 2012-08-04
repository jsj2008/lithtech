// ----------------------------------------------------------------------- //
//
// MODULE  : CategoryDB.h
//
// PURPOSE : Helper class to define interface to a DB Category
//
// CREATED : 5/1/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __CATEGORYDB_H__
#define __CATEGORYDB_H__

//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "iobjectplugin.h"
#include "CommonUtilities.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CategoryDB
//
//	PURPOSE:	Database for accessing single category
//
// ----------------------------------------------------------------------- //

class CategoryDB : public CGameDatabaseMgr
{
public:

	CategoryDB( ) { m_hCategory = NULL; }
	~CategoryDB( ) { Term( ); }
	
	// Init/term
	bool	Init( char const* pszCategory, char const* pszDatabaseFile );
	void	Term();

	// Queries
	HCATEGORY			GetCategory() const { return m_hCategory; }

	// Gets the number of records in the category.
	uint32 GetNumRecords( ) const
	{
		return g_pLTDatabase->GetNumRecords( GetCategory( ));
	}

	// Gets the record by index.
	HRECORD GetRecordByIndex( uint32 nIndex ) const
	{
		return g_pLTDatabase->GetRecordByIndex( GetCategory( ), nIndex );
	}

	// Gets a record of the category by name.
	HRECORD GetRecordByName( char const* pszRecordName ) const
	{
		return GetRecord( GetCategory( ), pszRecordName );
	}

private:

	// We're overriding and overloading Init, so import the init from our base.
	using CGameDatabaseMgr::Init;

private:

	HCATEGORY			m_hCategory;	
};

// Use these macros to define a simple interface to a database category.  It can service most needs, but isn't
// recommended for interfaces that need to massage the database through the accessors.
// Start the category definition wiht BEGIN_DATABASE_CATEGORY, specifying the name of the category and the category path
// in the database.  Add DEFINE_GETRECORDATTRIB for each attribute to access.  Then end the category with END_DATABASE_CATEGORY.
// Access the category attributes with DATABASE_CATEGORY( ).GETRECORDATTRIB.
// Example:
//		BEGIN_DATABASE_CATEGORY( CollisionProperty, "Collisions\\CollisionProperty" )
//		DEFINE_GETRECORDATTRIB( Duration, float );
//		END_DATABASE_CATEGORY( );
#define BEGIN_DATABASE_CATEGORY( category, path ) \
	class _DatabaseCategory_##category : public CategoryDB \
	{ \
		DECLARE_SINGLETON_SIMPLE( _DatabaseCategory_##category ); \
	public: \
		virtual bool Init( const char *szDatabaseFile = DB_Default_File ) { return CategoryDB::Init( path, szDatabaseFile ); }

#define END_DATABASE_CATEGORY( ) \
	private: \
		using CategoryDB::Init; \
	};

// Accessor to a Category defined with DATABASE_CATEGORY.
#define DATABASE_CATEGORY( category ) \
	_DatabaseCategory_##category::Instance( )

// Get Attribute given a category name, HRECORD and Attribute name.  The Category name must be the same as passed to 
// BEGIN_DATABASE_CATEGORY.  AttribName must be teh same as passed to DEFINE_GETRECORDATTRIB.
#define GETCATRECORDATTRIB( category, hRecord, AttribName ) \
	DATABASE_CATEGORY( category ).GETRECORDATTRIB( hRecord, AttribName )


// ----------------------------------------------------------------------- //
//
//	CategoryPlugin
//
//	PURPOSE:	Fills in string list with all the records in a category.
//				Preceeds the list with "<none>" using StringList_None.
//
// ----------------------------------------------------------------------- //
class CategoryPlugin : public IObjectPlugin
{
	DECLARE_SINGLETON_SIMPLE( CategoryPlugin );

public:

	static bool PopulateStringList( HCATEGORY hCategory, char** aszStrings, uint32* pcStrings,
		const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

#define StringList_None "<none>"

#endif // __CATEGORYDB_H__
