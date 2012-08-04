// FolderTexture.cpp: implementation of the CFolderTexture class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderTexture.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	int kHeaderWidth = 300;
	int kSliderWidth = 200;
	int kSpacerWidth = 25;
	int kTotalWidth  = kHeaderWidth + kSpacerWidth;

	int nInitVal[6];

	int kDefaultValues[6][3] =
	{
		{1,0,0},
		{1,1,0},
		{1,1,0},
		{1,1,0},
		{2,1,0},
		{2,1,0},
	};
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderTexture::CFolderTexture()
{
	m_nOverall = 0;

	for (int i = 0; i < 6; i++)
	{
		m_nSliderVal[i] = 0;
		m_pSlider[i] = LTNULL;
	}

}

CFolderTexture::~CFolderTexture()
{
	Term();
}

// Build the folder
LTBOOL CFolderTexture::Build()
{

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_TEXTURE,"ColumnWidth"))
	{
		kTotalWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_TEXTURE,"ColumnWidth");
		kHeaderWidth = kTotalWidth - kSpacerWidth;
	}
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_TEXTURE,"SliderWidth"))
		kSliderWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_TEXTURE,"SliderWidth");

	CreateTitle(IDS_TITLE_TEXTURE);

	m_pOverallCtrl = AddCycleItem(IDS_DETAILLEVEL,IDS_HELP_DETAILLEVEL,kHeaderWidth,kSpacerWidth,&m_nOverall);
	m_pOverallCtrl->AddString(IDS_LOW);
	m_pOverallCtrl->AddString(IDS_MEDIUM);
	m_pOverallCtrl->AddString(IDS_HIGH);
	m_pOverallCtrl->AddString(IDS_CUSTOMIZED);

	m_pSlider[0]=AddSlider(IDS_TEXTURE_WORLD, IDS_HELP_TEXTURE_WORLD, kTotalWidth, kSliderWidth, &m_nSliderVal[0]);
	m_pSlider[1]=AddSlider(IDS_TEXTURE_WEAPONS, IDS_HELP_TEXTURE_WEAPONS, kTotalWidth, kSliderWidth, &m_nSliderVal[1]);
	m_pSlider[2]=AddSlider(IDS_TEXTURE_CHARS, IDS_HELP_TEXTURE_CHARS, kTotalWidth, kSliderWidth, &m_nSliderVal[2]);
	m_pSlider[3]=AddSlider(IDS_TEXTURE_PROPS, IDS_HELP_TEXTURE_PROPS, kTotalWidth, kSliderWidth, &m_nSliderVal[3]);
	m_pSlider[4]=AddSlider(IDS_TEXTURE_SFX, IDS_HELP_TEXTURE_SFX, kTotalWidth, kSliderWidth, &m_nSliderVal[4]);
	m_pSlider[5]=AddSlider(IDS_TEXTURE_SKY, IDS_HELP_TEXTURE_SKY, kTotalWidth, kSliderWidth, &m_nSliderVal[5]);

	for (int i = 0; i < 6; i++)
	{
		m_pSlider[i]->SetSliderRange(0,5);
		m_pSlider[i]->SetSliderIncrement(1);
	}
 	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	return LTTRUE;

}

void CFolderTexture::Term()
{
	// Make sure to call the base class
	CBaseFolder::Term();
}

uint32 CFolderTexture::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};


void CFolderTexture::OnFocus(LTBOOL bFocus)
{

	
	if (bFocus)
	{
		LTBOOL bPotential[3] = {LTTRUE,LTTRUE,LTTRUE};
		for (int i = 0; i < 6; i++)
		{
			char szKey[16];
			sprintf(szKey,"GroupOffset%d",i+1);
			int offset = GetConsoleInt(szKey,0);
			m_nSliderVal[i] = 3 - offset;
			nInitVal[i] = m_nSliderVal[i];
			for (int j = 0; j < 3; j++)
			{
				if (offset != kDefaultValues[i][j])
					bPotential[j] = LTFALSE;
			}
		}

		m_nOverall = 0;
		while (m_nOverall < 3 && !bPotential[m_nOverall])
			m_nOverall++;

		UpdateData(LTFALSE);
	}
	else
	{
		LTBOOL bRebind = LTFALSE;
        UpdateData(LTTRUE);
		for (int i = 0; i < 6; i++)
		{
			char szKey[16];
			sprintf(szKey,"GroupOffset%d",i+1);
			int offset = 3 - m_nSliderVal[i];
			if (nInitVal[i] != m_nSliderVal[i])
			{
				bRebind = LTTRUE;
				WriteConsoleInt(szKey,offset);
			}
		}
		if (bRebind)
		{
            g_pLTClient->Start3D();
            g_pLTClient->StartOptimized2D();

			g_pInterfaceResMgr->DrawMessage(GetSmallFont(),IDS_REBINDING_TEXTURES);

            g_pLTClient->EndOptimized2D();
            g_pLTClient->End3D();
            g_pLTClient->FlipScreen(0);

			g_pLTClient->RunConsoleString("RebindTextures");
		}

	}
	CBaseFolder::OnFocus(bFocus);
}


LTBOOL CFolderTexture::OnLeft()
{
	if (GetSelectedControl() == m_pOverallCtrl)
	{
		int detailLevel = m_pOverallCtrl->GetSelIndex();
		--detailLevel;
		if (detailLevel < 0)
			detailLevel = 2;
		m_pOverallCtrl->SetSelIndex(detailLevel);
		SetSliders(detailLevel);
        return LTTRUE;
	}
	else if (IsSlider(GetSelectedControl()))
	{
		LTBOOL bSlid = GetSelectedControl()->OnLeft();
		if (bSlid)
		{
			GetSelectedControl()->UpdateData();
			CheckOverall();
		}
		return bSlid;
	}

	return CBaseFolder::OnLeft();
}

LTBOOL CFolderTexture::OnRight()
{
	if (GetSelectedControl() == m_pOverallCtrl)
	{
		int detailLevel = m_pOverallCtrl->GetSelIndex();
		++detailLevel;
		if (detailLevel > 2)
			detailLevel = 0;
		m_pOverallCtrl->SetSelIndex(detailLevel);
		SetSliders(detailLevel);
		return LTTRUE;
	}
	else if (IsSlider(GetSelectedControl()))
	{
		LTBOOL bSlid = GetSelectedControl()->OnRight();
		if (bSlid)
		{
			GetSelectedControl()->UpdateData();
			CheckOverall();
		}
		return bSlid;
	}
	return CBaseFolder::OnRight();
}


void CFolderTexture::SetSliders(int nLevel)
{
	if (nLevel < 0 || nLevel >= 3) return;
	for (int i = 0; i < 6; i++)
	{
		m_nSliderVal[i] = 3-kDefaultValues[i][nLevel];
	}
	m_nOverall = nLevel;
	UpdateData(LTFALSE);
}

void CFolderTexture::CheckOverall()
{
	LTBOOL bPotential[3] = {LTTRUE,LTTRUE,LTTRUE};
	for (int i = 0; i < 6; i++)
	{
		int offset = 3 - m_nSliderVal[i];
		for (int j = 0; j < 3; j++)
		{
			if (offset != kDefaultValues[i][j])
				bPotential[j] = LTFALSE;
		}
	}
	m_nOverall = 0;
	while (m_nOverall < 3 && !bPotential[m_nOverall])
		m_nOverall++;

	UpdateData(LTFALSE);
}

LTBOOL CFolderTexture::IsSlider(CLTGUICtrl* pCtrl)
{
	for (int i = 0; i < 6; i++)
	{
		if ((CLTGUICtrl*)m_pSlider[i] == pCtrl) return LTTRUE;
	}
	return LTFALSE;
}


LTBOOL CFolderTexture::OnLButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pOverallCtrl)
		{
			return OnRight();
		}
		else if (IsSlider(pCtrl))
		{
			LTBOOL bSlid = pCtrl->OnLButtonUp(x, y);
			if (bSlid)
			{
				pCtrl->UpdateData();
				CheckOverall();
			}
			return bSlid;
		}
	}
	return CBaseFolder::OnLButtonUp(x, y);
}

LTBOOL CFolderTexture::OnRButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pOverallCtrl)
		{
			return OnLeft();
		}
	}
	return CBaseFolder::OnRButtonUp(x, y);
}