#if !defined(_BUTEMGR_H_)
#define _BUTEMGR_H_

// disable warning C4786: symbol greater than 255 character,
// okay to ignore
#pragma warning(disable: 4786)

#include "limits.h"
#include "float.h"
#include "cryptmgr.h"
#include "avector.h"
#include "arange.h"
#include "stdlith.h"

#if defined(_USE_REZFILE_)
#include "rezmgr.h"
#endif

#include "lithtypes.h"

#include <map>
#include <set>
#include <functional>
#include <hash_set>
#include <hash_map>
#if _MSC_VER >= 1300
#	include <iosfwd>
#	include <strstream>
#	include <iostream>
#else
#	include <istream>
#	include "fstream.h"
#	include "strstrea.h"
#endif // VC7
#include <fstream>


#if _MSC_VER >= 1300

class ButeMgrHashCompare
{
public:

	// parameters for hash table
	enum
	{
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8		// min_buckets = 2 ^^ N, 0 < N
	};

	ButeMgrHashCompare()
	{
	}

	size_t 		operator()(const char *pKey) const
	{
		// hash _Keyval to size_t value
		return Convert(pKey);
	}

	bool 		operator()(const char *pKey1, const char *pKey2) const
	{
		// test if _Keyval1 ordered before _Keyval2
		return Compare(pKey1, pKey2);
	}

private:

	size_t		Convert(const char *pKey) const
	{
		uint32 nHash = 0;
		for (; *pKey; ++pKey)
		{
			nHash = 13 * nHash + (toupper(*pKey) - '@');
		}

		return nHash;
	}

	bool 		Compare(const char *lhs, const char *rhs) const
	{
		return stricmp(lhs, rhs) < 0;
	}
};

#else

struct equal_str
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) == 0;
	}
};

struct equal_str_nocase
{
	bool operator()(const char* s1, const char* s2) const
	{
		return stricmp(s1, s2) == 0;
	}
};

struct hash_str_nocase
{
	// Copied for stl-port's std::hash<const char*>.
	// Added tolower function on the string.
	unsigned long operator()(const char* str) const
	{
	  unsigned long hash = 0;
	  for ( ; *str; ++str)
		  hash = 5*hash + tolower(*str);

	  return hash;
	}
};

#endif // VC7



class CButeMgr
{

public:

	enum SymTypes { NullType, IntType, DwordType, ByteType, BoolType, DoubleType, FloatType, StringType, RectType, PointType, VectorType, RangeType };

	class CSymTabItem
	{
	public:

		SymTypes SymType;

		CSymTabItem( ) { SymType = NullType; m_pButeMgr = NULL; }

		void Init(CButeMgr &bm, SymTypes t, int val) { Term( ); m_pButeMgr = &bm; SymType = t; data.i = val; }
		void Init(CButeMgr &bm, SymTypes t, DWORD val) { Term( ); m_pButeMgr = &bm; SymType = t; data.dw = val; }
		void Init(CButeMgr &bm, SymTypes t, BYTE val) { Term( ); m_pButeMgr = &bm; SymType = t; data.byte = val; }
		void Init(CButeMgr &bm, SymTypes t, bool val) { Term( ); m_pButeMgr = &bm; SymType = t; data.b = val; }
		void Init(CButeMgr &bm, SymTypes t, float val) { Term( ); m_pButeMgr = &bm; SymType = t; data.f = val; }
		void Init(CButeMgr &bm, SymTypes t, double val) { Term( ); m_pButeMgr = &bm; SymType = t; data.d = val; }
		void Init(CButeMgr &bm, SymTypes t, const char * val) { Term( ); m_pButeMgr = &bm; SymType = t; data.s = &bm.AddString( val ); }
		void Init(CButeMgr &bm, SymTypes t, const CRect& val) { Term( ); m_pButeMgr = &bm; SymType = t; data.r = bm.m_RectBank.Allocate( ); *data.r = val; }
		void Init(CButeMgr &bm, SymTypes t, const CPoint& val) { Term( ); m_pButeMgr = &bm; SymType = t; data.point = bm.m_PointBank.Allocate( ); *data.point = val; }
		void Init(CButeMgr &bm, SymTypes t, const CAVector& val) { Term( ); m_pButeMgr = &bm; SymType = t; data.v = bm.m_AVectorBank.Allocate( ); *data.v = val; }
		void Init(CButeMgr &bm, SymTypes t, const CARange& val) { Term( ); m_pButeMgr = &bm; SymType = t; data.range = bm.m_ARangeBank.Allocate( ); *data.range = val; }

		~CSymTabItem()
		{
			Term( );
		}

		void Term( )
		{
			if( !m_pButeMgr )
				return;

			switch (SymType)
			{
			case StringType:
				// This doesn't actually delete the string that was added
				// to the string holder.  Because the stringholder strings aren't
				// reference counted, this is not possible.  This will be ok
				// as long as unique strings aren't continuously generated.
				data.s = NULL;
				break;
			case RectType:
				m_pButeMgr->m_RectBank.Free( data.r );
				data.r = NULL;
				break;
			case PointType:
				m_pButeMgr->m_PointBank.Free( data.point );
				data.point = NULL;
				break;
			case VectorType:
				m_pButeMgr->m_AVectorBank.Free( data.v );
				data.v = NULL;
				break;
			case RangeType:
				m_pButeMgr->m_ARangeBank.Free( data.range );
				data.range = NULL;
				break;
			}
		}

		union
		{
			int i;
			DWORD dw;
			BYTE byte;
			bool b;
			double d;
			float f;
			CString const* s;
			CRect* r;
			CPoint* point;
			CAVector* v;
			CARange* range;
		} data;

		CButeMgr* m_pButeMgr;

		private:

			// Prevent use of copy constructor and assignment op.
			// Will cause compile error if used.
			CSymTabItem(const CSymTabItem& sti);
			const CSymTabItem& operator=(const CSymTabItem& sti);

	};


	friend class CSymTabItem;

public:

	CButeMgr();
	virtual ~CButeMgr();

	void Init();
	void Init(void (*pF)(const char* szMsg));

	void Term();

	DWORD GetChecksum() { return m_checksum; }
	void SetDisplayFunc(void (*pF)(const char* szMsg)) { m_pDisplayFunc = pF; }
	CString GetErrorString() { return m_sErrorString; }

#if _MSC_VER >= 1300
	bool Parse( std::istream& iStream, int decryptCode = 0);
	bool Parse( std::istream& iCrypt, std::streamsize nLen, const char* cryptKey);
#else
	bool Parse( istream& iStream, int decryptCode = 0);
	bool Parse( istream& iCrypt, streamsize nLen, const char* cryptKey);
#endif // VC7

#if defined(_USE_REZFILE_)
	bool Parse(CRezItm* pItem, int decryptCode = 0);
	bool Parse(CRezItm* pItem, const char* cryptKey);
#endif
	bool Parse(CString sAttributeFilename, int decryptCode = 0);
	bool Parse(CString sAttributeFilename, const char* cryptKey);
	bool Parse(void* pData, unsigned long size, int decryptCode = 0, CString sAttributeFilename="");
	bool Parse(void* pData, unsigned long size, const char* cryptKey, CString sAttributeFilename="");

	bool Save(const char* szNewFileName = NULL);

	typedef bool (*GetTagsCallback)( const char* pszTagName, void* pContext );
	void GetTags( GetTagsCallback pCallback, void* pContext = NULL);

	typedef bool (*GetKeysCallback)( const char* pszKeyName, CSymTabItem* pItem, void* pContext );
	void GetKeys( const char* pszTagName, GetKeysCallback pCallback, void* pContext = NULL);

	int GetInt(const char* szTagName, const char* szAttName, int defVal);
	int GetInt(const char* szTagName, const char* szAttName);
	void SetInt(const char* szTagName, const char* szAttName, int val);

	DWORD GetDword(const char* szTagName, const char* szAttName, DWORD defVal);
	DWORD GetDword(const char* szTagName, const char* szAttName);
	void SetDword(const char* szTagName, const char* szAttName, DWORD val);

	BYTE GetByte(const char* szTagName, const char* szAttName, BYTE defVal);
	BYTE GetByte(const char* szTagName, const char* szAttName);
	void SetByte(const char* szTagName, const char* szAttName, BYTE val);

	bool GetBool(const char* szTagName, const char* szAttName, bool defVal);
	bool GetBool(const char* szTagName, const char* szAttName);
	void SetBool(const char* szTagName, const char* szAttName, bool val);

	float GetFloat(const char* szTagName, const char* szAttName, float defVal);
	float GetFloat(const char* szTagName, const char* szAttName);
	void SetFloat(const char* szTagName, const char* szAttName, float val);

	double GetDouble(const char* szTagName, const char* szAttName, double defVal);
	double GetDouble(const char* szTagName, const char* szAttName);
	void SetDouble(const char* szTagName, const char* szAttName, double val);

	void GetString(const char* szTagName, const char* szAttName, const char* defVal, char *szResult, DWORD maxLen);
	void GetString(const char* szTagName, const char* szAttName, char *szResult, DWORD maxLen);
	void SetString(const char* szTagName, const char* szAttName, const char  *val);

	CRect& GetRect(const char* szTagName, const char* szAttName, CRect& defVal);
	CRect& GetRect(const char* szTagName, const char* szAttName);
	void SetRect(const char* szTagName, const char* szAttName, const CRect& val);

	CPoint& GetPoint(const char* szTagName, const char* szAttName, CPoint& defVal);
	CPoint& GetPoint(const char* szTagName, const char* szAttName);
	void SetPoint(const char* szTagName, const char* szAttName, const CPoint& val);

	CAVector& GetVector(const char* szTagName, const char* szAttName, CAVector& defVal);
	CAVector& GetVector(const char* szTagName, const char* szAttName);
	void SetVector(const char* szTagName, const char* szAttName, const CAVector& val);

	CARange& GetRange(const char* szTagName, const char* szAttName, CARange& defVal);
	CARange& GetRange(const char* szTagName, const char* szAttName);
	void SetRange(const char* szTagName, const char* szAttName, const CARange& val);

	bool AddTag(const char *szTagName);

	CButeMgr::SymTypes GetType(const char* szTagName, const char* szAttName);  // returns NullType if tag/key doesn't exist

	bool Success() { return m_bSuccess; }
	bool Exist(const char* szTagName, const char* szAttName = NULL);

	const char * GetString(const char* szTagName, const char* szAttName, const char * defVal);
	const char * GetString(const char* szTagName, const char* szAttName);

private:

	void Reset();

	void DisplayMessage(const char* szMsg, ...);

	// Parser stuff
	bool Statement();
	bool StatementList();
	bool Tag();
	bool TagList();

	// Scanner stuff
	bool Match(int tok);
	void ConsumeChar();
	short CharClass(unsigned char currentChar);
	short Action( short State, unsigned char currentChar );
	short NextState( short State, unsigned char currentChar );
	void LookupCodes( short State, unsigned char currentChar);
	bool ScanTok();

	DWORD m_decryptCode;
	DWORD m_checksum;
	int m_lineNumber;
	bool m_bLineCounterFlag;

	bool m_bSuccess;
	bool m_bErrorFlag;
	CString m_sErrorString;

	void (*m_pDisplayFunc)(const char* szMsg);

#if _MSC_VER == 1300 // NET 2002
	// Used to define dictionary of strings.
	// This must be case sensitive!
	typedef std::hash_set< CString, ButeMgrHashCompare > StringHolder;

	// Used to define map of strings to CSymTabItems.
	typedef std::hash_map< char const*, CSymTabItem*, ButeMgrHashCompare > TableOfItems;

	// Used to define map of strings to TableOfItems.
	typedef std::hash_map< char const*, TableOfItems*, ButeMgrHashCompare > TableOfTags;
#elif _MSC_VER > 1300  // NET 2003
	// Used to define dictionary of strings.
	// This must be case sensitive!
	typedef stdext::hash_set< CString, ButeMgrHashCompare > StringHolder;

	// Used to define map of strings to CSymTabItems.
	typedef stdext::hash_map< char const*, CSymTabItem*, ButeMgrHashCompare > TableOfItems;

	// Used to define map of strings to TableOfItems.
	typedef stdext::hash_map< char const*, TableOfItems*, ButeMgrHashCompare > TableOfTags;
#else
	// Used to define dictionary of strings.
	// This must be case sensitive!
	typedef std::hash_set< CString, std::hash< char const* >, equal_str > StringHolder;

	// Used to define map of strings to CSymTabItems.
	typedef std::hash_map< char const*, CSymTabItem*, hash_str_nocase, equal_str_nocase > TableOfItems;

	// Used to define map of strings to TableOfItems.
	typedef std::hash_map< char const*, TableOfItems*, hash_str_nocase, equal_str_nocase > TableOfTags;
#endif // .NET 2002


	// Holds all strings for keys and string values.
	StringHolder m_stringHolder;

	// Main table of tags.  This table is created when a butefile is parsed.
	TableOfTags m_tagTab;

	// Auxiliary table of tags.  This table is created by creating new attributes in a tag that
	// already exists in the main table.
	TableOfTags m_auxTagTab;

	// New tags.  This table is created by creating new tags that don't exist in the main
	// table of tags.
	TableOfTags m_newTagTab;

	// Current table of items.
	TableOfItems* m_pCurrTabOfItems;


	// Callback function passed to TraverseTableOfTags.  Returns false to stop iterating.
	typedef bool (*TraverseTableOfTagsCallback)( char const* pszTagName, TableOfItems& theTableOfItems, void* pContext );

	// Calls callback for each TableOfItems in theTableOfTags.  Passes pContext to callback.
	static void TraverseTableOfTags( TableOfTags& theTableOfTags, TraverseTableOfTagsCallback pCb, void* pContext );

	// Callback function passed to TraverseTableOfItems.  Returns false to stop iterating.
	typedef bool (*TraverseTableOfItemsCallback)( char const* pszAttName, CSymTabItem& theItem, void* pContext );

	// Calls callback for each CSymTabItem in theTableOfItems.  Passes pContext to callback.
	static void TraverseTableOfItems( TableOfItems& theTableOfItems, TraverseTableOfItemsCallback pCb, void* pContext );


	static bool AuxTabItemsSave( const char* pszAttName, CSymTabItem& theItem, void* pContext );
	static bool NewTabsSave( const char* pszTagName, TableOfItems& theTabOfItems, void* pContext );

	struct GetKeysData
	{
		GetKeysData( GetKeysCallback pCallback, void* pContext )
		{
			m_pCallback = pCallback;
			m_pContext = pContext;
		}
		GetKeysCallback m_pCallback;
		void* m_pContext;
	};
	static bool GetKeysTraverseFunc( char const* pszAttName, CSymTabItem& theItem, void* pContext );

	struct GetTagsData
	{
		GetTagsData( GetTagsCallback pCallback, void* pContext )
		{
			m_pCallback = pCallback;
			m_pContext = pContext;
		}
		GetTagsCallback m_pCallback;
		void* m_pContext;
	};
	static bool GetTagsTraverseFunc( char const* pszTagName, TableOfItems& theTableOfItems, void* pContext );

#if _MSC_VER >= 1300
	std::istream* m_pData;
	std::iostream *m_pSaveData;
#else
	istream* m_pData;
	iostream *m_pSaveData;
#endif // VC7

	char * m_szLineBuffer;
	const char * m_szLineBufferPtr;

	unsigned char  m_currentChar;
	short m_token;
	short m_tokenMinor;

	char m_szTokenString[4096];

	CString m_sTagName;
	CString m_sAttribute;

	CString m_sAttributeFilename;

	bool m_bPutChar;

	bool m_bCrypt;
	CCryptMgr m_cryptMgr;


	// Creates a new TableOfItems and adds it to a TableOfTags.
	TableOfItems* CreateTableOfItems( TableOfTags& tableOfTags, char const* pszTagName );

	// Finds a pre-existing TableOfItems in a TableOfTags.
	TableOfItems* FindTableOfItems( TableOfTags& tableOfTags, char const* pszTagName );

	// Creates a new CSymTabItem and adds it to a TableOfItems.
	CSymTabItem* CreateSymTabItem( TableOfItems& tableOfItems, char const* pszAttName );

	// Finds a pre-existing CSymTabItem in a TableOfItems.
	CSymTabItem* FindSymTabItem( TableOfItems& tableOfItems, char const* pszAttName );

	// Finds a pre-existing CSymTabItem from any of the TableOfItems.
	CSymTabItem* FindSymTabItem( char const* pszTagName, char const* pszAttName );

	// Gets a symtabitem from any of the tags, or creates one if not available.
	CSymTabItem* GetSymTabItem( char const* pszTagName, char const* pszAttName );

	// Called by destructor to delete the symtabitems.
	static bool TraverseTableOfItemsDeleteSymTab( char const* pszAttName, CSymTabItem& theSymTab, void* pContext );
	static bool TraverseTableOfTagsDeleteSymTab( char const* pszTagName, TableOfItems& theTableOfItems,
											   void* pContext );

	// Adds string to string list and returns the pointer to the string in the list.
	CString const& AddString( char const* pszString );

	ObjectBank< CRect > m_RectBank;
	ObjectBank< CPoint > m_PointBank;
	ObjectBank< CAVector > m_AVectorBank;
	ObjectBank< CARange > m_ARangeBank;
	ObjectBank< CSymTabItem > m_SymTabItemBank;
};


#if _MSC_VER >= 1300
inline bool CButeMgr::Parse( std::istream& iStream, int decryptCode)
#else
inline bool CButeMgr::Parse( istream& iStream, int decryptCode)
#endif // VC7
{
	Reset();
	m_pData = &iStream;
	m_decryptCode = decryptCode;

	bool retVal = true;
	if (!TagList())
	{
		m_bErrorFlag = true;
		retVal = false;
	}

	m_pData = NULL;

	return retVal;
}

#if _MSC_VER >= 1300
inline bool CButeMgr::Parse( std::istream& iCrypt, std::streamsize nLen, const char* cryptKey)
#else
inline bool CButeMgr::Parse( istream& iCrypt, streamsize nLen, const char* cryptKey)
#endif // VC7
{
	m_bCrypt = true;
	m_cryptMgr.SetKey(cryptKey);
	char* buf2 = new char[(unsigned int)nLen];
#if _MSC_VER >= 1300
	std::ostrstream* pOss = new std::ostrstream(buf2, nLen);
#else
	ostrstream* pOss = new ostrstream(buf2, nLen);
#endif // VC7
	m_cryptMgr.Decrypt(iCrypt, *pOss);

#if _MSC_VER >= 1300
	std::istrstream* pIStream = new std::istrstream(const_cast<const char *>(buf2), pOss->pcount());
#else
	istrstream* pIStream = new istrstream(buf2, pOss->pcount());
#endif // VC7

	delete pOss;

	Reset();

	bool retVal = Parse( *pIStream );

	delete pIStream;
	delete buf2;
	return retVal;
}



#if defined(_USE_REZFILE_)
inline bool CButeMgr::Parse(CRezItm* pItem, int decryptCode)
{
	if (!pItem)
		return false;
#if _MSC_VER >= 1300
	std::istrstream* pIStream = new std::istrstream((char*)pItem->Load(), pItem->GetSize());
#else
	istrstream* pIStream = new istrstream((char*)pItem->Load(), pItem->GetSize());
#endif // VC7
	if( !pIStream )
		return false;

	retVal = Parse( *pIStream, decryptCode );

	pItem->UnLoad();
	delete pIStream;

	return retVal;
}


inline bool CButeMgr::Parse(CRezItm* pItem, const char* cryptKey)
{
	if (!pItem)
		return false;
	char* buf1 = (char*)pItem->Load();
	int len = pItem->GetSize();
#if _MSC_VER >= 1300
	std::istrstream* pIss = new std::istrstream(buf1, len);
#else
	istrstream* pIss = new istrstream(buf1, len);
#endif // VC7

	Reset();

	retVal = Parse( *pIss, len, cryptKey );

	pItem->UnLoad();
	delete pIss;
	return retVal;
}
#endif





inline bool CButeMgr::Parse(void* pData, unsigned long size, int decryptCode,
							CString sAttributeFilename)
{
	if (!pData)
		return false;
#if _MSC_VER >= 1300
	std::istrstream* pIStream = new std::istrstream((char*)pData, size);
#else
	istrstream* pIStream = new istrstream((char*)pData, size);
#endif // VC7

	// Need to set the attribute filename if we want to Save the butemgr later...
	m_sAttributeFilename = sAttributeFilename;

	bool retVal = Parse( *pIStream, decryptCode );

	delete pIStream;

	return retVal;
}




inline bool CButeMgr::Parse(void* pData, unsigned long size, const char* cryptKey,
							CString sAttributeFilename)
{
	if (!pData)
		return false;
	char* buf1 = (char*)pData;
	int len = size;
#if _MSC_VER >= 1300
	std::istrstream* pIss = new std::istrstream(buf1, len);
#else
	istrstream* pIss = new istrstream(buf1, len);
#endif // VC7

	Reset();

	// Need to set the attribute filename if we want to Save the butemgr later...
	m_sAttributeFilename = sAttributeFilename;

	bool retVal = Parse( *pIss, len, cryptKey );

	delete pIss;

	return retVal;
}




inline bool CButeMgr::Parse(CString sAttributeFilename, int decryptCode)
{
#if _MSC_VER >= 1300
	std::ifstream* pIStream = new std::ifstream(sAttributeFilename, std::ios_base::in);
#else
	ifstream* pIStream = new ifstream(sAttributeFilename, ios::in | ios::nocreate);
#endif // VC7
	if( !pIStream )
		return false;
	if (pIStream->fail())
	{
		delete pIStream;
		return false;
	}

	Reset();

	m_sAttributeFilename = sAttributeFilename;

	bool retVal = Parse( *pIStream, decryptCode );

	delete pIStream;
	return retVal;
}



inline bool CButeMgr::Parse(CString sAttributeFilename, const char* cryptKey)
{
#if _MSC_VER >= 1300
	std::ifstream* pIs = new std::ifstream(sAttributeFilename, std::ios_base::binary);
#else
	ifstream* pIs = new ifstream(sAttributeFilename, ios::nocreate | ios::binary);
#endif // VC7
	if (!pIs)
		return false;
	if (pIs->fail())
	{
		delete pIs;
		return false;
	}

#if _MSC_VER >= 1300
	pIs->seekg(0, std::ios_base::end);
#else
	pIs->seekg(0, ios::end);
#endif // VC7
	std::streamoff len = pIs->tellg();

	pIs->seekg(0);

	Reset();
	m_sAttributeFilename = sAttributeFilename;

	bool retVal = Parse( *pIs, len, cryptKey );

	delete pIs;
	return retVal;
}





#endif
