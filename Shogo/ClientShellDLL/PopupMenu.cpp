#include "PopupMenu.h"
#include "RiotClientShell.h"
#include "iltclient.h"
#include "VKDefs.h"
#include "Slider.h"
#include "TextHelper.h"
#include "RiotSoundTypes.h"
#include "ClientRes.h"

CPopupMenu::CPopupMenu (int nLeft, int nTop, int nWidth, int nHeight)
{
	m_nID = 0;

	m_pClientDE = LTNULL;
	m_pRiotMenu = LTNULL;

	m_pParentPopup = LTNULL; 

	m_nLeft = nLeft;
	m_nTop = nTop;
	m_nRight = m_nLeft + nWidth - 1;
	m_nBottom = m_nTop + nHeight - 1;

	m_nLeftMargin = 2;
	m_nTopMargin = 2;

	m_itemArray.SetMemCopyable (1);
	m_itemArray.SetGrowBy (48);
	m_nItems = 0;
	m_nFirstItem = 0;
	m_nLastItem = 0;

	m_nSelection = 0;

	m_hTitle = LTNULL;
	m_hBackground = LTNULL;
	m_hCursor = LTNULL;
	m_cxCursor = 0;

	m_bConfiguring = LTFALSE;
	m_bEditing = LTFALSE;
	m_strEdit = LTNULL;
}

CPopupMenu::~CPopupMenu()
{
	if (m_pClientDE)
	{
		if (m_hTitle) m_pClientDE->DeleteSurface (m_hTitle);
		if (m_hBackground) m_pClientDE->DeleteSurface (m_hBackground);
		if (m_hCursor) m_pClientDE->DeleteSurface (m_hCursor);
		for (uint32 i = 0; i < m_nItems; i++)
		{
			if (m_itemArray[i].hSurface)
			{
				m_pClientDE->DeleteSurface (m_itemArray[i].hSurface);
			}
			if (m_itemArray[i].hSelected)
			{
				m_pClientDE->DeleteSurface (m_itemArray[i].hSelected);
			}
		}
	}

	for (uint32 i = 0; i < m_nItems; i++)
	{
		if (m_itemArray[i].pData && (m_itemArray[i].nType == Edit || m_itemArray[i].nType == Multiple || m_itemArray[i].nType == KeyConfig))
		{
			delete [] (char*) m_itemArray[i].pData;
		}
		if (m_itemArray[i].pData && m_itemArray[i].nType == Slider)
		{
			delete (CSlider*) m_itemArray[i].pData;
		}
		if (m_itemArray[i].pData && m_itemArray[i].nType == PopupMenu)
		{
			delete (CPopupMenu*) m_itemArray[i].pData;
		}
	}
	m_itemArray.Flush();
	m_nItems = 0;

	if (m_strEdit) delete [] m_strEdit;
}

LTBOOL CPopupMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu)
{
	if (!pClientDE) return LTFALSE;

	m_pClientDE = pClientDE;
	m_pRiotMenu = pRiotMenu;

	// load the menu border pieces
	
	HSURFACE hBorderTL = LTNULL;
	HSURFACE hBorderTR = LTNULL;
	HSURFACE hBorderBR = LTNULL;
	HSURFACE hBorderBL = LTNULL;
	HSURFACE hBorderL = LTNULL;
	HSURFACE hBorderT = LTNULL;
	HSURFACE hBorderR = LTNULL;
	HSURFACE hBorderB = LTNULL;
	HLTFONT hFont = LTNULL;
	HSTRING hString = LTNULL;

	if (LTFALSE)
	{
fail:

		if (hBorderTL) m_pClientDE->DeleteSurface (hBorderTL);
		if (hBorderTR) m_pClientDE->DeleteSurface (hBorderTR);
		if (hBorderBR) m_pClientDE->DeleteSurface (hBorderBR);
		if (hBorderBL) m_pClientDE->DeleteSurface (hBorderBL);
		if (hBorderL) m_pClientDE->DeleteSurface (hBorderL);
		if (hBorderR) m_pClientDE->DeleteSurface (hBorderR);
		if (hBorderT) m_pClientDE->DeleteSurface (hBorderT);
		if (hBorderB) m_pClientDE->DeleteSurface (hBorderB);
		if (hFont) m_pClientDE->DeleteFont (hFont);
		if (hString) m_pClientDE->FreeString (hString);

		return LTFALSE;
	}
	
	if (!(hBorderTL = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuTL.pcx"))) goto fail;
	if (!(hBorderTR = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuTR.pcx"))) goto fail;
	if (!(hBorderBR = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuBR.pcx"))) goto fail;
	if (!(hBorderBL = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuBL.pcx"))) goto fail;
	if (!(hBorderL = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuL.pcx"))) goto fail;
	if (!(hBorderT = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuT.pcx"))) goto fail;
	if (!(hBorderR = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuR.pcx"))) goto fail;
	if (!(hBorderB = m_pClientDE->CreateSurfaceFromBitmap ("interface/PopupMenuB.pcx"))) goto fail;

	// create the cursor

	HSTRING hstrFont = m_pClientDE->FormatString (IDS_INGAMEFONT);
	hFont = m_pClientDE->CreateFont (m_pClientDE->GetStringData(hstrFont), 8, 16, LTFALSE, LTFALSE, LTFALSE);
	m_pClientDE->FreeString (hstrFont);

	if (!hFont) goto fail;
	HLTCOLOR hForeText = m_pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, LTFALSE);
	
	m_hCursor = CTextHelper::CreateSurfaceFromString (m_pClientDE, hFont, "_", hForeText);
	if (!m_hCursor) goto fail;
	
	uint32 nDummy;
	m_pClientDE->GetSurfaceDims (m_hCursor, &m_cxCursor, &nDummy);
	
	m_pClientDE->DeleteFont (hFont);
	hFont = LTNULL;
		
	// create the menu background

	m_hBackground = m_pClientDE->CreateSurface (Width(), Height());
	if (!m_hBackground) goto fail;
	m_pClientDE->FillRect (m_hBackground, NULL, NULL);

	m_pClientDE->DrawSurfaceToSurface (m_hBackground, hBorderTL, NULL, 0, 0);
	m_pClientDE->DrawSurfaceToSurface (m_hBackground, hBorderTR, NULL, Width() - BORDERSIZE, 0);
	m_pClientDE->DrawSurfaceToSurface (m_hBackground, hBorderBR, NULL, Width() - BORDERSIZE, Height() - BORDERSIZE);
	m_pClientDE->DrawSurfaceToSurface (m_hBackground, hBorderBL, NULL, 0, Height() - BORDERSIZE);

	LTRect rcDst;
	rcDst.left = 0;
	rcDst.top = BORDERSIZE;
	rcDst.right = BORDERSIZE;
	rcDst.bottom = Height() - BORDERSIZE;

	LTRect rcSrc;
	rcSrc.left = 0;
	rcSrc.top = 0;
	rcSrc.right = BORDERSIZE;
	rcSrc.bottom = 1;

	m_pClientDE->ScaleSurfaceToSurface (m_hBackground, hBorderL, &rcDst, &rcSrc);

	
	/*
	LTRect rcD, rcS;
	rcD.left = 10;
	rcD.top = 10;
	rcD.right = 16;
	rcD.bottom = 11;
	rcS.left = 0;
	rcS.top = 0;
	rcS.right = 5;
	rcS.bottom = 0;
	m_pClientDE->ScaleSurfaceToSurface (m_hBackground, hBorderL, &rcD, &rcS);
	rcD.left = 20;
	rcD.top = 20;
	rcD.right = 26;
	rcD.bottom = 26;
	m_pClientDE->DrawSurfaceToSurface (m_hBackground, hBorderL, NULL, rcD.left, rcD.top);
	*/

	rcDst.right = Width();
	rcDst.left = Width() - BORDERSIZE;

	m_pClientDE->ScaleSurfaceToSurface (m_hBackground, hBorderR, &rcDst, &rcSrc);

	rcDst.left = BORDERSIZE;
	rcDst.top = 0;
	rcDst.right = Width() - BORDERSIZE;
	rcDst.bottom = BORDERSIZE;

	rcSrc.right = 1;
	rcSrc.bottom = BORDERSIZE;

	m_pClientDE->ScaleSurfaceToSurface (m_hBackground, hBorderT, &rcDst, &rcSrc);

	rcDst.top = Height() - BORDERSIZE;
	rcDst.bottom = Height();

	m_pClientDE->ScaleSurfaceToSurface (m_hBackground, hBorderB, &rcDst, &rcSrc);

	// clean up

	m_pClientDE->DeleteSurface (hBorderTL);
	m_pClientDE->DeleteSurface (hBorderTR);
	m_pClientDE->DeleteSurface (hBorderBR);
	m_pClientDE->DeleteSurface (hBorderBL);
	m_pClientDE->DeleteSurface (hBorderL);
	m_pClientDE->DeleteSurface (hBorderT);
	m_pClientDE->DeleteSurface (hBorderR);
	m_pClientDE->DeleteSurface (hBorderB);

	return LTTRUE;
}

void CPopupMenu::Draw (HSURFACE hScreen)
{
	// would a flashing cursor be visible?

	LTBOOL bFlashingVisible = fmod (m_pClientDE->GetTime(), CURSORBLINKTIME) < (CURSORBLINKTIME / 2.0f);

	// draw the background to the screen

	m_pClientDE->DrawSurfaceToSurface (hScreen, m_hBackground, NULL, m_nLeft, m_nTop);

	// create the font and color we may need for any edit controls...

	HSTRING hstrFont = m_pClientDE->FormatString (IDS_INGAMEFONT);
	HLTFONT hFont = m_pClientDE->CreateFont (m_pClientDE->GetStringData(hstrFont), 8, 16, LTFALSE, LTFALSE, LTFALSE);
	m_pClientDE->FreeString (hstrFont);

	HLTCOLOR hForeText = m_pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, LTFALSE);
	
	// if a title exists, draw it

	int nCurrentY = BORDERSIZE + m_nTopMargin;
	
	if (m_hTitle)
	{
		uint32 nWidth, nHeight;
		m_pClientDE->GetSurfaceDims (m_hTitle, &nWidth, &nHeight);

		LTRect rcSrc;
		rcSrc.left = 0;
		rcSrc.top = 0;
		rcSrc.right = nWidth;
		rcSrc.bottom = nHeight;
		
		if ((int)nWidth > Width() - (BORDERSIZE << 1) - m_nLeftMargin)
		{
			rcSrc.right -= nWidth - (Width() - (BORDERSIZE << 1) - m_nLeftMargin);
		}
		if (nCurrentY + (int)nHeight > Height() - BORDERSIZE)
		{
			rcSrc.bottom -= (nCurrentY + nHeight) - (Height() - BORDERSIZE);
		}
		
		m_pClientDE->DrawSurfaceToSurface (hScreen, m_hTitle, &rcSrc, BORDERSIZE + m_nLeftMargin + m_nLeft, BORDERSIZE + m_nTopMargin + m_nTop);

		nCurrentY += (int) nHeight + m_nTopMargin;
	}

	// draw each item

	uint32 nCurrentItem = m_nFirstItem;
	while (nCurrentItem < m_nItems && nCurrentY < m_nBottom)
	{
		// get this item's width and clamp it if necessary

		uint32 nWidth, nHeight;
		m_pClientDE->GetSurfaceDims (m_itemArray[nCurrentItem].hSurface, &nWidth, &nHeight);

		LTRect rcSrc;
		rcSrc.left = 0;
		rcSrc.top = 0;
		rcSrc.right = nWidth;
		rcSrc.bottom = nHeight;

		if ((int)nWidth > Width() - (BORDERSIZE << 1) - m_nLeftMargin)
		{
			rcSrc.right -= nWidth - (Width() - (BORDERSIZE << 1) - m_nLeftMargin);
		}
		if (nCurrentY + m_itemArray[nCurrentItem].nHeight > Height() - BORDERSIZE)
		{
			rcSrc.bottom -= (nCurrentY + m_itemArray[nCurrentItem].nHeight) - (Height() - BORDERSIZE);
		}

		if (nCurrentItem == m_nSelection)
		{
			//HLTCOLOR hRed = m_pClientDE->SetupColor2 (1.0f, 0.0f, 0.0f, LTFALSE);
			//m_pClientDE->DrawSurfaceSolidColor (hScreen, m_itemArray[nCurrentItem].hSurface, &rcSrc, BORDERSIZE + m_nLeftMargin + m_nLeft, nCurrentY + m_nTop, NULL, hRed);
			m_pClientDE->DrawSurfaceToSurface (hScreen, m_itemArray[nCurrentItem].hSelected, &rcSrc, BORDERSIZE + m_nLeftMargin + m_nLeft, nCurrentY + m_nTop);
		}
		else
		{
			if (!m_itemArray[nCurrentItem].bEnabled)
			{
				//HLTCOLOR hDisabledColor = m_pClientDE->SetupColor2 (0.3f, 0.3f, 0.3f, LTFALSE);
				//m_pClientDE->DrawSurfaceSolidColor (hScreen, m_itemArray[nCurrentItem].hSurface, &rcSrc, BORDERSIZE + m_nLeftMargin + m_nLeft, nCurrentY + m_nTop, NULL, hDisabledColor);
				m_pClientDE->DrawSurfaceToSurface (hScreen, m_itemArray[nCurrentItem].hSurface, &rcSrc, BORDERSIZE + m_nLeftMargin + m_nLeft, nCurrentY + m_nTop);
			}
			else
			{
				m_pClientDE->DrawSurfaceToSurface (hScreen, m_itemArray[nCurrentItem].hSurface, &rcSrc, BORDERSIZE + m_nLeftMargin + m_nLeft, nCurrentY + m_nTop);
			}
		}

		// if this item is an edit control, draw the editable string...

		if (m_itemArray[nCurrentItem].nType == Edit && m_itemArray[nCurrentItem].pData)
		{
			char* pString = ((nCurrentItem == m_nSelection) && m_bEditing) ? m_strEdit : (char*)m_itemArray[nCurrentItem].pData;
			if (strlen (pString) == 0)
			{
				nWidth = 0;
			}
			else
			{
				HSURFACE hSurface = LTNULL;
				if (m_bEditing && nCurrentItem == m_nSelection)
				{
					hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, hFont, m_strEdit, hForeText);
				}
				else
				{
					hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, hFont, (char*)m_itemArray[nCurrentItem].pData, hForeText);
				}
				
				// get the source rect and clip it if necessary

				m_pClientDE->GetSurfaceDims (hSurface, &nWidth, &nHeight);
				rcSrc.right = nWidth;
				rcSrc.bottom = nHeight;

				if (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth > (Width() - (BORDERSIZE << 1) - m_nLeftMargin))
				{
					rcSrc.right -= (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth) - (Width() - (BORDERSIZE << 1) - m_nLeftMargin);
				}
				if (nCurrentY + (int)nHeight > Height() - BORDERSIZE)
				{
					rcSrc.bottom -= (nCurrentY + (int)nHeight) - (Height() - BORDERSIZE);
				}
				
				m_pClientDE->DrawSurfaceToSurface (hScreen, hSurface, &rcSrc, 
												   BORDERSIZE + m_nLeftMargin + m_nLeft + m_itemArray[nCurrentItem].nSecondColumn, 
												   nCurrentY + m_nTop);

				m_pClientDE->DeleteSurface (hSurface);
			}

			// if we're editing, draw a flashing cursor

			if (m_bEditing && bFlashingVisible && nCurrentItem == m_nSelection)
			{
				rcSrc.right = m_cxCursor;
				
				// if we're editing, we should be able to assume that we don't need to clip vertically

				if (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth + (int)m_cxCursor > (Width() - (BORDERSIZE << 1) - m_nLeftMargin))
				{
					rcSrc.right -= (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth + (int)m_cxCursor) - (Width() - (BORDERSIZE << 1) - m_nLeftMargin);
				}

				m_pClientDE->DrawSurfaceToSurface (hScreen, m_hCursor, &rcSrc,
												   BORDERSIZE + m_nLeftMargin + m_nLeft + m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth,
												   nCurrentY + m_nTop);
			}
		}
		
		// if this item is a multiple-choice control, draw the selected string...

		if (m_itemArray[nCurrentItem].nType == Multiple && m_itemArray[nCurrentItem].pData)
		{
			// find the current string

			char* pList = new char [strlen ((char*)m_itemArray[nCurrentItem].pData) + 1];
			strcpy (pList, (char*)m_itemArray[nCurrentItem].pData);

			char* ptr = strtok (pList, "|");// put the delimeter in the string resources!!!!
			for (uint32 i = 0; i < m_itemArray[nCurrentItem].nData; i++)
			{
				ptr = strtok (NULL, "|");
				assert (ptr);
			}

			HSURFACE hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, hFont, ptr, hForeText);
				
			delete [] pList;

			// get the source rect and clip it if necessary

			m_pClientDE->GetSurfaceDims (hSurface, &nWidth, &nHeight);
			rcSrc.right = nWidth;
			rcSrc.bottom = nHeight;

			if (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth > (Width() - (BORDERSIZE << 1) - m_nLeftMargin))
			{
				rcSrc.right -= (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth) - (Width() - (BORDERSIZE << 1) - m_nLeftMargin);
			}
			if (nCurrentY + (int)nHeight > Height() - BORDERSIZE)
			{
				rcSrc.bottom -= (nCurrentY + (int)nHeight) - (Height() - BORDERSIZE);
			}
			
			m_pClientDE->DrawSurfaceToSurface (hScreen, hSurface, &rcSrc, 
											   BORDERSIZE + m_nLeftMargin + m_nLeft + m_itemArray[nCurrentItem].nSecondColumn, 
											   nCurrentY + m_nTop);

			m_pClientDE->DeleteSurface (hSurface);
		}

		// if this item is a KeyConfig control, draw the associated key...

		if (m_itemArray[nCurrentItem].nType == KeyConfig && m_itemArray[nCurrentItem].pData)
		{
			HSURFACE hSurface = LTNULL;
			if (nCurrentItem == m_nSelection && m_bConfiguring)
			{
				hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, hFont, "Press a key...", hForeText);
			}
			else
			{
				hSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, hFont, (char*)m_itemArray[nCurrentItem].pData, hForeText);
			}
				
			// get the source rect and clip it if necessary

			m_pClientDE->GetSurfaceDims (hSurface, &nWidth, &nHeight);
			rcSrc.right = nWidth;
			rcSrc.bottom = nHeight;

			if (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth > (Width() - (BORDERSIZE << 1) - m_nLeftMargin))
			{
				rcSrc.right -= (m_itemArray[nCurrentItem].nSecondColumn + (int)nWidth) - (Width() - (BORDERSIZE << 1) - m_nLeftMargin);
			}
			if (nCurrentY + (int)nHeight > Height() - BORDERSIZE)
			{
				rcSrc.bottom -= (nCurrentY + (int)nHeight) - (Height() - BORDERSIZE);
			}
			
			m_pClientDE->DrawSurfaceToSurface (hScreen, hSurface, &rcSrc, 
											   BORDERSIZE + m_nLeftMargin + m_nLeft + m_itemArray[nCurrentItem].nSecondColumn, 
											   nCurrentY + m_nTop);

			m_pClientDE->DeleteSurface (hSurface);
		}

		// if this item is a Slider, draw the slider...

		if (m_itemArray[nCurrentItem].nType == Slider && m_itemArray[nCurrentItem].pData)
		{
			CSlider* pSlider = (CSlider*) m_itemArray[nCurrentItem].pData;
			
			int nHeight = pSlider->GetHeight();
			int y = (m_itemArray[nCurrentItem].nHeight - nHeight) / 2;
			
			if (y + nCurrentY + nHeight <= Height() - BORDERSIZE)
			{
				pSlider->Draw (hScreen, BORDERSIZE + m_nLeftMargin + m_nLeft + m_itemArray[nCurrentItem].nSecondColumn, m_nTop + nCurrentY + y);
			}
		}

		// increment the current y value and go on to the next item...

		nCurrentY += m_itemArray[nCurrentItem].nHeight;

		if (nCurrentY < Height() - BORDERSIZE) m_nLastItem = nCurrentItem;

		nCurrentItem++;
	}

	// clean up

	m_pClientDE->DeleteFont (hFont);
}

void CPopupMenu::AddItem (POPUPMENUITEM* ppmi)
{
	m_itemArray[m_nItems++] = *ppmi;
}

LTBOOL CPopupMenu::ScrollUp()
{
	if (m_bEditing) return LTFALSE;

	uint32 nOriginalSelection = m_nSelection;

	if (m_nSelection > 0) m_nSelection--;

	if (!m_itemArray[m_nSelection].bEnabled)
	{
		if (m_nSelection > 0)
		{
			m_nSelection--;
		}
		else
		{
			m_nSelection++;
		}
	}

	if (m_nSelection < m_nFirstItem)
	{
		m_nFirstItem = m_nSelection;
	}

	if (m_nSelection == nOriginalSelection) return LTFALSE;

	return LTTRUE;
}

LTBOOL CPopupMenu::ScrollDown()
{
	if (m_bEditing) return LTFALSE;

	uint32 nOriginalSelection = m_nSelection;

	if (m_nSelection < m_nItems - 1)
	{
		m_nSelection++;
		if (!m_itemArray[m_nSelection].bEnabled)
		{
			if (m_nSelection < m_nItems - 1)
			{
				m_nSelection++;
			}
			else
			{
				m_nSelection--;
			}
		}
	}

	if (m_nSelection > m_nLastItem)
	{
		m_nFirstItem++;
	}

	if (m_nSelection == nOriginalSelection) return LTFALSE;

	return LTTRUE;
}

LTBOOL CPopupMenu::PageUp()
{
	// this routine DOES NOT account for disabled controls!!!

	if (m_bEditing) return LTFALSE;

	uint32 nOriginalSelection = m_nSelection;

	int nVisible = m_nLastItem - m_nFirstItem + 1;
	if ((int)m_nFirstItem - nVisible < 0)
	{
		m_nFirstItem = 0;
		m_nSelection = 0;
	}
	else
	{
		m_nFirstItem -= nVisible;
		m_nSelection -= nVisible;
	}

	if (m_nSelection == nOriginalSelection) return LTFALSE;

	return LTTRUE;
}

LTBOOL CPopupMenu::PageDown()
{
	// this routine DOES NOT account for disabled controls!!!

	uint32 nOriginalSelection = m_nSelection;

	int nVisible = m_nLastItem - m_nFirstItem + 1;
	if (m_nLastItem + nVisible <= m_nItems - 1)
	{
		m_nFirstItem = m_nLastItem + 1;
		m_nSelection += nVisible;
	}
	else
	{
		int nDiff = (m_nItems - nVisible) - m_nFirstItem;
		m_nFirstItem += nDiff;
		m_nSelection += nDiff;
		if (m_nSelection >= m_nItems) m_nSelection = m_nItems - 1;
	}

	if (m_nSelection == nOriginalSelection) return LTFALSE;

	return LTTRUE;
}

int CPopupMenu::SelectCurrentItem()
{
	if (!m_nItems) return 0;

	if (m_itemArray[m_nSelection].nType == List)
	{
		// call the ProcessItemChange function

		//m_pRiotMenu->ProcessItemChange (this, &m_itemArray[m_nSelection]);

		// now return the id of this item

		return m_itemArray[m_nSelection].nID;
	}
	else if (m_itemArray[m_nSelection].nType == Edit && m_itemArray[m_nSelection].pData)
	{
		m_bEditing = !m_bEditing;

		if (m_bEditing)
		{
			// create a temporary string the length of the item's edit string

			m_strEdit = new char [m_itemArray[m_nSelection].nData];
			memset (m_strEdit, 0, m_itemArray[m_nSelection].nData);
		}
		else
		{
			// call the ProcessItemChange function

			//m_pRiotMenu->ProcessItemChange (this, &m_itemArray[m_nSelection], m_strEdit);
			
			// set permanent string equal to temporary string,
			// and delete the temporary string

			strcpy ((char*)m_itemArray[m_nSelection].pData, m_strEdit);
			delete [] m_strEdit;
			m_strEdit = LTNULL;
		}

		return 0;
	}
	else if (m_itemArray[m_nSelection].nType == KeyConfig)
	{
		m_bConfiguring = LTTRUE;
	}
	else if (m_itemArray[m_nSelection].nType == PopupMenu && m_itemArray[m_nSelection].pData)
	{
		//m_pRiotMenu->SetPopupMenu ((CPopupMenu*) m_itemArray[m_nSelection].pData);
	}

	return 0;
}

void CPopupMenu::ProcessKey (int nKey, int nRep)
{
	// if we are editing an editable control, process user input

	if (m_bConfiguring)
	{
		if (nKey == VK_ESCAPE)
		{
			m_bConfiguring = LTFALSE;
		}
		else
		{
			// call the ProcessItemChange function
			
			//m_pRiotMenu->ProcessItemChange (this, &m_itemArray[m_nSelection]);

			// temporary solution
			itoa (nKey, (char*)m_itemArray[m_nSelection].pData, 10);
			m_bConfiguring = LTFALSE;
		}
	}
	else if (m_bEditing)
	{
		char nChar = VKToASCII (nKey);
		int nStrLen = strlen (m_strEdit);
		if (nChar >= 32 && nChar <= 126)
		{
			// add character to the string
			if (nStrLen == (int)m_itemArray[m_nSelection].nData - 1) return;
			m_strEdit[nStrLen] = nChar;
		}
		else if (nChar == 8)	// Backspace
		{
			// remove last character from the string
			
			if (nStrLen == 0) return;
			m_strEdit[nStrLen - 1] = '\0';
		}
		else if (nChar == 27)	// ESC
		{
			// discard temporary string and stop editing

			delete [] m_strEdit;
			m_strEdit = LTNULL;

			m_bEditing = LTFALSE;
		}
	}

	// if this is a multiple-choice control, only look at left and right arrows

	if (m_itemArray[m_nSelection].nType == Multiple && m_itemArray[m_nSelection].pData)
	{
		if (nKey == VK_RIGHT || nKey == VK_RETURN)
		{
			char* pList = new char [strlen ((char*) m_itemArray[m_nSelection].pData) + 1];
			strcpy (pList, (char*) m_itemArray[m_nSelection].pData);

			// see if there is another available selection to the right
			char* ptr = strtok (pList, "|");
			for (uint32 i = 0; i < m_itemArray[m_nSelection].nData; i++)
			{
				ptr = strtok (NULL, "|");
				assert (ptr);
			}
			ptr = strtok (NULL, "|");
			
			if (ptr == LTNULL)
			{
				m_itemArray[m_nSelection].nData = 0;
			}
			else
			{
				m_itemArray[m_nSelection].nData++;
			}

			delete [] pList;

			// call the ProcessItemChange function

			//m_pRiotMenu->ProcessItemChange (this, &m_itemArray[m_nSelection]);
		}
		else if (nKey == VK_LEFT)
		{
			if (m_itemArray[m_nSelection].nData > 0)
			{
				m_itemArray[m_nSelection].nData--;
			}
			else
			{
				// count how many strings there are

				char* pList = new char [strlen ((char*) m_itemArray[m_nSelection].pData) + 1];
				strcpy (pList, (char*) m_itemArray[m_nSelection].pData);

				int nCount = 0;
				char* ptr = strtok (pList, "|");
				while (ptr)
				{
					nCount++;
					ptr = strtok (NULL, "|");
				}

				m_itemArray[m_nSelection].nData = nCount - 1;

				delete [] pList;
			}
			
			// call the ProcessItemChange function

			//m_pRiotMenu->ProcessItemChange (this, &m_itemArray[m_nSelection]);
		}
	}

	if (m_itemArray[m_nSelection].nType == Slider && m_itemArray[m_nSelection].pData)
	{
		if (nKey == VK_RIGHT)
		{
			PlaySoundLocal ("Sounds\\Interface\\Scroll.wav", SOUNDPRIORITY_MISC_MEDIUM);
			if (((CSlider*)m_itemArray[m_nSelection].pData)->IncPos())
			{
				// call the ProcessItemChange function
				//m_pRiotMenu->ProcessItemChange (this, &m_itemArray[m_nSelection]);
			}
		}
		else if (nKey == VK_LEFT)
		{
			PlaySoundLocal ("Sounds\\Interface\\Scroll.wav", SOUNDPRIORITY_MISC_MEDIUM);
			if (((CSlider*)m_itemArray[m_nSelection].pData)->DecPos())
			{
				// call the ProcessItemChange function
				//m_pRiotMenu->ProcessItemChange (this, &m_itemArray[m_nSelection]);
			}
		}
	}
}
