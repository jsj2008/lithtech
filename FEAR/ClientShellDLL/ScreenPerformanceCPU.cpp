// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformanceCPU.cpp
//
// PURPOSE : Defines the CScreenPerformanceCPU class.  This class sets
//           the CPU performance options.
//
// CREATED : 04/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPerformanceCPU.h"
#include "PerformanceMgr.h"
#include "ScreenCommands.h"
#include "PerformanceDB.h"


CScreenPerformanceCPU::CScreenPerformanceCPU() :
	CScreenPerformanceAdvanced( "PerformanceCPU_Title", 0 ),
	m_dwFlags(0)
{
}

CScreenPerformanceCPU::~CScreenPerformanceCPU()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceCPU::OnFocus()
//
//	PURPOSE:	join/leave screen
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceCPU::OnFocus(bool bFocus)
{
	if( bFocus )
	{
		// clear all warning flags
		m_dwFlags &= ~eFlagWarningMask;
	}
	else
	{

	}

	CScreenPerformanceAdvanced::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceCPU::Load()
//
//	PURPOSE:	read in the settings and apply them to the controls
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceCPU::Load()
{
	CScreenPerformanceAdvanced::Load();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceCPU::Save()
//
//	PURPOSE:	get settings from controls, and apply them
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceCPU::Save()
{
	CScreenPerformanceAdvanced::Save();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceCPU::UpdateOption()
//
//	PURPOSE:	overridden to test for special cases
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceCPU::UpdateOption( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex )
{
	LTASSERT( pCtrl, "Invalid control pointer" );
	if( !pCtrl )
		return;

	HRECORD hOptRec = CPerformanceMgr::Instance().GetOptionRecord(m_nGlobalType, nGroup, nIndex);
	if( !hOptRec )
		return;

	const char* pszRecordName = g_pLTDatabase->GetRecordName( hOptRec );
	if( LTStrIEquals( pszRecordName, "WorldDetail" ))
	{
		UpdateOption_WorldDetail( pCtrl, nGroup, nIndex );
	}
	else
	{
		CScreenPerformanceAdvanced::UpdateOption( pCtrl, nGroup, nIndex );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceCPU::UpdateOption_WorldDetail()
//
//	PURPOSE:	updates the world detail option.  overridden to display a 
//				message box to warn the user of object removal
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceCPU::UpdateOption_WorldDetail( CLTGUIColumnCtrl *pCtrl, int nGroup, int nIndex )
{
	CScreenPerformanceAdvanced::UpdateOption( pCtrl, nGroup, nIndex );

	CLTGUITextCtrl *pColumn = pCtrl->GetColumn( 1 );
	if( !pColumn )
		return;

	const wchar_t *pwszCurrentDetailName = pColumn->GetString();

	if( !(m_dwFlags & eFlag_WorldDetailWarning) &&
		!LTStrIEquals( pwszCurrentDetailName, LoadString( "PerformanceOptionSetting_Maximum" )) )
	{
		MBCreate mb;
		mb.eType = LTMB_OK;
		g_pInterfaceMgr->ShowMessageBox( "PerformanceMessage_WorldDetail", &mb );
		m_dwFlags |= eFlag_WorldDetailWarning;
	}
}
