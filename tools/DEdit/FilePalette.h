//
//   (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : FilePalette.h
//
//	PURPOSE	  : Defines the CFilePalette class, used to save a list 
//              of files as a "palette"
//
//	CREATED	  : October 3 1996
//
//
//------------------------------------------------------------------

#ifndef __FILEPALETTE_H__
#define __FILEPALETTE_H__


class CFilePalette : public CMoArray<CString>
{
private :
	// Internal default constructor to allow multiple constructors
	void ctor();

protected:
	// Members
	CString m_csHeader;
	CString m_csName;

	int m_iLastError;

	virtual void SetLastError(int iError) { m_iLastError = iError; };

public:

	// Constructors
	CFilePalette() { ctor(); };
	CFilePalette(UINT uHeader) { ctor(); m_csHeader.LoadString(uHeader); };
	CFilePalette(LPCTSTR pHeader) { ctor(); SetHeader(pHeader); };
	virtual ~CFilePalette() {};

	// Member access
	virtual void SetHeader(LPCTSTR pHeader) { m_csHeader = pHeader; };
	virtual LPCTSTR GetHeader() const { return m_csHeader; };

	virtual void SetName(LPCTSTR pName) { m_csName = pName; };
	virtual LPCTSTR GetName() const { return m_csName; };

	// Add an entry (Returns FALSE if the entry already exists)
	virtual BOOL AddEntry(LPCTSTR pFileName);
	// Remove an entry (Removes the number of instances of the entry that were removed)
	virtual int RemoveEntry(LPCTSTR pFileName);

	// File access  (Returns FALSE on failure)
	virtual BOOL Load(LPCTSTR pFileName, BOOL bQuiet = FALSE, BOOL bStrictType = TRUE);
	virtual BOOL Save(LPCTSTR pFileName, BOOL bQuiet = FALSE);

	// Error results
	enum EErrorResults {
		ERROR_OK = 0, // No error
		ERROR_OPEN = 1, // Error opening file
		ERROR_BADHEADER = 2  // Invalid file header
	};

	virtual int GetLastError() const { return m_iLastError; };
};

#endif //__FILEPALETTE_H__