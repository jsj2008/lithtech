// ----------------------------------------------------------------------- //
//
// MODULE  : HUDRadio.cpp
//
// PURPOSE : Implementation of CHUDRadio to display messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDRadio.h"
#include "InterfaceMgr.h"
#include "ClientConnectionMgr.h"
#include "GameClientShell.h"
#include "BroadcastDB.h"
#include "GameModeMgr.h"
#include "iltinput.h"
#include "bindmgr.h"
#include "CommandIDs.h"
#include "CharacterFX.h"

static ILTInput *g_pLTInput;
define_holder(ILTInput, g_pLTInput);

static wchar_t szKeys[9][4] =
{
	L"1",
	L"2",
	L"3",
	L"4",
	L"5",
	L"6",
	L"7",
	L"8",
	L"9"
};

CHUDRadio::CHUDRadio()
{
	m_eLevel		= kHUDRenderDead;
	m_UpdateFlags	= kHUDNone;
	m_bVisible		= false;
	m_hCurrent		= NULL;

	for (int i = 0; i < MAX_RADIO_CHOICES; i++)
	{
		m_pText[i] = NULL;
		m_hSubset[i] = NULL;
	}
	m_pText[MAX_RADIO_CHOICES] = NULL;
	m_nNumChoices = 0;
	m_bBindingsSet = false;
}
	

bool CHUDRadio::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;

}
void CHUDRadio::Term()
{
	m_Dlg.Destroy();
}

void CHUDRadio::Render()
{
	if (!m_bVisible) return;
	m_Dlg.Render();
}

void CHUDRadio::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;

}


void CHUDRadio::Show(bool bShow)
{
	if	(!m_nNumChoices) return;

	if (!g_pProfileMgr->GetCurrentProfile()->m_bAllowBroadcast)
	{
		return;
	}

	//can't send another message while still playing an old one
	CCharacterFX *pFX = g_pGameClientShell->GetLocalCharacterFX();
	if (bShow && pFX && pFX->IsPlayingBroadcast())
		return;

	m_bVisible = bShow;
	if (!m_bVisible && m_vecHistory.size())
	{
		ResetText();
	}
	Update();

	SetBindings(bShow);
}

bool CHUDRadio::Choose(uint8 nChoice)
{

	if (nChoice == COMMAND_ID_CHOOSE_0 && m_vecHistory.size())
	{
		HRECORD hSet = m_vecHistory.back();
		m_vecHistory.pop_back();
		m_hCurrent = NULL;
		if (hSet)
		{
			SetText(hSet);
			if (m_nNumChoices)
			{
				return true;
			}
		}
		m_bVisible = false;
		ResetText();
		return true;
	};

	if (nChoice >= m_nNumChoices)
		return false;


	if (m_hSubset[nChoice])
	{
	
		SetText(m_hSubset[nChoice]);
		if (m_nNumChoices)
		{
			
		}
		else
		{
			m_bVisible = false;
			ResetText();
		}
	}
	else
	{
		m_bVisible = false;
		uint32 nLocalID;
		g_pLTClient->GetLocalClientID(&nLocalID);
		g_pClientConnectionMgr->DoBroadcast(nLocalID,m_sBroadcast[nChoice].c_str());
		if (m_vecHistory.size())
		{
			ResetText();
		}
	}

	return true;
}

void CHUDRadio::ScaleChanged()
{
	LTVector2n pos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(pos);
	m_Dlg.SetBasePos(pos);
}

void CHUDRadio::UpdateLayout()
{

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDRadio");
	}

	CHUDItem::UpdateLayout();

	uint32 nHeaderWidth = 2 * m_sTextFont.m_nHeight;

	ASSERT( m_vIconSize.x == ( uint16 )m_vIconSize.x );
	m_nWidth = ( uint16 )m_vIconSize.x;

	LTVector2n offset = m_vTextOffset;

	uint32 nTextWidth = (m_nWidth - 2 * offset.x) - nHeaderWidth;

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMax = m_vIconSize;
	m_Dlg.Create(m_hIconTexture,cs);
	m_Dlg.Show(true);

	for (int i = 0; i < MAX_RADIO_CHOICES+1; i++)
	{
		if (m_pText[i])
		{
			m_Dlg.SetControlOffset(m_pText[i],offset);
			m_pText[i]->SetFont(m_sTextFont);
		}
		else
		{
			
			
			cs.rnBaseRect.Right() = nTextWidth+nHeaderWidth;
			cs.rnBaseRect.Bottom() = m_sTextFont.m_nHeight;
			m_pText[i] = debug_new(CLTGUIColumnCtrl);
			m_pText[i]->Create(m_sTextFont,cs);
			

			if (i < MAX_RADIO_CHOICES)
			{
				wchar_t wszTmp[4];
				swprintf(wszTmp,L"%d.",i+1);
				m_pText[i]->AddColumn(wszTmp,nHeaderWidth,true);
				m_pText[i]->AddColumn(L"X",nTextWidth,true);
			}
			else
			{
				m_pText[i]->AddColumn(L"0.",nHeaderWidth,true);
				m_pText[i]->AddColumn(LoadString("IDS_BACK"),nTextWidth,true);
			}

			m_Dlg.AddControl(m_pText[i],offset);
		}

		m_pText[i]->SetColor(m_cTextColor);

		offset.y += m_pText[i]->GetBaseHeight() + 4;

	}

	m_Dlg.SetBasePos(m_vBasePos);
	m_Dlg.SetSize( LTVector2n(m_nWidth,(offset.y+m_vTextOffset.y)) );

}

void CHUDRadio::ResetText()
{
	m_vecHistory.clear();
	m_hCurrent = NULL;
	m_nNumChoices = 0;

	const char* szSet = GameModeMgr::Instance( ).m_grsBroadcastSet;
	if (LTStrEmpty(szSet))
	{
		return;
	}

	HRECORD hSet = DATABASE_CATEGORY( BroadcastSet ).GetRecordByName(szSet);
	if (!hSet) 
	{
		return;
	}
	SetText(hSet);
}

void CHUDRadio::SetText(HRECORD hSet)
{
	m_nNumChoices = 0;
	for (int i = 0; i < MAX_RADIO_CHOICES; i++)
	{
		HRECORD hBroadcast = DATABASE_CATEGORY( BroadcastSet ).GETRECORDATTRIBINDEX( hSet, Broadcast, i);
		if (hBroadcast)
		{
			const char* szLabel = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hBroadcast, Label );
			if (m_pText[m_nNumChoices])
			{
				m_pText[m_nNumChoices]->GetColumn(1)->SetString( LoadString(szLabel) );
				m_sBroadcast[m_nNumChoices] = g_pLTDatabase->GetRecordName(hBroadcast);
				m_hSubset[m_nNumChoices] = NULL;
				m_pText[m_nNumChoices]->Show(true);
				m_nNumChoices++;
			}
		}
		else
		{
			//no broadcast, so see if we have a subset for this index
			HRECORD hSubSet = DATABASE_CATEGORY( BroadcastSet ).GETRECORDATTRIBINDEX( hSet, BroadcastSubSet, i);
			if (hSubSet)
			{
				const char* szLabel = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hSubSet, Label );
				if (m_pText[m_nNumChoices])
				{
					m_pText[m_nNumChoices]->GetColumn(1)->SetString( LoadString(szLabel) );
					m_sBroadcast[m_nNumChoices] = "";
					m_hSubset[m_nNumChoices] = hSubSet;
					m_pText[m_nNumChoices]->Show(true);
					m_nNumChoices++;
				}
			}
		}
	}

	for (int i = m_nNumChoices; i < MAX_RADIO_CHOICES; i++)
	{
		m_pText[i]->GetColumn(1)->SetString( L"" );
		m_sBroadcast[i] = "";
		m_hSubset[i] = NULL;
		m_pText[i]->Show(false);
	}

	if (m_nNumChoices > 0)
	{
		if (m_hCurrent)
		{
			m_vecHistory.push_back(m_hCurrent);
		}
		m_hCurrent = hSet;

		uint8 nLast = m_nNumChoices-1;
		if (m_vecHistory.size())
		{
			LTVector2n offset = m_pText[nLast]->GetBasePos() - m_Dlg.GetBasePos();
			offset.y += m_pText[nLast]->GetBaseHeight() + 4;
			m_Dlg.SetControlOffset(m_pText[MAX_RADIO_CHOICES],offset);
			m_pText[MAX_RADIO_CHOICES]->Show(true);
			nLast += 1;
		}
		else
		{
			m_pText[MAX_RADIO_CHOICES]->Show(false);
		}
		uint32 nBottom = m_pText[nLast]->GetBasePos().y + m_pText[nLast]->GetBaseHeight();
		uint32 nTop = m_pText[0]->GetBasePos().y;

		uint32 nHeight = (nBottom-nTop) + (2 * m_vTextOffset.y);

		m_Dlg.SetSize( LTVector2n(m_nWidth,nHeight) );

	}
	

}
void CHUDRadio::SetBindings(bool bSet)
{
	// Look up the keyboard device
	uint32 nDeviceIndex;
	if (g_pLTInput->FindFirstDeviceByCategory(ILTInput::eDC_Keyboard, &nDeviceIndex) != LT_OK)
	{
		LTERROR("error finding keyboard device");
		return;
	} 

	m_bBindingsSet = bSet;


	if (bSet)
	{
		//save off the commands bound to the hotkeys
		for (uint8 i = 0; i < MAX_RADIO_CHOICES; ++i)
		{
			// Look up the object on the device
			uint32 nObjectIndex;
			if (g_pLTInput->FindDeviceObjectByName(nDeviceIndex, szKeys[i], &nObjectIndex) != LT_OK)
				continue;

			CBindMgr::SBinding sCurrentBinding;
			if (CBindMgr::GetSingleton().GetDeviceBinding(nDeviceIndex,nObjectIndex,0,&sCurrentBinding))
			{
				m_nSavedCommands[i] = sCurrentBinding.m_nCommand;
			}
			else
			{
				m_nSavedCommands[i] = COMMAND_ID_UNASSIGNED;
			}

			// Set up the binding for it
			CBindMgr::SBinding sNewBinding;
			sNewBinding.m_nCommand = (COMMAND_ID_CHOOSE_1 + i);
			sNewBinding.m_nDevice = nDeviceIndex;
			sNewBinding.m_nObject = nObjectIndex;
			CBindMgr::GetSingleton().SetBinding(sNewBinding);

		}

		//save off the binding for the back control
		uint32 nObjectIndex;
		if (g_pLTInput->FindDeviceObjectByName(nDeviceIndex, L"0", &nObjectIndex) == LT_OK)
		{
			CBindMgr::SBinding sCurrentBinding;
			if (CBindMgr::GetSingleton().GetDeviceBinding(nDeviceIndex,nObjectIndex,0,&sCurrentBinding))
			{
				m_nSavedCommands[MAX_RADIO_CHOICES] = sCurrentBinding.m_nCommand;
			}
			else
			{
				m_nSavedCommands[MAX_RADIO_CHOICES] = COMMAND_ID_UNASSIGNED;
			}

			// Set up the binding for it
			CBindMgr::SBinding sNewBinding;
			sNewBinding.m_nCommand = (COMMAND_ID_CHOOSE_0);
			sNewBinding.m_nDevice = nDeviceIndex;
			sNewBinding.m_nObject = nObjectIndex;
			CBindMgr::GetSingleton().SetBinding(sNewBinding);
		}

	}
	else
	{
		//restore the commands bound to the hotkeys
		for (uint8 i = 0; i < MAX_RADIO_CHOICES; ++i)
		{
			// Look up the object on the device
			uint32 nObjectIndex;
			if (g_pLTInput->FindDeviceObjectByName(nDeviceIndex, szKeys[i], &nObjectIndex) != LT_OK)
				continue;

			if (m_nSavedCommands[i] == COMMAND_ID_UNASSIGNED)
			{
				continue;
			}

			// Set up the binding for it
			CBindMgr::SBinding sNewBinding;
			sNewBinding.m_nCommand = m_nSavedCommands[i];
			sNewBinding.m_nDevice = nDeviceIndex;
			sNewBinding.m_nObject = nObjectIndex;
			CBindMgr::GetSingleton().SetBinding(sNewBinding);

		}

		//restore the 0 key
		uint32 nObjectIndex;
		if (g_pLTInput->FindDeviceObjectByName(nDeviceIndex, L"0", &nObjectIndex) == LT_OK &&
			(m_nSavedCommands[MAX_RADIO_CHOICES] != COMMAND_ID_UNASSIGNED))
		{
			// Set up the binding for it
			CBindMgr::SBinding sNewBinding;
			sNewBinding.m_nCommand = m_nSavedCommands[MAX_RADIO_CHOICES];
			sNewBinding.m_nDevice = nDeviceIndex;
			sNewBinding.m_nObject = nObjectIndex;
			CBindMgr::GetSingleton().SetBinding(sNewBinding);

		}

	}

}


