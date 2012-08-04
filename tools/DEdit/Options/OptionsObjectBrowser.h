// OptionsObjectBrowser.h: interface for the COptionsObjectBrowser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSOBJECTBROWSER_H__9C8B9304_03DA_11D3_BE1D_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSOBJECTBROWSER_H__9C8B9304_03DA_11D3_BE1D_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class COptionsObjectBrowser : public COptionsBase  
{
public:
	COptionsObjectBrowser();
	virtual ~COptionsObjectBrowser();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to options
	BOOL	IsGroupByType()					{ return m_bGroupByType; }
	void	SetGroupByType(BOOL bGroup)		{ m_bGroupByType=bGroup; }

protected:
	// Group the objects by their type
	BOOL	m_bGroupByType;
};

#endif // !defined(AFX_OPTIONSOBJECTBROWSER_H__9C8B9304_03DA_11D3_BE1D_0060971BDC6D__INCLUDED_)
