// OptionsGenerateUniqueNames.h: interface for the COptionsGenerateUniqueNames class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSGENERATEUNIQUENAMES_H__9E2D3202_0D52_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSGENERATEUNIQUENAMES_H__9E2D3202_0D52_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class COptionsGenerateUniqueNames : public COptionsBase  
{
public:
	COptionsGenerateUniqueNames();
	virtual ~COptionsGenerateUniqueNames();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to options
	BOOL	GetUpdateRefProps()							{ return m_bUpdateRefProps; }
	BOOL	GetUpdateSelPropsOnly()						{ return m_bUpdateSelPropsOnly; }
	BOOL	GetDisplayReportOfChanges()					{ return m_bDisplayReportOfChanges; }

	void	SetUpdateRefProps(BOOL bUpdate)				{ m_bUpdateRefProps=bUpdate; }
	void	SetUpdateSelPropsOnly(BOOL bUpdate)			{ m_bUpdateSelPropsOnly=bUpdate; }
	void	SetDisplayReportOfChanges(BOOL bDisplay)	{ m_bDisplayReportOfChanges=bDisplay; }

protected:
	BOOL	m_bUpdateRefProps;
	BOOL	m_bUpdateSelPropsOnly;
	BOOL	m_bDisplayReportOfChanges;
};

#endif // !defined(AFX_OPTIONSGENERATEUNIQUENAMES_H__9E2D3202_0D52_11D3_BE24_0060971BDC6D__INCLUDED_)
