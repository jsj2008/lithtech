//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsPrimitives.cpp: implementation of the COptionsPrimitives class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "optionsprimitives.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsPrimitives::COptionsPrimitives()
{

}

COptionsPrimitives::~COptionsPrimitives()
{

}

/************************************************************************/
// Loads the registry information for the primitives
BOOL COptionsPrimitives::Load()
{
	LoadSphereOptions("Sphere", m_sphereOptions);
	LoadSphereOptions("Dome", m_domeOptions);

	return TRUE;
}

/************************************************************************/
// Saves the registry information for the primitives
BOOL COptionsPrimitives::Save()
{
	SaveSphereOptions("Sphere", m_sphereOptions);
	SaveSphereOptions("Dome", m_domeOptions);

	return FALSE;
}

/************************************************************************/
// Saves a sphere options struct with a prefix (either sphere or dome)
void COptionsPrimitives::SaveSphereOptions(CString sKeyPrefix, sphereOptionsStruct &source)
{
	SetDWordValue(sKeyPrefix+"NumSides", source.m_nSides);
	SetDWordValue(sKeyPrefix+"VerticalDivisions", source.m_nVerticalSubdivisions);
	SetDWordValue(sKeyPrefix+"Radius", (DWORD)(source.m_fRadius*1000.0f));
}

/************************************************************************/
// Saves a sphere options struct with a prefix (either sphere or dome)
void COptionsPrimitives::LoadSphereOptions(CString sKeyPrefix, sphereOptionsStruct &dest)
{
	dest.m_nSides=GetDWordValue(sKeyPrefix+"NumSides", 8);
	dest.m_nVerticalSubdivisions=GetDWordValue(sKeyPrefix+"VerticalDivisions", 4);
	dest.m_fRadius=(float)GetDWordValue(sKeyPrefix+"Radius", 64*1000)/1000.0f;
}
