// OptionsWindows.h: interface for the COptionsWindows class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSWINDOWS_H__0367F6C0_2514_11D3_A607_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSWINDOWS_H__0367F6C0_2514_11D3_A607_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"
#include "mainfrm.h"

class COptionsWindows : public COptionsBase  
{
public:
	COptionsWindows();
	virtual ~COptionsWindows();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to the options
	int		GetNumMainControls()		{ return m_mainControlArray.GetSize(); }
	int		GetNumBarControls()			{ return m_barControlArray.GetSize(); }

	// Returns a control at a specific index
	CMainFrame::ProjectControl	GetMainControl(int nIndex)
	{
		return m_mainControlArray[nIndex];
	}

	// Returns a control at a specific index
	CMainFrame::ProjectControl	GetBarControl(int nIndex)
	{
		return m_barControlArray[nIndex];
	}

	// Set the control arrays
	void	SetMainControlArray(CMoArray<CProjectControlBarInfo *> &array);
	void	SetBarControlArray(CMoArray<CProjectControlBarInfo *> &array);

protected:
	CMoArray<CMainFrame::ProjectControl>	m_mainControlArray;
	CMoArray<CMainFrame::ProjectControl>	m_barControlArray;
};

#endif // !defined(AFX_OPTIONSWINDOWS_H__0367F6C0_2514_11D3_A607_0060971BDC6D__INCLUDED_)
