// ----------------------------------------------------------------------- //
//
// MODULE  : Credits.cpp
//
// PURPOSE : Implementation of class to manage in-game credits
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "Credits.h"
#include "CreditsDB.h"
#include "LayoutDB.h"
#include "sys/win/mpstrconv.h"
#include "SoundDB.h"


static float GetFlickerDelay()
{
	return 0.1f + 0.4f * (GetSinCycle(9.0f) + GetSinCycle(13.0f));
}
static float GetFlickerDuration()
{
	return 0.1f + 0.2f * (GetSinCycle(5.0f) * GetSinCycle(11.0f));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Credits::Credits
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CreditOrder::CreditOrder( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreditOrder::~CreditOrder
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CreditOrder::~CreditOrder( )
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreditOrder::Init()
//
//	PURPOSE:	Initialize credit system
//
// ----------------------------------------------------------------------- //

void CreditOrder::Init()
{
	m_bPlaying = false;
	m_hOrder = NULL;
	m_nIndex = 0;
	m_Timer.SetEngineTimer(GameTimeTimer::Instance());
	m_FlickerTimer.SetEngineTimer(GameTimeTimer::Instance());
}

void CreditOrder::Term()
{
	ClearStrings();
}

void CreditOrder::Update()
{
	if (!m_bPlaying || !m_Timer.IsStarted())
		return;

	float fFade = 0.0f;
	switch(m_eState) 
	{
	case eState_FadeIn:
		if (m_Timer.IsTimedOut())
		{
			fFade = 1.0f;
			m_eState = eState_Hold;
			m_Timer.Start(DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, HoldTime));
		}
		else
		{
			fFade = (float)(m_Timer.GetElapseTime() / m_Timer.GetDuration());
		}
		break;
	case eState_Hold:
		fFade = 1.0f;
		if (m_Timer.IsTimedOut())
		{
			m_eState = eState_FadeOut;
			m_Timer.Start(DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, FadeOutTime));
		}
		break;
	case eState_FadeOut:
		if (m_Timer.IsTimedOut())
		{
			fFade = 1.0f;
			m_eState = eState_Delay;
			m_Timer.Start(DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, DelayTime));
		}
		else
		{
			fFade = (float)(m_Timer.GetTimeLeft() / m_Timer.GetDuration());
		}
		break;
	case eState_Delay:
		if (m_Timer.IsTimedOut())
		{
			m_nIndex++;
			if (!StartEntry())
			{
				Stop();
			}
		}
		fFade = 0.0f;
		break;
	}

	m_fFade = LTCLAMP(fFade,0.0f,1.0f);
	float fMax = LTSqrt(fFade);
	float fMin = LTSqr(fFade) * 0.75f;

	if (m_FlickerTimer.IsTimedOut())
	{
		m_bFlicker = !m_bFlicker;
		if (m_bFlicker)
		{
			if (m_bDoFlicker && m_hFlickerSound)
			{
				float fVol = fMax - fMin;
				uint8 nVol = 25 + uint8( 75.0f * fVol);
				if (nVol > 25)
				{
					//DebugCPrint(0,"credit vol: %d",nVol);
					g_pClientSoundMgr->PlayDBSoundLocal( m_hFlickerSound, SOUNDPRIORITY_INVALID, 0, nVol );
				}
			}
			m_FlickerTimer.Start(GetFlickerDuration());
		}
		else
		{
			float fDelay = GetFlickerDelay();
			if (m_eState == eState_Hold )
			{
				fDelay *= 2.0f;
			}
			m_FlickerTimer.Start(fDelay);
		}

	}


	//we may have been stopped in mid-update, so check again
	if (m_bPlaying)
	{
		for (uint32 n = 0; n < m_nLines; ++n)
		{
			CLTGUIString* pGUIString = m_Lines[n];

			if (m_bFlicker && m_bDoFlicker && (!m_bDoDistort || m_fFade >= 0.8f ) ) 
			{
				float fAlpha = GetRandom(fMin,fMax);
				pGUIString->SetColor( FadeARGB(m_cTextColors[n],fAlpha) );
			}
			else
			{
				pGUIString->SetColor( FadeARGB(m_cTextColors[n],fMax) );
			}
		}
	}
	
}


void CreditOrder::Render()
{
	if (!m_bPlaying)
		return;


	if (m_eState != eState_Delay)
	{
		for (uint32 n = 0; n < m_nLines; ++n)
		{
			CLTGUIString* pGUIString = m_Lines[n];
			if (m_bFlicker && m_bDoDistort && m_fFade < 0.8f)
			{
				float fAlpha = GetRandom((m_fFade + 1.0f)/ 2.0f ,1.0f);
				pGUIString->RenderTransition(fAlpha);
			}
			else
			{
				pGUIString->Render();
			}
			
		}
	}
}

void CreditOrder::Start(HRECORD hOrder)
{
	if (IsPlaying())
		Stop();

	m_hOrder = hOrder;

	m_nIndex = 0;

	if (!m_hOrder)
		return;

	m_bPlaying = StartEntry();

}

void CreditOrder::Start(const char* szOrder)
{
	HRECORD hRec = DATABASE_CATEGORY( CreditOrder ).GetRecordByName( szOrder );

	Start(hRec);

}


bool CreditOrder::StartEntry()
{
	if (!m_hOrder)
		return false;

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(m_hOrder,"Entries");
	if (m_nIndex >= g_pLTDatabase->GetNumValues(hAtt))
		return false;

	m_hEntry = g_pLTDatabase->GetRecordLink(hAtt,m_nIndex,NULL);
	if (!m_hEntry) 
	{
		return false;
	}

	LTVector2 vBasePos = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, BasePos);
	vBasePos.x *= g_pInterfaceResMgr->GetXRatio();
	vBasePos.y *= g_pInterfaceResMgr->GetYRatio();


	eTextAlign eTextAlignment = kLeft;
	const char* pszAlign = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, TextAlignment);
	if (LTStrIEquals(pszAlign,"Right"))
		eTextAlignment = kRight;
	else if (LTStrIEquals(pszAlign,"Center"))
		eTextAlignment = kCenter;

	const char* pszStyle = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, FadeStyle);
	if (LTStrIEquals(pszStyle,"Flicker"))
	{
		m_bDoFlicker = true;
		m_bDoDistort = false;
	}
	else if (LTStrIEquals(pszStyle,"DistortFlicker"))
	{
		m_bDoFlicker = true;
		m_bDoDistort = true;
	}
	else
	{
		m_bDoFlicker = false;
		m_bDoDistort = false;
	}

	m_hFlickerSound = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, FlickerSound );

	HATTRIBUTE hLines = g_pLTDatabase->GetAttribute(m_hEntry,"Lines");
	m_nLines = g_pLTDatabase->GetNumValues(hLines);
	if (!m_nLines)
		return false;
	m_Lines.reserve(m_nLines);
	m_cTextColors.reserve(m_nLines);

	HCATEGORY hLineLayoutCat = DATABASE_CATEGORY( CreditLineLayout ).GetCategory( );

	for (uint32 n = 0; n < m_nLines; ++n)
	{
		HATTRIBUTE hLineLayoutAttrib = DATABASE_CATEGORY( CreditEntry ).GetStructAttribute( hLines, n, "LineLayout" );
		HRECORD hLineLayout = DATABASE_CATEGORY( CreditEntry ).GetRecordLinkToLocalizedDB( hLineLayoutAttrib,
			0, hLineLayoutCat );
		if( !hLineLayout )
		{
			LTERROR_PARAM1( "Invalid LineLayout %s", g_pLTDatabase->GetRecordName( m_hEntry ));
			continue;
		}

		HRECORD hFont = DATABASE_CATEGORY( CreditLineLayout ).GETRECORDATTRIB( hLineLayout, Font );
		HATTRIBUTE hFontAtt = g_pLTDatabase->GetAttribute(hFont,"Face");
		const char* pszFont = g_pLTDatabase->GetString(hFontAtt,0,"");
		if (!pszFont || !pszFont[0])
		{
			pszFont = g_pLayoutDB->GetHUDFont();
		}
		uint32 nFontSize = DATABASE_CATEGORY( CreditLineLayout ).GETRECORDATTRIB( hLineLayout, TextSize );
		CFontInfo sTextFont(pszFont,nFontSize);

		uint32 nBaseWidth = DATABASE_CATEGORY( CreditLineLayout ).GETRECORDATTRIB(hLineLayout, BaseWidth);
		uint32 nWidth = (uint32)((float)nBaseWidth * g_pInterfaceResMgr->GetXRatio());


		m_cTextColors[n] = DATABASE_CATEGORY( CreditEntry ).GETSTRUCTATTRIB(Lines, hLines, n, TextColor);

		CLTGUIString* pGUIString;
		if (n >= m_Lines.size())
		{
			pGUIString = debug_new(CLTGUIString);
			m_Lines.push_back(pGUIString);
		}
		else
		{
			pGUIString = m_Lines[n];
		}

		const char* pszText = DATABASE_CATEGORY( CreditEntry ).GETSTRUCTATTRIB(Lines, hLines, n, Line);
		pGUIString->SetText( CreateHelpString(pszText) );
		pGUIString->SetFont(sTextFont);
		pGUIString->SetPos(vBasePos);
		pGUIString->WordWrap(nWidth);
		pGUIString->SetAlignment(eTextAlignment);
		pGUIString->SetColor(m_cTextColors[n]);
		pGUIString->SetDropShadow(1);

		vBasePos.y += nFontSize;

	}

	HRECORD hSound = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, Sound );

	if (hSound)
	{
		g_pClientSoundMgr->PlayDBSoundLocal( hSound, SOUNDPRIORITY_INVALID );
	}
	

	for (uint32 n = m_nLines; n < m_Lines.size(); ++n)
	{
		CLTGUIString* pGUIString = m_Lines[n];
		pGUIString->FlushTexture();
	}


	float fDuration = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, FadeInTime);
	if (fDuration > 0.0f)
	{
		m_eState = eState_FadeIn;
	}
	else
	{
		fDuration = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, HoldTime);
		if (fDuration > 0.0f)
		{
			m_eState = eState_Hold;
		}
		else
		{
			fDuration = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, FadeOutTime);
			if (fDuration > 0.0f)
			{
				m_eState = eState_FadeOut;
			}
			else
			{
				fDuration = DATABASE_CATEGORY( CreditEntry ).GETRECORDATTRIB(m_hEntry, DelayTime);
				if (fDuration > 0.0f)
				{
					m_eState = eState_Delay;
				}
				else
				{
					return false;
				}
			}

		}
	}

	m_bFlicker = false;
	m_Timer.Start(fDuration);
	m_FlickerTimer.Start(GetFlickerDelay() * 0.5f);

	return true;	

}

void CreditOrder::Stop()
{
	m_bPlaying = false;
	m_hOrder = NULL;
	m_nIndex = 0;
	m_hEntry = false;
	m_Timer.Stop();
	ClearStrings();
}



// Remove all strings
void CreditOrder::ClearStrings()
{
	LTGUIStringArray::iterator iter = m_Lines.begin();
	while ( iter != m_Lines.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_Lines.clear();
	m_nLines = 0;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Credits::Credits
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Credits::Credits( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Credits::~Credits
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Credits::~Credits( )
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Credits::Init()
//
//	PURPOSE:	Initialize credit system
//
// ----------------------------------------------------------------------- //

void Credits::Init()
{
	DATABASE_CATEGORY( CreditEntry ).Init( DB_Default_File );
	DATABASE_CATEGORY( CreditOrder ).Init( DB_Default_File );
	DATABASE_CATEGORY( CreditLineLayout ).Init( DB_Default_Localized_File );


}

void Credits::Term()
{
	CreditOrderList::iterator iter = m_ActiveOrders.begin();
	while (iter != m_ActiveOrders.end())
	{ 

		debug_delete(*iter);
		iter++;
	}
	m_ActiveOrders.clear();

	iter = m_ReserveOrders.begin();
	while (iter != m_ReserveOrders.end())
	{ 
		debug_delete(*iter);
		iter++;
	}
	m_ReserveOrders.clear();
}

void Credits::Update()
{
	if (!IsPlaying())
	{
		return;
	}

	CreditOrderList::iterator iter = m_ActiveOrders.begin();
	while (iter != m_ActiveOrders.end())
	{ 
		CreditOrder* pOrder = (*iter);
		pOrder->Update();
		if (!pOrder->IsPlaying())
		{
			pOrder->Term();
			m_ReserveOrders.push_back(pOrder);
//			DebugCPrint(0,"%s - copying CreditOrder to reserve",__FUNCTION__);

			iter = m_ActiveOrders.erase(iter);
		}
		else
		{
			iter++;
		}
		
	}

}


void Credits::Render()
{
	CreditOrderList::iterator iter = m_ActiveOrders.begin();
	while (iter != m_ActiveOrders.end())
	{ 
		CreditOrder* pOrder = (*iter);
		if (pOrder->IsPlaying())
		{
			pOrder->Render();
		}
		iter++;
	}
}

void Credits::Start(HRECORD hOrder)
{

	CreditOrder* pOrder = NULL;
	if (m_ReserveOrders.empty())
	{
		pOrder = debug_new(CreditOrder);
		//DebugCPrint(0,"%s - creating new CreditOrder",__FUNCTION__);
		
	}
	else
	{
		pOrder = m_ReserveOrders[0];
		m_ReserveOrders.erase( m_ReserveOrders.begin() );
		//DebugCPrint(0,"%s - retrieving reserve CreditOrder",__FUNCTION__);
	}

	pOrder->Init();
	pOrder->Start(hOrder);
	m_ActiveOrders.push_back(pOrder);

}

void Credits::Start(const char* szOrder)
{
	HRECORD hRec = DATABASE_CATEGORY( CreditOrder ).GetRecordByName( szOrder );

	Start(hRec);

}

void Credits::Stop(HRECORD hOrder)
{
	CreditOrderList::iterator iter = m_ActiveOrders.begin();
	while (iter != m_ActiveOrders.end())
	{ 
		CreditOrder* pOrder = (*iter);
		if (pOrder->GetRecord() == hOrder)
		{
			pOrder->Stop();
		}
	}

}
void Credits::StopAll()
{
	CreditOrderList::iterator iter = m_ActiveOrders.begin();
	while (iter != m_ActiveOrders.end())
	{ 
		CreditOrder* pOrder = (*iter);
		pOrder->Stop();
		iter++;
	}
}
