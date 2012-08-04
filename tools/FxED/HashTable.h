#if !defined(__HASHTABLE_H__)
#define __HASHTABLE_H__

#include <afxcoll.h>

class CHashTable : public CMapStringToPtr
{
	public:
	
		BOOL		Exists( const char *pszKey );
		const char *AddKey( const char *pszKey );

		CString		GetNextKey( POSITION &pos );
		CString		GetHead( );
		CString		RemoveHead( );
		POSITION	Find( const char *pszKey );
};

inline BOOL CHashTable::Exists( const char *pszKey )
{
	void *pVal;

	if( !pszKey )
		return FALSE;

	if( !Lookup( pszKey, pVal ))
		return FALSE;

	return TRUE;
}

inline const char *CHashTable::AddKey( const char *pszKey )
{
	if( !pszKey )
		return NULL;

	SetAt( pszKey, NULL );

	return pszKey;
}

inline CString CHashTable::GetNextKey( POSITION &pos )
{
	CString sKey;
	void *pVal;

	GetNextAssoc( pos, sKey, pVal );
	return sKey;
}

inline CString CHashTable::GetHead( )
{
	CString sKey;
	POSITION pos;
	void *pVal;

	pos = GetStartPosition( );
	if( pos )
	{
		GetNextAssoc( pos, sKey, pVal );
	}

	return sKey;
}


inline CString CHashTable::RemoveHead( )
{
	CString sKey;
	POSITION pos;
	void *pVal;

	pos = GetStartPosition( );
	if( pos )
	{
		GetNextAssoc( pos, sKey, pVal );
		RemoveKey( sKey );
	}

	return sKey;
}

inline POSITION CHashTable::Find( const char *pszKey )
{
	CString sKey;
	POSITION pos, nextPos;

	pos = GetStartPosition( );
	if( pos )
	{
		nextPos = pos;
		sKey = GetNextKey( nextPos );
		if( sKey == pszKey )
			return pos;

		pos = nextPos;
	}

	return NULL;
}

#endif // __HASHTABLE_H__
