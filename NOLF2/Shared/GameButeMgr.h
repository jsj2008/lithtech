// ----------------------------------------------------------------------- //
//
// MODULE  : GameButeMgr.h
//
// PURPOSE : GameButeMgr definition - Base class for all bute mgrs
//
// CREATED : 3/30/99
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BUTE_MGR_H__
#define __GAME_BUTE_MGR_H__

#include "ButeMgr.h"

#pragma warning( disable : 4786 )
#include <hash_map>


void GBM_DisplayError(const char* szMsg);


// ----------------------------------------------------------------------- //
//
// Character pointer hash map definition...
//
// ----------------------------------------------------------------------- //

#if _MSC_VER == 1300

typedef std::hash_map< const char *, int, ButeMgrHashCompare > IndexTable;

#elif _MSC_VER > 1300

typedef stdext::hash_map< const char *, int, ButeMgrHashCompare > IndexTable;

#else

struct eqstr_nocase
{
  bool operator()(const char* s1, const char* s2) const
  {
	return stricmp(s1, s2) == 0;
  }
};

struct GBM_hash_str_nocase
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

typedef std::hash_map<const char *,int, GBM_hash_str_nocase, eqstr_nocase> IndexTable;

#endif // VC7



class CGameButeMgr
{
	public :

        virtual LTBOOL Init(const char* szAttributeFile="") = 0;
		virtual void Term( ) { m_buteMgr.Term( ); m_strAttributeFile.Empty( ); }

		CGameButeMgr()
		{
			m_buteMgr.Init(GBM_DisplayError);
            m_pCryptKey = LTNULL;
            m_bInRezFile = LTTRUE;
		}

		virtual ~CGameButeMgr() { Term( ); }

        inline void SetInRezFile(LTBOOL bInRezFile) { m_bInRezFile = bInRezFile; }

		char const* GetAttributeFile( ) { return m_strAttributeFile; }

		virtual void Save();

	protected :

		CString		m_strAttributeFile;
		CButeMgr	m_buteMgr;

		char*		m_pCryptKey;
        LTBOOL       m_bInRezFile;

        LTBOOL       Parse(const char* sButeFile);
};


#endif // __GAME_BUTE_MGR_H__
