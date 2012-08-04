//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsClassDialog.h: interface for the COptionsClassDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSCLASSDIALOG_H__15797192_F420_11D2_BE16_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSCLASSDIALOG_H__15797192_F420_11D2_BE16_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class COptionsClassDialog : public COptionsBase  
{
public:
	COptionsClassDialog();
	virtual ~COptionsClassDialog();

	// Loads/Saves the class dialog options to the registry
	BOOL		Load();
	BOOL		Save();

	// Copies the recent classes from one CStringArray to
	// this classes internal array.
	void		SetRecentClasses(CStringArray &source)	{ m_recentClasses.Copy(source); }

	// Copies the recent classes from this classes internal array to
	// the one specified in dest.
	void		GetRecentClasses(CStringArray &dest)	{ dest.Copy(m_recentClasses); }

	////////////////////////////////
	// Access to member variables	
	void		SetMaxRecentClasses(int nMax)			{ m_nMaxRecentClasses=nMax; }
	void		SetShowTree(BOOL bShow)					{ m_bShowTree=bShow; }
	void		SetBindIndividually(BOOL bIndividual)	{ m_bBindIndividually=bIndividual; }
	
	int			GetMaxRecentClasses()					{ return m_nMaxRecentClasses; }
	BOOL		GetShowTree()							{ return m_bShowTree; }
	BOOL		GetBindIndividually()					{ return m_bBindIndividually; }

protected:
	CStringArray	m_recentClasses;		// The recent classes
	int				m_nMaxRecentClasses;	// The maximum number of recent classes that can be saved
	BOOL			m_bShowTree;			// TRUE if the class heirarchy should be shown
	BOOL			m_bBindIndividually;	// TRUE if brushes should be bound to individual brushes
};

#endif // !defined(AFX_OPTIONSCLASSDIALOG_H__15797192_F420_11D2_BE16_0060971BDC6D__INCLUDED_)
