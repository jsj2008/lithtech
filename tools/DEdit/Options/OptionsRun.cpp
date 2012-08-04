//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsRun.cpp: implementation of the COptionsRun class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "optionsrun.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsRun::COptionsRun()
{

}

COptionsRun::~COptionsRun()
{

}

/************************************************************************/
// Load the options
BOOL COptionsRun::Load()
{
#ifdef _PUBLIC_DEVELOPMENT
	// Load the executable path
	m_sExecutable=GetStringValue("ExecutablePath", "LithTechDev.exe");
#else // _PUBLIC_DEVELOPMENT
	// Load the executable path
	m_sExecutable=GetStringValue("ExecutablePath", "LithTech.exe");
#endif // _PUBLIC_DEVELOPMENT

	// Load the working directory
	m_sWorkingDirectory=GetStringValue("WorkingDirectory", "..");

	// Load the program arguments
	m_sProgramArguments=GetStringValue("ProgramArguments", "-rez engine.rez -rez Game +runworld \"%WorldName%\"");

	// Load the packer directory
	m_sPackerDirectory=GetStringValue("PackerDirectory", ".");

	return TRUE;
}

BOOL COptionsRun::LoadFromOptionsFile()
{
	// Load the executable path
	m_sExecutable=GetStringValueFromOptionsFile("ExecutablePath", m_sExecutable);

	// Load the working directory
	m_sWorkingDirectory=GetStringValueFromOptionsFile("WorkingDirectory", m_sWorkingDirectory);

	// Load the program arguments
	m_sProgramArguments=GetStringValueFromOptionsFile("ProgramArguments", m_sProgramArguments);

	// Load the packer directory
	m_sPackerDirectory=GetStringValueFromOptionsFile("PackerDirectory", m_sPackerDirectory);

	return TRUE;
}

/************************************************************************/
// Save the options
BOOL COptionsRun::Save()
{
	// Save the executable path
	SetStringValue("ExecutablePath", m_sExecutable);

	// Save the working directory
	SetStringValue("WorkingDirectory", m_sWorkingDirectory);

	// Save the program arguments
	SetStringValue("ProgramArguments", m_sProgramArguments);

	// Save the packer directory
	SetStringValue("PackerDirectory", m_sPackerDirectory);

	return TRUE;
}

BOOL COptionsRun::SaveToOptionsFile()
{
	// Load the executable path
	SetStringValueToOptionsFile("ExecutablePath", m_sExecutable);

	// Load the working directory
	SetStringValueToOptionsFile("WorkingDirectory", m_sWorkingDirectory);

	// Load the program arguments
	SetStringValueToOptionsFile("ProgramArguments", m_sProgramArguments);

	// Load the packer directory
	SetStringValueToOptionsFile("PackerDirectory", m_sPackerDirectory);

	return TRUE;
}

