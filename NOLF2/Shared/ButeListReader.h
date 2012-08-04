// ----------------------------------------------------------------------- //
//
// MODULE  : ButeListReader.h
//
// PURPOSE : ButeListReader - Declaration.  Used to read 
//			 lists from bute.txt files into various buteMgrs.
//
// CREATED : 5/30/2001
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BUTE_LIST_READER_H__
#define __BUTE_LIST_READER_H__

class CButeMgr;

class CButeListReader
{

public:

	 CButeListReader();
	~CButeListReader();

	void	Read(CButeMgr* pButeMgr, const char* pszTagName, const char* pszAttName, 
				 const uint32 nStrLen);

	void	CopyList(uint8 iStart, char* paszDest, uint32 nStrLen) const;
	void	ReadAndCopy(CButeMgr* pButeMgr, const char* pszTagName, const char* pszAttName, 
						char* paszDest, uint32 nStrLen);

	uint8	GetNumItems() const { return m_cItems; }
	char const *GetItem(uint8 iItem) const { return m_szItems[iItem]; }

	void	SetItem(uint8 iItem, const char* szItem, const uint32 nStrLen);

private:

	uint8		m_cItems;
	char**		m_szItems;

	static char ms_aAttName[100];
};

#endif
