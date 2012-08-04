// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformanceAdvanced.cpp
//
// PURPOSE : Defines the CScreenPerformanceAdvanced class.  This class
//           handles an advanced performance menu.  All advanced
//           performance menus inherit from this class.
//
// CREATED : 04/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPerformanceAdvanced.h"
#include "PerformanceMgr.h"
#include "ScreenCommands.h"
#include "PerformanceDB.h"

static const uint32 kOffset = 1024;

// number of controls in the list before the actual options
static const uint32 kCtrlOffset = 1;

CScreenPerformanceAdvanced::CScreenPerformanceAdvanced( const char* pszTitleStringID, uint32 nGlobalType ) :
	m_pszTitleStringID(pszTitleStringID),
	m_nGlobalType(nGlobalType),
	m_nType(0)
{
}

CScreenPerformanceAdvanced::~CScreenPerformanceAdvanced()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceAdvanced::Build()
//
//	PURPOSE:	build the controls used on the screen
//
// ----------------------------------------------------------------------- //

bool CScreenPerformanceAdvanced::Build()
{
	CreateTitle(m_pszTitleStringID);

	m_nOptionWidth	= g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);
	m_nSettingWidth	= g_pLayoutDB->GetListColumnWidth(m_hLayout,0,1);
	m_ListRect		= g_pLayoutDB->GetListRect(m_hLayout,0);
	m_nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	m_nWarningColor	= g_pLayoutDB->GetInt32(m_hLayout, LDB_ScreenAddColor, 0);

	LTVector2n topPos(m_ScreenRect.Left(),m_ListRect.Top());
	LTVector2n sz(m_ListRect.Left() - topPos.x,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	LTVector2n vIndent = g_pLayoutDB->GetListIndent(m_hLayout,0);
	uint8 nExpand = g_pLayoutDB->GetListFrameExpand(m_hLayout,0); 

	for (uint32 nGroup = 0; nGroup < CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType); nGroup++)
	{
		HRECORD hRec = CPerformanceMgr::Instance().GetGroupRecord(m_nGlobalType,nGroup);
		if (!hRec)
		{
			continue;
		}

		{
			CLTGUICtrl_create cs;

			if( nGroup == 0 )
			{
				// first control is absolute
				cs.rnBaseRect.m_vMin = topPos;
				cs.rnBaseRect.m_vMax = topPos + sz;
			}
			else
			{
				// the rest are relative to the first
				cs.rnBaseRect.m_vMin.Init();
				cs.rnBaseRect.m_vMax = sz;
			}
			cs.nCommandID = CMD_CUSTOM + nGroup;
			cs.szHelpID = DATABASE_CATEGORY( PerformanceGroup ).GETRECORDATTRIB(hRec, Help );
			CLTGUITextCtrl *pCtrl = AddTextItem(DATABASE_CATEGORY( PerformanceGroup ).GETRECORDATTRIB(hRec, Name ),cs);
		}
	}

	for (uint32 nGroup = 0; nGroup < CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType); nGroup++)
	{
		HRECORD hRec = CPerformanceMgr::Instance().GetGroupRecord(m_nGlobalType,nGroup);
		if (!hRec)
		{
			continue;
		}

		CLTGUIListCtrl_create lcs;
		lcs.bArrows = false;
		lcs.rnBaseRect = m_ListRect;
		lcs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 

		m_pList[nGroup] = AddList(lcs);
		m_pList[nGroup]->SetFrame(hFrame,NULL,nExpand);
		m_pList[nGroup]->SetIndent(vIndent);
		m_pList[nGroup]->SetItemSpacing(1);
		m_pList[nGroup]->Show(false);

		{
			CLTGUITextCtrl *pCtrl = NULL;

			CLTGUICtrl_create cs;
			cs.rnBaseRect.m_vMin.Init();
			cs.rnBaseRect.m_vMax = LTVector2n(m_nOptionWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

			pCtrl = CreateTextItem(DATABASE_CATEGORY( PerformanceGroup ).GETRECORDATTRIB(hRec, Name ),cs,true);
			if (pCtrl)
				m_pList[nGroup]->AddControl(pCtrl);
		}
	}

	InitOptionList();

	// display the first group
	SetCurrentType( CMD_CUSTOM );

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;
}

// Adds the columns to the controls
void CScreenPerformanceAdvanced::InitOptionList()
{
	//read in the layout data we'll need
	static LTRect2n dlgRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenDialogRect);
	static const uint32 nDlgItemWidth = dlgRect.GetWidth() - 16;
	static const uint32 kColumn1 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,2);
	static const uint32 kColumn0 = nDlgItemWidth - kColumn1;

	LTVector2n offset(8,8);

	char const* pszListFont = g_pLayoutDB->GetListFont(m_hLayout,0);

	//create all of the group setting controls 
	for (uint32 nGroup = 0; nGroup < CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType); nGroup++)
	{
		CLTGUICycleCtrl_create ccs;
		ccs.rnBaseRect.m_vMin.Init();
		ccs.rnBaseRect.m_vMax = LTVector2n(m_ListRect.GetWidth(),m_nListFontSize);
		ccs.nHeaderWidth = kColumn0;
		ccs.nCommandID = CMD_CUSTOM+(m_nGlobalType * kOffset)+nGroup;
		ccs.pCommandHandler = this;

		uint32 nNumOptions = CPerformanceMgr::Instance().GetNumOptions(m_nGlobalType,nGroup);
		for (uint32 nOpt = 0; nOpt < nNumOptions; nOpt++)
		{
			HRECORD hOptRec = CPerformanceMgr::Instance().GetOptionRecord(m_nGlobalType,nGroup,nOpt);
			if (!hOptRec)
			{
				continue;
			}

			LTVector2n pos(m_ListRect.Left(),m_ListRect.Top());

			CLTGUICtrl_create cs;
			cs.rnBaseRect.m_vMin.Init();
			cs.rnBaseRect.m_vMax = LTVector2n(m_ListRect.GetWidth(),m_nListFontSize);

			cs.nCommandID = CMD_CHANGE_CONTROL;
			cs.szHelpID = DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIB(hOptRec, Help );
			cs.pCommandHandler = this;

			// The initial column (contains the action)
			CLTGUIColumnCtrl *pCtrl=CreateColumnCtrl(cs,false,"",m_nListFontSize);

			// The option column
			pCtrl->AddColumn(LoadString(DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIB(hOptRec, Name )), m_nOptionWidth, true);

			// The column that contains the key which is assigned to the control!
			pCtrl->AddColumn(LoadString("IDS_LOW"), m_nSettingWidth, true);

			pCtrl->SetParam1(nOpt);

			m_pList[nGroup]->AddControl(pCtrl);
		}
	}
}

void CScreenPerformanceAdvanced::UpdateOptionList()
{
	AdjustOptionFrame();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceAdvanced::OnCommand()
//
//	PURPOSE:	handle user input
//
// ----------------------------------------------------------------------- //
uint32 CScreenPerformanceAdvanced::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if( dwCommand >= CMD_CUSTOM )
	{
		SetCurrentType(dwCommand - CMD_CUSTOM);
		return 1;
	}

	switch( dwCommand )
	{
	case CMD_CHANGE_CONTROL:
		{
			// Set the current screen item to ??? while tracking the device.
			int nIndex = m_pList[m_nType]->GetSelectedIndex();
			if( nIndex < kCtrlOffset )
				break;

			CLTGUIColumnCtrl *pCtrl= (CLTGUIColumnCtrl*)m_pList[m_nType]->GetSelectedControl();

			UpdateOption( pCtrl, m_nType, nIndex - kCtrlOffset );
		}
		return 1;
	}

	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceAdvanced::OnFocus()
//
//	PURPOSE:	join/leave screen
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceAdvanced::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		Load();
		UpdateData(false);

		// display the first group
		SetCurrentType( 0 );
	}
	else
	{
		Save();
		UpdateData();
	}

	CBaseScreen::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceAdvanced::Load()
//
//	PURPOSE:	read in the settings and apply them to the controls
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceAdvanced::Load()
{
	for (uint32 nGroup = 0; nGroup < CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType); ++nGroup)
	{
		uint32 nNumOptions = CPerformanceMgr::Instance().GetNumOptions(m_nGlobalType,nGroup);
		for (uint32 nOption = 0; nOption < nNumOptions; ++nOption)
		{
			int32 nLevel = CPerformanceMgr::Instance().GetOptionLevel(m_nGlobalType,nGroup,nOption);
			SetOption( nGroup, nOption, nLevel, false );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPerformanceAdvanced::Save()
//
//	PURPOSE:	get settings from controls, and apply them
//
// ----------------------------------------------------------------------- //
void CScreenPerformanceAdvanced::Save()
{
}

void CScreenPerformanceAdvanced::SetCurrentType(int nType)
{
	if (nType < 0 || nType > (int)CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType))
	{
		nType = CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType);
	}
	for (int n = 0; n < (int)CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType); n++)
	{
		m_pList[n]->Show( (n==nType) );
	}
	m_nType = nType;
	if (nType < (int)CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType))
	{
		AdjustOptionFrame();
		SetSelection(GetIndex(m_pList[nType]));
	}
	else
	{
		SetSelection(0);
	}
}

void CScreenPerformanceAdvanced::AdjustOptionFrame()
{
	if (m_nType >= (int)CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType)) 
	{
		return;
	}
	m_pList[m_nType]->CalculatePositions();

	LTVector2n listpos = m_pList[m_nType]->GetBasePos();
	uint16 i = m_pList[m_nType]->GetNumControls() - 1;
	CLTGUICtrl *pCtrl = m_pList[m_nType]->GetControl(i);
	if (pCtrl)
	{
		LTVector2n pos = pCtrl->GetBasePos();
		LTVector2n sz( m_ListRect.GetWidth(), (pos.y - listpos.y) + pCtrl->GetBaseHeight() + 4);

		m_pList[m_nType]->SetSize(sz);

	}
}

// updates a value
void CScreenPerformanceAdvanced::UpdateOption( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex )
{
	LTASSERT( pCtrl, "Invalid control pointer" );
	if( !pCtrl )
		return;

	HRECORD hOptRec = CPerformanceMgr::Instance().GetOptionRecord(m_nGlobalType, nGroup, nIndex);
	if( !hOptRec )
		return;

	CLTGUITextCtrl*	pColumn = pCtrl->GetColumn( 1 );
	if( !pColumn )
		return;

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hOptRec,"DetailNames");
	const uint32 nLevelCount = g_pLTDatabase->GetNumValues(hAtt);

	const wchar_t* pwszCurrentDetailName = pColumn->GetString();

	int32 nLevel = nLevelCount - 1;
	for(;nLevel>=0;--nLevel)
	{
		const char* pszDetailName = DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIBINDEX(hOptRec, DetailNames, nLevel);
		if( LTStrIEquals(pwszCurrentDetailName, LoadString(pszDetailName)) )
			break;
	}

	if( --nLevel < 0 )
		nLevel = nLevelCount - 1;

	SetOption( nGroup, nIndex, nLevel );
}

void CScreenPerformanceAdvanced::SetOption( uint32 nGroup, uint32 nOption, int32 nLevel, bool bUpdate /*= true*/ )
{
	if( nLevel < 0 )
		return;

	CLTGUIColumnCtrl* pCtrl = (CLTGUIColumnCtrl*)m_pList[nGroup]->GetControl( nOption + kCtrlOffset );
	if( !pCtrl )
		return;

	HRECORD hOptRec = CPerformanceMgr::Instance().GetOptionRecord(m_nGlobalType, nGroup, nOption);
	if( !hOptRec )
		return;

	CLTGUITextCtrl*	pColumn = pCtrl->GetColumn( 1 );
	if( !pColumn )
		return;

	const char* pszDetailName = DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIBINDEX(hOptRec, DetailNames, nLevel);

	pCtrl->SetString(1, LoadString(pszDetailName));

	if( bUpdate )
		CPerformanceMgr::Instance().SetOptionLevel( m_nGlobalType, nGroup, nOption, nLevel );
}

// tries to locate a column control with the given name
CLTGUIColumnCtrl* CScreenPerformanceAdvanced::FindColumnControl( const wchar_t* wszName )
{
	for(uint32 nGroup=0;nGroup<CPerformanceMgr::Instance().GetNumGroups(m_nGlobalType);++nGroup)
	{
		const uint32 nControlCount = m_pList[nGroup]->GetNumControls();
		for(uint32 nControl=kCtrlOffset;nControl<nControlCount;++nControl)
		{
			CLTGUIColumnCtrl* pCtrl = (CLTGUIColumnCtrl*)m_pList[nGroup]->GetControl( nControl );
			if( !pCtrl )
				continue;

			CLTGUITextCtrl* pTextCtrl = pCtrl->GetColumn( 0 );
			if( !pTextCtrl )
				continue;

			if( LTStrIEquals(pTextCtrl->GetString(), wszName) )
				return pCtrl;
		}
	}

	return NULL;
}

