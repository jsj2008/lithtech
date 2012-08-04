// ----------------------------------------------------------------------- //
//
// MODULE  : DatabaseUtils.h
//
// PURPOSE : definition of base classes for reading and writing GameDatabases
//
// CREATED : 07/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DATABASE_UTILS_H__
#define __DATABASE_UTILS_H__

//
// Includes...
//

	#include "platform.h"
	#include "idatabasemgr.h"
	#include "idatabasecreatormgr.h"


//
// Defines...
//

#if defined(PLATFORM_LINUX)
	#define GDB_DLL_NAME	"libGameDatabase.so"
#else
	#define GDB_DLL_NAME	"GameDatabase"
#endif


// Defines for getting the database functions...
typedef IDatabaseMgr* (*TGetIDatabaseMgrFn)();
typedef IDatabaseCreatorMgr* (*TGetIDatabaseCreatorMgrFn)();

//Database DLL accessors
extern IDatabaseMgr			*g_pLTDatabase;
extern IDatabaseCreatorMgr	*g_pLTDatabaseCreator;

// Use these to make sure the database dll is loaded and freed correctly...
// Every load must be matched with a free...
bool LoadDatabaseInterface( const char *pszDllFile = GDB_DLL_NAME );
void FreeDatabaseInterface( );

// Debug macro for making sure the attribute we want to access is of the correct type...
#ifdef _DEBUG
/*
#define VERIFY_ATTRIBUTE_TYPE( hrecord, name, type ) \
{ \
HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hrecord, name ); \
EAttributeType eActualType = g_pLTDatabase->GetAttributeType( hAtt ); \
LTASSERT( eActualType == type, "Attribute is of different type" ); \
} \
*/
const char* GetDBErrorString();
bool VerifyAttribute( HRECORD hrecord, const char * pszName, EAttributeType type );
#define VERIFY_ATTRIBUTE_TYPE( hrecord, name, type  ) \
	LTASSERT(VerifyAttribute(hrecord, name, type),GetDBErrorString());


#else

#define VERIFY_ATTRIBUTE_TYPE( hrecord, name, type  )

#endif

// Temporary macros for creating a record and adding attributes to that record...
// Used to convert old bute files to new database format. These will be removed once
// the database editor is finished.

#define DB_CREATE_RECORD( category, name ) \
	HRECORDCREATOR hRec = g_pLTDatabaseCreator->CreateRecord( category, name ); \

#define DB_CREATE_INT( name, val ) \
{ \
	HATTRIBUTECREATOR hAtt; \
	hAtt = g_pLTDatabaseCreator->CreateAttribute( hRec, name, eAttributeType_Int32, 1 ); \
	g_pLTDatabaseCreator->SetInt32( hAtt, 0, val ); \
} \

#define DB_CREATE_FLOAT( name, val ) \
{ \
	HATTRIBUTECREATOR hAtt; \
	hAtt = g_pLTDatabaseCreator->CreateAttribute( hRec, name, eAttributeType_Float, 1 ); \
	g_pLTDatabaseCreator->SetFloat( hAtt, 0, val ); \
} \

#define DB_CREATE_BOOL( name, val ) \
{ \
	HATTRIBUTECREATOR hAtt; \
	hAtt = g_pLTDatabaseCreator->CreateAttribute( hRec, name, eAttributeType_Bool, 1 ); \
	g_pLTDatabaseCreator->SetBool( hAtt, 0, val ); \
} \

#define DB_CREATE_VECTOR( name, val ) \
{ \
	HATTRIBUTECREATOR hAtt; \
	hAtt = g_pLTDatabaseCreator->CreateAttribute( hRec, name, eAttributeType_Vector3, 1 ); \
	g_pLTDatabaseCreator->SetVector3( hAtt, 0, val ); \
} \

#define DB_CREATE_VECTOR2( name, val ) \
{ \
	HATTRIBUTECREATOR hAtt; \
	hAtt = g_pLTDatabaseCreator->CreateAttribute( hRec, name, eAttributeType_Vector2, 1 ); \
	g_pLTDatabaseCreator->SetVector2( hAtt, 0, val ); \
} \

#define DB_CREATE_STRING( name, val ) \
{ \
	HATTRIBUTECREATOR hAtt; \
	hAtt = g_pLTDatabaseCreator->CreateAttribute( hRec, name, eAttributeType_String, 1 ); \
	g_pLTDatabaseCreator->SetString( hAtt, 0, val ); \
} \

class CGameDatabaseReader
{
public :	// Methods...

	CGameDatabaseReader( );
	virtual ~CGameDatabaseReader( );

	HCATEGORY GetCategory( HDATABASE hDatabase, const char *pszCategory ) const
	{
		return g_pLTDatabase->GetCategory(hDatabase,pszCategory);
	}

	HRECORD GetRecord( HCATEGORY hCat, const char *pszRecord ) const;
	HRECORD GetRecord( HDATABASE hDatabase, const char *pszCategory, const char *pszRecord ) const;

	static HATTRIBUTE GetAttribute( HRECORD hRecord, const char *pszAtt )
	{
		return g_pLTDatabase->GetAttribute( hRecord, pszAtt );
	}

	template< typename T >
	T GetFromRecordType( HRECORD hRecord, char const* pszAttributeName, uint32 nValueIndex ) const { return GetFloat( hRecord, pszAttributeName, nValueIndex ); }
	template< typename T >
	T GetFromAttributeType( HATTRIBUTE hAttribute, uint32 nValueIndex ) const { return GetFloat( hAttribute, nValueIndex ); }

	// Get an integer attribute value...
	int32 GetInt32( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, int32 nDef = 0 ) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_Int32 );
		return g_pLTDatabase->GetInt32( GetAttribute( hRecord, pszAtt ), nValueIndex, nDef );
	}
	int32 GetInt32( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, int32 nDef = 0 ) const
	{
		return g_pLTDatabase->GetInt32( hAttribute, nValueIndex, nDef );
	}

	// Get a float attribute value...
	float GetFloat( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, float fDef = 0.0f ) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_Float );
		return g_pLTDatabase->GetFloat( GetAttribute( hRecord, pszAtt ), nValueIndex, fDef );
	}
	float GetFloat( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, float fDef = 0.0f ) const
	{
		return g_pLTDatabase->GetFloat( hAttribute, nValueIndex, fDef );
	}

	// Get a string attribute valuse...
	const char* GetString( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, const char *pszDef = "" ) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_String );
		return g_pLTDatabase->GetString( GetAttribute( hRecord, pszAtt ), nValueIndex, pszDef );
	}
	const char* GetString( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, const char *pszDef = "" ) const
	{
		return g_pLTDatabase->GetString( hAttribute, nValueIndex, pszDef );
	}

	// Get a wide character string attribute value...
	const wchar_t* GetWString( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, const wchar_t *pszDef = L"" ) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_WString );
		return g_pLTDatabase->GetWString( GetAttribute( hRecord, pszAtt ), nValueIndex, pszDef );
	}
	const wchar_t* GetWString( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, const wchar_t *pszDef = L"" ) const
	{
		return g_pLTDatabase->GetWString( hAttribute, nValueIndex, pszDef );
	}

	// Get a two deminsional vector attribute value...
	LTVector2 GetVector2( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, const LTVector2 &rv2Def = LTVector2( 0.0f, 0.0f )) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_Vector2 );
		return g_pLTDatabase->GetVector2( GetAttribute( hRecord, pszAtt ), nValueIndex, rv2Def );
	}
	LTVector2 GetVector2( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, const LTVector2 &rv2Def = LTVector2( 0.0f, 0.0f )) const
	{
		return g_pLTDatabase->GetVector2( hAttribute, nValueIndex, rv2Def );
	}

	// Get a three deminsional vector attribute value...
	LTVector GetVector3( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, const LTVector &rv3Def = LTVector( 0.0f, 0.0f, 0.0f )) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_Vector3);
		return g_pLTDatabase->GetVector3( GetAttribute( hRecord, pszAtt ), nValueIndex, rv3Def );
	}
	LTVector GetVector3( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, const LTVector &rv3Def = LTVector( 0.0f, 0.0f, 0.0f )) const
	{
		return g_pLTDatabase->GetVector3( hAttribute, nValueIndex, rv3Def );
	}

	// Get a four deminsional vector attribute value...
	LTVector4 GetVector4( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, const LTVector4 &rv4Def = LTVector4( 0.0f, 0.0f, 0.0f, 0.0f )) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_Vector4 );
		return g_pLTDatabase->GetVector4( GetAttribute( hRecord, pszAtt ), nValueIndex, rv4Def );
	}
	LTVector4 GetVector4( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, const LTVector4 &rv4Def = LTVector4( 0.0f, 0.0f, 0.0f, 0.0f )) const
	{
		return g_pLTDatabase->GetVector4( hAttribute, nValueIndex, rv4Def );
	}

	// Get a boolean attribute value...
	bool GetBool( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, bool bDef = false ) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_Bool );
		return g_pLTDatabase->GetBool( GetAttribute( hRecord, pszAtt ), nValueIndex, bDef );
	}
	bool GetBool( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, bool bDef = false ) const
	{
		return g_pLTDatabase->GetBool( hAttribute, nValueIndex, bDef );
	}

	// Get a recordlink attribute value...
	HRECORD GetRecordLink( HRECORD hRecord, const char *pszAtt, uint32 nValueIndex = 0, HRECORD hDef = NULL ) const
	{
		VERIFY_ATTRIBUTE_TYPE( hRecord, pszAtt, eAttributeType_RecordLink );
		return g_pLTDatabase->GetRecordLink( GetAttribute( hRecord, pszAtt ), nValueIndex, hDef );
	}
	HRECORD GetRecordLink( HATTRIBUTE hAttribute, uint32 nValueIndex = 0, HRECORD hDef = NULL ) const
	{
		return g_pLTDatabase->GetRecordLink( hAttribute, nValueIndex, hDef );
	}

	//////////////////////////////////////////////////////////////////////////
	// Function name   : GetStructAttribute
	// Description     : Gets an leaf attribute from a struct.
	// Return type     : HATTRIBUTE - Attribute of parent struct.
	// Argument        : HATTRIBUTE hParent - parent struct element.
	// Argument        : uint32 nValueIndex - index to which struct data to use.
	// Argument        : char const* pszAttribute - attribute of struct.
	//////////////////////////////////////////////////////////////////////////
	static HATTRIBUTE GetStructAttribute( HATTRIBUTE const hParent, uint32 nValueIndex, char const* pszAttribute )
	{
		if( !hParent || !pszAttribute || !pszAttribute[0] )
			return NULL;

		char szCompleteName[256];
		LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.%d.%s", g_pLTDatabase->GetAttributeName( hParent ), 
			nValueIndex, pszAttribute );
		HATTRIBUTE hAttribute = g_pLTDatabase->GetAttribute( g_pLTDatabase->GetAttributeParent( hParent ), szCompleteName );
		return hAttribute;
	}

	//////////////////////////////////////////////////////////////////////////
	// Function name   : GetCrossDatabaseRecordLink
	// Description     : Gets a recordlink across database boundaries by
	//						using a string name in the originating database of the record in the target database.
	// Return type     : HRECORD - Resulting record or NULL if not found.
	// Argument        : HATTRIBUTE hOriginAttrib - String based Recordlink attribute in the originating database.
	// Argument        : uint32 nOriginValueIndex - Value index of the hOriginAttrib
	// Argument        : HCATEGORY hTargetCat - Category of the record in the target database.
	//////////////////////////////////////////////////////////////////////////
	HRECORD GetCrossDatabaseRecordLink( HATTRIBUTE hOriginAttrib, uint32 nOriginValueIndex, HCATEGORY hTargetCat )
	{
		char const* pszRecordName = g_pLTDatabase->GetString( hOriginAttrib, nOriginValueIndex, "" );
		return g_pLTDatabase->GetRecord( hTargetCat, pszRecordName );
	}
};

// Macro used by CGameDatabaseReader to define accessors to specific types. 
// Usage:
//		DEFINE_GETFROMATTRIBTYPE( float, Float );
//		This will define an accessor like this:
//		template< >
//		float GetFromAttributeType< float >( HATTRIBUTE hAttribute, uint32 nValueIndex = 0 ) { return GetFloat( hAttribute, nValueIndex ); }
#define DEFINE_GETFROMATTRIBTYPE( returntype, dbtype ) \
	template< > \
	inline returntype CGameDatabaseReader::GetFromAttributeType< returntype >( HATTRIBUTE hAttribute, uint32 nValueIndex ) const { return Get##dbtype( hAttribute, nValueIndex ); }

// Accessor defines for the known attribute value types.
DEFINE_GETFROMATTRIBTYPE( uint32, Int32 );
DEFINE_GETFROMATTRIBTYPE( int32, Int32 );
DEFINE_GETFROMATTRIBTYPE( float, Float );
DEFINE_GETFROMATTRIBTYPE( char const*, String );
DEFINE_GETFROMATTRIBTYPE( const wchar_t*, WString );
DEFINE_GETFROMATTRIBTYPE( LTVector2, Vector2 );
DEFINE_GETFROMATTRIBTYPE( LTVector, Vector3 );
DEFINE_GETFROMATTRIBTYPE( LTVector4, Vector4 );
DEFINE_GETFROMATTRIBTYPE( bool, Bool );
DEFINE_GETFROMATTRIBTYPE( HRECORD, RecordLink );


// Macro to declare in DB class to define a quick accessor to a specific attribute of a record.
// Usage:
//		DEFINE_GETRECORDATTRIB( TransitionPeriod, float );
//		This will define a accessor like this:
//		float GetTransitionPeriod( HRECORD hRecord, char const* pszRecordName ) { return GetFloat( hRecord, "TransitionPeriod" ); }
#define DEFINE_GETRECORDATTRIB( AttribName, type ) \
	type Get##AttribName( HRECORD hRecord, uint32 nValueIndex = 0 ) const { return GetFromAttributeType< type >( GetAttribute( hRecord, #AttribName ), nValueIndex ); }

// Macro to access an attribute defined by DEFINE_GETRECORDATTRIB
// Usage:
//		float fValue = GETRECORDATTRIB( hRecord, TransitionPeriod );
//		This will define a call to an accessor like this:
//		float fValue = GetTransitionPeriod( hRecord );
#define GETRECORDATTRIB( hRecord, AttribName ) \
	Get##AttribName( hRecord )
#define GETRECORDATTRIBINDEX( hRecord, AttribName, nIndex ) \
	Get##AttribName( hRecord, nIndex )


// Macro to declare in DB class to define a quick accessor to a specific struct of a record.
// Usage:
//		DEFINE_GETRECORDSTRUCT( Responses );
//		This will define a accessor like this:
//		float GetResponses( HRECORD hRecord ) { return GetStruct( hRecord, "Responses" ); }
#define DEFINE_GETRECORDSTRUCT( StructName ) \
	HATTRIBUTE Get##StructName( HRECORD hRecord ) const { return GetAttribute( hRecord, #StructName ); }

// Macro to declare in DB class to define a quick accessor to a specific struct of a struct.
// Usage:
//		DEFINE_GETSTRUCTSTRUCT( Responses, SoundRecord );
//		This will define a accessor like this:
//		float GetResponsesSoundRecord( HATTRIBUTE hStruct, uint32 nStructIndex ) { return GetStructAttribute( hStruct, nStructIndex, "SoundRecord" ); }
#define DEFINE_GETSTRUCTSTRUCT( ParentStruct, StructName ) \
	HATTRIBUTE Get##ParentStruct##StructName( HATTRIBUTE hStruct, uint32 nStructIndex ) const { return GetStructAttribute( hStruct, nStructIndex, #StructName ); }


// Macro to access an attribute defined by DEFINE_GETRECORDATTRIB/DEFINE_GETSTRUCTSTRUCT
// Usage:
//		float fValue = GETRECORDSTRUCT( hRecord, Responses );
//		This will define a call to an accessor like this:
//		float fValue = GetResponses( hRecord );
#define GETRECORDSTRUCT( hRecord, StructName ) \
	Get##StructName( hRecord )
#define GETSTRUCTSTRUCT( ParentStruct, hStruct, nStructIndex, StructName ) \
	Get##ParentStruct##StructName( hStruct, nStructIndex )

// Macro to declare in DB class to define a quick accessor to a specific attribute of a struct.
// Usage:
//		DEFINE_GETSTRUCTATTRIB( SlowMo, TransitionPeriod, float );
//		This will define a accessor like this:
//		float GetSlowMoTransitionPeriod( HATTRIBUTE hStruct, uint32 nStructIndex, uint32 nValueIndex = 0 ) const { return GetFromAttributeType< float >( GetStructAttribute( hStruct, "TransitionPeriod" ), nValueIndex ); }
#define DEFINE_GETSTRUCTATTRIB( StructName, AttribName, type ) \
	type Get##StructName##AttribName( HATTRIBUTE hStruct, uint32 nStructIndex, uint32 nValueIndex = 0 ) const { return GetFromAttributeType< type >( GetStructAttribute( hStruct, nStructIndex, #AttribName ), nValueIndex ); }

// Macro to access an attribute defined by DEFINE_GETRECORDATTRIB
// Usage:
//		float fValue = GETSTRUCTATTRIB( SlowMo, hStruct, 0, TransitionPeriod );
//		This will define a call to an accessor like this:
//		float fValue = GetSlowMoTransitionPeriod( hSturct, 0 );
#define GETSTRUCTATTRIB( StructName, hStruct, nStructIndex, AttribName ) \
	Get##StructName##AttribName( hStruct, nStructIndex )
#define GETSTRUCTATTRIBINDEX( StructName, hStruct, nStructIndex, AttribName, nValueIndex ) \
	Get##StructName##AttribName( hStruct, nStructIndex, nValueIndex )

class CGameDatabaseCreator
{
public :	// Methods...

	CGameDatabaseCreator( );
	virtual ~CGameDatabaseCreator( );

	HCATEGORYCREATOR CreateCategory( HDATABASECREATOR hDatabase, const char *pszCategory)
	{
		return g_pLTDatabaseCreator->CreateCategory(hDatabase,pszCategory,"");
	}

	HRECORDCREATOR CreateRecord( HDATABASECREATOR hDatabase, HCATEGORYCREATOR hCategory, const char *pszRecord )
	{
		LTUNREFERENCED_PARAMETER( hDatabase);
		return g_pLTDatabaseCreator->CreateRecord(hCategory,pszRecord);
	}

	HRECORDCREATOR CreateRecord( HDATABASECREATOR hDatabase, const char *pszCategory, const char *pszRecord )
	{
		HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDatabase,pszCategory,"");
		return g_pLTDatabaseCreator->CreateRecord(hCat,pszRecord);
	}


	//helper functions for database creation
	HATTRIBUTECREATOR CreateAttribute(HRECORDCREATOR hRec, const char* pszAttributeName, EAttributeType eType)
	{
		return g_pLTDatabaseCreator->CreateAttribute( hRec, pszAttributeName, eType, eAttributeUsage_Default, 1 );
	}


	bool CreateInt32(HRECORDCREATOR hRec, const char* pszAttributeName, int32 nVal)
	{
		return g_pLTDatabaseCreator->SetInt32( CreateAttribute(hRec,pszAttributeName,eAttributeType_Int32), 0, nVal );
	}

	bool CreateFloat(HRECORDCREATOR hRec, const char* pszAttributeName, float fVal)
	{
		return g_pLTDatabaseCreator->SetFloat( CreateAttribute(hRec,pszAttributeName,eAttributeType_Float), 0, fVal );
	}

	bool CreateBool(HRECORDCREATOR hRec, const char* pszAttributeName, bool bVal)
	{
		return g_pLTDatabaseCreator->SetBool( CreateAttribute(hRec,pszAttributeName,eAttributeType_Bool), 0, bVal );
	}
	bool CreateVector2(HRECORDCREATOR hRec, const char* pszAttributeName, const LTVector2& vVal)
	{
		return g_pLTDatabaseCreator->SetVector2( CreateAttribute(hRec,pszAttributeName,eAttributeType_Vector2), 0, vVal );
	}
	bool CreateVector3(HRECORDCREATOR hRec, const char* pszAttributeName, const LTVector& vVal)
	{
		return g_pLTDatabaseCreator->SetVector3( CreateAttribute(hRec,pszAttributeName,eAttributeType_Vector3), 0, vVal );
	}
	bool CreateVector4(HRECORDCREATOR hRec, const char* pszAttributeName, const LTVector4& vVal)
	{
		return g_pLTDatabaseCreator->SetVector4( CreateAttribute(hRec,pszAttributeName,eAttributeType_Vector4), 0, vVal );
	}

	bool CreateString(HRECORDCREATOR hRec, const char* pszAttributeName, const char* pszVal)
	{
		if (pszVal)
		{
			return g_pLTDatabaseCreator->SetString( CreateAttribute(hRec,pszAttributeName,eAttributeType_String), 0, pszVal );
		}
		return false;
	}
	bool CreateWString(HRECORDCREATOR hRec, const char* pszAttributeName, const wchar_t* pszVal)
	{
		if (pszVal)
		{
			return g_pLTDatabaseCreator->SetWString( CreateAttribute(hRec,pszAttributeName,eAttributeType_WString), 0, pszVal );
		}
		return false;
	}


};

//-----------------------------------------------------------------------------
// Abstract wrapper to allow us to treat records and structs uniformly.

struct DatabaseItem
{
	HATTRIBUTE hParent;
	INT nIndex;

	DatabaseItem() : hParent(NULL) {}

	DatabaseItem(HATTRIBUTE hParent, INT nIndex)
		: hParent(hParent), nIndex(nIndex){}

	DatabaseItem(const DatabaseItem& Other)
		: hParent(Other.hParent), nIndex(Other.nIndex){}

	operator bool() const
	{
		return( hParent != NULL );
	}

	HATTRIBUTE GetAttribute(const char* pszAttribute) const
	{
		EAttributeType eAttributeType = g_pLTDatabase->GetAttributeType(hParent);
		switch (eAttributeType)
		{
		case eAttributeType_RecordLink:
			{
				return g_pLTDatabase->GetAttribute(GetRecordLink(), pszAttribute);
			}

		case eAttributeType_Struct:
			{
				char szCompleteName[256];
				LTSNPrintF(szCompleteName, LTARRAYSIZE(szCompleteName),
					"%s.%d.%s", g_pLTDatabase->GetAttributeName(hParent), nIndex, pszAttribute);
				return g_pLTDatabase->GetAttribute(g_pLTDatabase->GetAttributeParent(hParent), szCompleteName);
			}
		};

		return NULL;
	}

	HRECORD GetRecordLink() const
	{
		return g_pLTDatabase->GetRecordLink(hParent, nIndex, NULL);
	}

	const char* GetName() const
	{
		EAttributeType eAttributeType = g_pLTDatabase->GetAttributeType(hParent);
		switch (eAttributeType)
		{
		case eAttributeType_RecordLink:
			{
				return g_pLTDatabase->GetRecordName(GetRecordLink());
			}

		case eAttributeType_Struct:
			{
				static char szCompleteName[256];	//!!ARL: Not re-entrant safe.
				LTSNPrintF(szCompleteName, LTARRAYSIZE(szCompleteName),
					"%s.%d", g_pLTDatabase->GetAttributeName(hParent), nIndex);
				return szCompleteName;
			}
		};
		return "<invalid>";
	}

#define QUERY_FUNC(_func,_type,_def)								\
	_type _func( const char* pszAttribute, _type def=_def ) const	\
	{																\
		HATTRIBUTE hAtt = GetAttribute(pszAttribute);				\
		LTASSERT( hAtt, "Attribute not found!" );					\
		return g_pLTDatabase->_func(hAtt, 0, def);					\
	}

	QUERY_FUNC(GetRecordLink,HRECORD,NULL)
	QUERY_FUNC(GetBool,bool,false)
	QUERY_FUNC(GetFloat,float,0.0f)
	QUERY_FUNC(GetString,const char*,"")
	QUERY_FUNC(GetVector2,LTVector2,LTVector2(0,0))
	QUERY_FUNC(GetVector3,LTVector,LTVector(0,0,0))
};

#endif // __DATABASE_UTILS_H__
