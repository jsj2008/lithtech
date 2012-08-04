//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsRun.h: interface for the COptionsRun class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSRUN_H__889A7434_F8FE_11D2_BE1C_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSRUN_H__889A7434_F8FE_11D2_BE1C_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class COptionsRun : public COptionsBase  
{
public:
	COptionsRun();
	virtual ~COptionsRun();

	// Load/Save the options
	BOOL		Load();
	BOOL		Save();

	BOOL		LoadFromOptionsFile();	// support for project-based run options
	BOOL		SaveToOptionsFile();	// support for project-based run options

	// Access to the options
	const CString&	GetExecutable() const					{ return m_sExecutable; }
	const CString&	GetWorkingDirectory() const				{ return m_sWorkingDirectory; }
	const CString&	GetProgramArguments() const	 			{ return m_sProgramArguments; }
	const CString&	GetPackerDirectory() const				{ return m_sPackerDirectory; }

	void		SetExecutable(const char* sExecutable)		{ m_sExecutable=sExecutable; }
	void		SetWorkingDirectory(const char* sDirectory)	{ m_sWorkingDirectory=sDirectory; }
	void		SetProgramArguments(const char* sArguments)	{ m_sProgramArguments=sArguments; }
	void		SetPackerDirectory(const char* sDirectory)	{ m_sPackerDirectory=sDirectory; } 

protected:
	CString		m_sExecutable;			// The executable to run
	CString		m_sWorkingDirectory;	// The working directory
	CString		m_sProgramArguments;	// The program arguments
	CString		m_sPackerDirectory;
};

#endif 
