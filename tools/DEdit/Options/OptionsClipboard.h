// OptionsClipboard.h: interface for the COptionsClipboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSCLIPBOARD_H__A6D7D3B2_0D74_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSCLIPBOARD_H__A6D7D3B2_0D74_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class COptionsClipboard : public COptionsBase  
{
public:
	COptionsClipboard();
	virtual ~COptionsClipboard();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to the options
	BOOL	GetGenerateUniqueNames()					{ return m_bGenerateUniqueNames; }
	BOOL	GetUpdateRefProps()							{ return m_bUpdateRefProps; }
	BOOL	GetDisplayNameChangeReport()				{ return m_bDisplayNameChangeReport; }

	void	SetGenerateUniqueNames(BOOL bGenerate)		{ m_bGenerateUniqueNames=bGenerate; }
	void	SetUpdateRefProps(BOOL bUpdate)				{ m_bUpdateRefProps=bUpdate; }
	void	SetDisplayNameChangeReport(BOOL bDisplay)	{ m_bDisplayNameChangeReport=bDisplay; }

protected:
	BOOL	m_bGenerateUniqueNames;		// Indicates that unique names should be generated for pasted objects
	BOOL	m_bUpdateRefProps;			// Indicates that object properties should be updated for pasted objects
	BOOL	m_bDisplayNameChangeReport;	// Indicates that a report of object name changes (if any) should be displayed
};

#endif // !defined(AFX_OPTIONSCLIPBOARD_H__A6D7D3B2_0D74_11D3_BE24_0060971BDC6D__INCLUDED_)
