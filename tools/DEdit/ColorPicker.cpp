//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ColorPicker.cpp: implementation of the CColorPicker class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "colorpicker.h"
#include "regmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern TCHAR szRegKeyCompany[];
extern TCHAR szRegKeyApp[];
extern TCHAR szRegKeyVer[];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CColorPicker::~CColorPicker()
{

}

// Initializes the color picker by loading the custom colors
// from the registry.
BOOL CColorPicker::Init()
{
	if (GetApp()->GetOptions().GetBinaryValue("CustomColors", (void *)&m_customColors, sizeof(m_customColors)))
	{	
		m_cc.lpCustColors=(COLORREF *)&m_customColors;
	}

	m_cc.Flags |= CC_FULLOPEN;

	return TRUE;
}

// Sets the current color.  Call this before calling DoModal
void CColorPicker::SetCurrentColor(COLORREF color)
{
	m_cc.Flags |= CC_RGBINIT;
	m_cc.rgbResult = color;									
}

// DoModal override which saves the custom colors to the registry
// before returning.
int CColorPicker::DoModal()
{
	int nResult=CColorDialog::DoModal();

	// Save the custom colors
	GetApp()->GetOptions().SetBinaryValue("CustomColors", (void *)m_cc.lpCustColors, sizeof(m_customColors));	

	return nResult;
}