//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsBase.h: interface for the COptionsBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSBASE_H__A045C4D3_F2AD_11D2_BE15_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSBASE_H__A045C4D3_F2AD_11D2_BE15_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGenRegMgr;
class COptionsBase  
{
public:
	COptionsBase();
	virtual ~COptionsBase();

	// Intializes the options class with a GenRegMgr and a registry root path
	virtual BOOL	Init(CGenRegMgr *pRegMgr, CString sRegRoot);

	// Loads/Saves the options to the registry
	virtual BOOL	Load();
	virtual BOOL	Save();

	////////////////////////////////////////////////////////////////
	// These wrappers call into GenRegMgr and adds the root path

	// Set registry values
	BOOL	SetStringValue (LPCSTR strValueName, LPCSTR strValue);
	BOOL	SetDWordValue (LPCSTR strValueName, DWORD dwValue);
	BOOL	SetBinaryValue (LPCSTR strValueName, void* pData, DWORD dwDataSize);
	BOOL	SetBoolValue (LPCSTR strValueName, BOOL bValue);

	// retrieve registry value sizes
	DWORD	GetValueSize (LPCSTR strValueName);

	// retrieve registry values
	CString	GetStringValue(LPCSTR strValueName, CString sDefault="", BOOL *pbSuccess=NULL);
	DWORD	GetDWordValue (LPCSTR strValueName, DWORD dwDefault=0, BOOL *pbSuccess=NULL);
	BOOL	GetBinaryValue (LPCSTR strValueName, void* pBuffer, DWORD dwBufferSize);	
	BOOL	GetBoolValue (LPCSTR strValueName, BOOL bDefault=FALSE, BOOL *pbSuccess=NULL);

	// retrieve options-file values
	CString	GetStringValueFromOptionsFile(LPCSTR strValueName, CString sDefault="", BOOL *pbSuccess=NULL);
	
	// set options-file values
	BOOL	SetStringValueToOptionsFile(LPCSTR strValueName, LPCSTR sValue);

protected:
	CGenRegMgr	*m_pRegMgr;		// The registry manager
	CString		m_sRegRoot;		// The root registry path
};

#endif // !defined(AFX_OPTIONSBASE_H__A045C4D3_F2AD_11D2_BE15_0060971BDC6D__INCLUDED_)
