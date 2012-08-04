#include "MessageBox.h"
#include "clientheaders.h"
#include "ClientRes.h"
#include "font08.h"

#define TEXTRECT_LEFT		28
#define TEXTRECT_RIGHT		234
#define TEXTRECT_TOP		6
#define TEXTRECT_BOTTOM		58

#define TEXTRECT_WIDTH		(TEXTRECT_RIGHT - TEXTRECT_LEFT)
#define TEXTRECT_HEIGHT		(TEXTRECT_BOTTOM - TEXTRECT_TOP)

CMessageBox::~CMessageBox()
{
	if (m_pClientDE && m_hMessageBox) 
	{
		m_pClientDE->DeleteSurface (m_hMessageBox);
	}
}

LTBOOL CMessageBox::Init (ILTClient* pClientDE, int nStringID, LTBOOL bYesNo, int nAlignment, LTBOOL bCrop)
{
	if (!pClientDE || !nStringID) return LTFALSE;

	CFont08 font;
	font.Init (pClientDE, "interface/font08n.pcx");

	m_bYesNo = bYesNo;

	HSURFACE hText = CTextHelper::CreateWrappedStringSurface (pClientDE, TEXTRECT_RIGHT - TEXTRECT_LEFT, &font, nStringID, nAlignment, bCrop);
	if (!hText) return LTFALSE;

	HSURFACE hPressAnyKey = CTextHelper::CreateSurfaceFromString (pClientDE, &font, bYesNo ? IDS_YESNO : IDS_PRESSKEYTOCONTINUE);
	if (!hPressAnyKey)
	{
		pClientDE->DeleteSurface (hText);
		return LTFALSE;
	}

	uint32 nPromptWidth, nPromptHeight;
	pClientDE->GetSurfaceDims (hPressAnyKey, &nPromptWidth, &nPromptHeight);

	uint32 nTextWidth, nTextHeight;
	pClientDE->GetSurfaceDims (hText, &nTextWidth, &nTextHeight);

	m_hMessageBox = pClientDE->CreateSurfaceFromBitmap ("interface/blanktag.pcx");
	if (!m_hMessageBox)
	{
		pClientDE->DeleteSurface (hText);
		pClientDE->DeleteSurface (hPressAnyKey);
		return LTFALSE;
	}

	pClientDE->GetSurfaceDims (m_hMessageBox, &m_szMessageBox.cx, &m_szMessageBox.cy);

	pClientDE->DrawSurfaceToSurfaceTransparent (m_hMessageBox, hText, LTNULL, TEXTRECT_LEFT + ((TEXTRECT_WIDTH - (int)nTextWidth) / 2), TEXTRECT_TOP + ((TEXTRECT_HEIGHT - (int)nTextHeight - (int)nPromptHeight) / 2), LTNULL);
	pClientDE->DrawSurfaceToSurfaceTransparent (m_hMessageBox, hPressAnyKey, LTNULL, TEXTRECT_LEFT + ((TEXTRECT_WIDTH - (int)nPromptWidth) / 2), TEXTRECT_BOTTOM - (int)nPromptHeight, LTNULL);

	pClientDE->DeleteSurface (hText);
	pClientDE->DeleteSurface (hPressAnyKey);

	m_pClientDE = pClientDE;

	return LTTRUE;
}

void CMessageBox::Draw()
{
	if (!m_pClientDE || !m_hMessageBox) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();

	uint32 nScreenWidth, nScreenHeight;
	m_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hMessageBox, LTNULL, ((int)nScreenWidth - (int)m_szMessageBox.cx) / 2, ((int)nScreenHeight - (int)m_szMessageBox.cy) / 2, LTNULL);
}
