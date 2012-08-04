// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUILargeText.cp
//
// PURPOSE : Simple text control which may be used as a menu item.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "LTGUILargeText.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUILargeText::CLTGUILargeText()
{
    m_nFixedHeight		= 0;
	m_nNumLines = 0;
	memset(m_nLineOffsets,0,sizeof(m_nLineOffsets));
	m_nFirstLine = kMaxNumLines+1;
	m_nLastLine = kMaxNumLines+1;
	m_nFrameWidth			= 0;
	memset(m_Frame,0,sizeof(m_Frame));

	m_pUp = LTNULL;
	m_pDown = LTNULL;

}

CLTGUILargeText::~CLTGUILargeText()
{
	Destroy();
}

// Create the control
LTBOOL CLTGUILargeText::Create (  	const char *pText, CUIFont *pFont, uint8 nFontSize, LTIntPt ptTextSize )
{
	if (!SetFont(pFont,nFontSize))
	{
        return LTFALSE;
	}

	if (!SetSize(ptTextSize))
	{
		return LTFALSE;
	}

	// Add the string
	if (pText)
	{
		SetString(pText);
	}


    return LTTRUE;
}

// Destroys the control
void CLTGUILargeText::Destroy ( )
{
	CLTGUITextCtrl::Destroy ( );
	if (m_pUp)
	{
		debug_delete(m_pUp);
		m_pUp = LTNULL;
	}
	if (m_pDown)
	{
		debug_delete(m_pDown);
		m_pDown = LTNULL;
	}

}


LTBOOL CLTGUILargeText::SetSize(LTIntPt ptTextSize)
{
	if (ptTextSize.x < (m_nBaseFontSize * 2))
		return LTFALSE;
	if (ptTextSize.y < m_nBaseFontSize)
		return LTFALSE;

	SetFixedWidth(ptTextSize.x);
	m_nFixedHeight = ptTextSize.y;

	CalculateSize();

	return LTTRUE;
}


void CLTGUILargeText::CalculateSize()
{

	m_nWidth = (uint16)(m_fScale * (float)m_nFixedWidth);
	m_nHeight = (uint16)(m_fScale * (float)m_nFixedHeight);

	if (m_pUp)
		m_nWidth += m_pUp->GetWidth();

	float frameW = ((float)m_nFrameWidth * m_fScale);

	//top
	float fx = (float)m_pos.x - frameW;
	float fy = (float)m_pos.y - frameW;
	float fw = (float)m_nWidth + frameW;
	float fh = frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[0],fx,fy,fw,fh);

	//right
	fx = (float)(m_pos.x + m_nWidth);
	fy = (float)m_pos.y - frameW;
	fw = (float)frameW;
	fh = (float)m_nHeight + frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[1],fx,fy,fw,fh);

	//bottom
	fx = (float)m_pos.x;
	fy = (float)(m_pos.y + m_nHeight);
	fw = (float)m_nWidth + frameW;
	fh = frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[2],fx,fy,fw,fh);

	//left
	fx = (float)m_pos.x - frameW;
	fy = (float)m_pos.y;
	fw = (float)frameW;
	fh = (float)m_nHeight + frameW;
	g_pDrawPrim->SetXYWH(&m_Frame[3],fx,fy,fw,fh);


	CalculateLines();
}


void CLTGUILargeText::CalculateLines()
{
	m_nNumLines = 0;
	memset(m_nLineOffsets,0,sizeof(m_nLineOffsets));
	m_nFirstLine = kMaxNumLines+1;
	m_nLastLine = kMaxNumLines+1;

	if (m_pString && m_pString->GetLength())
	{
		LT_POLYGT4*	pPolys = m_pString->GetPolys();

		uint16 nPos = 0;
		float fYPos = pPolys[nPos].verts[ 0 ].y;
		m_nNumLines = 1;
		m_nLineOffsets[0] = 0;

		//if the pos is in our rect, and we haven't found the first line yet, this is it
		if (fYPos >= m_pos.y)
			m_nFirstLine = 0;



		//while we have chars to test
		while (nPos < m_pString->GetLength())
		{
			//find the top of the char
			float fTestY = pPolys[nPos].verts[ 0 ].y;

			//have started a new line?
			if (fTestY > fYPos)
			{
				//if the pos is in our rect, and we haven't found the first line yet, this is it
				if (fTestY >= m_pos.y && m_nFirstLine > kMaxNumLines)
					m_nFirstLine = m_nNumLines;

				//if the os is past the bottom of our rect, and we haven't found the last line, this is it
				if (fTestY+m_nFontSize >= (m_pos.y+m_nHeight) && m_nLastLine > kMaxNumLines)
					m_nLastLine = m_nNumLines-1;

				fYPos = fTestY;
				m_nLineOffsets[m_nNumLines] = nPos;
				m_nNumLines++;



				if (m_nNumLines >= kMaxNumLines)
				{
					ASSERT(!"CLTGUILargeText::SetSize() : Too many lines.");
					return;
				}
				
			}
			nPos++;
		} 

		//if we never ran past the bottom, the last line visible is the last line
		if (m_nLastLine > kMaxNumLines)
			m_nLastLine = m_nNumLines-1;
		
	}

}

// Render the control
void CLTGUILargeText::Render()
{
	// Sanity checks...
	if (!IsVisible() || !m_pString) return;

	if (m_nFrameWidth)
	{
		// set up the render state	
		SetRenderState();

		for (int f = 0;f < 4; ++f)
			g_pDrawPrim->SetRGBA(&m_Frame[f],GetCurrentColor());

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Frame,4);
	}

	
    uint32 argbColor=GetCurrentColor();
	m_pString->SetColor(argbColor);
	
	//if we know what lines to draw
	if (m_nFirstLine <= kMaxNumLines)
	{
		//figure out the first char
		uint16 nFirstChar = m_nLineOffsets[m_nFirstLine];
		uint16 nLastChar = m_pString->GetLength();

		//if the last line drawn is not the last overall line, 
		// find the first char of the next line and step back one
		if (m_nLastLine < (m_nNumLines-1))
			nLastChar = m_nLineOffsets[m_nLastLine+1];

		// draw 'em
		m_pString->Render(nFirstChar,nLastChar);

		if (m_pUp && m_nFirstLine > 0)
			m_pUp->Render();
		if (m_pDown && m_nFirstLine < (m_nNumLines-1) )
			m_pDown->Render();

	}
}


// Sets the width of the text's frame, set to 0 to not show the frame
void CLTGUILargeText::SetFrameWidth(uint8 nFrameWidth)
{
	m_nFrameWidth = nFrameWidth;
	if (nFrameWidth)
	{
		CalculateSize();
	}
}


void CLTGUILargeText::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_NOCOLOROP);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
		
}

void CLTGUILargeText::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	if (m_pString)
	{
		float fi = (float)m_nIndent * m_fScale;

		float line = 0;
		if (m_nFirstLine <= kMaxNumLines)
		{
			line = (float)(m_nFirstLine * m_nFontSize);
		}


		m_pString->SetPosition((float)m_pos.x+fi,(float)m_pos.y+line);
	}

	if (m_pUp)
	{
		LTIntPt pos = m_basePos;
		pos.x += m_nFixedWidth;
		m_pUp->SetBasePos(pos);
	}

	if (m_pDown)
	{
		LTIntPt pos = m_basePos;
		pos.x += m_nFixedWidth;
		pos.y += (m_nFixedHeight - m_pDown->GetHeight());
		m_pDown->SetBasePos(pos);
	}

}

void CLTGUILargeText::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	uint16 nWidth = (uint16)(m_fScale * (float)(m_nFixedWidth-m_nIndent));
	if (m_pString)
	{
		float fi = (float)m_nIndent * m_fScale;
		float line = 0;
		if (m_nFirstLine <= kMaxNumLines)
		{
			line = (float)(m_nFirstLine * m_nFontSize);
		}

		m_pString->SetPosition((float)m_pos.x+fi,(float)m_pos.y+line);
		m_pString->SetCharScreenHeight(m_nFontSize);
		m_pString->SetWrapWidth(nWidth);
	}
	if (m_pUp)
	{
		m_pUp->SetScale(m_fScale);
	}
	if (m_pDown)
	{
		m_pDown->SetScale(m_fScale);
	}

	CalculateSize();

}


void CLTGUILargeText::SetIndent(uint16 nIndent)
{
	if (m_nFixedWidth)
	{
		ASSERT(m_nFixedWidth > nIndent);
		if(m_nFixedWidth <= nIndent)
			return;
	}

	m_nIndent = nIndent;

	if (m_nFixedWidth && !m_bClip)
	{
		uint16 nTrueWidth = (uint16)(m_fScale * (float)(m_nFixedWidth-m_nIndent));
		if (m_pString)
		{
			m_pString->SetWrapWidth(nTrueWidth);
		}

	}

	if (m_pString)
	{
		float fi = (float)m_nIndent * m_fScale;
		float line = 0;
		if (m_nFirstLine <= kMaxNumLines)
		{
			line = (float)(m_nFirstLine * m_nFontSize);
		}

		m_pString->SetPosition((float)m_pos.x+fi,(float)m_pos.y+line);
	}

	CalculateSize();

}


LTBOOL CLTGUILargeText::OnUp ( )
{
	if (!m_pString || m_nFirstLine == 0 || m_nFirstLine > kMaxNumLines)
		return LTFALSE;
	m_nFirstLine--;
	float fi = (float)m_nIndent * m_fScale;

	LT_POLYGT4*	pPolys = m_pString->GetPolys();

	uint16 nPos = m_nLineOffsets[m_nFirstLine];
	float fYPos = pPolys[nPos].verts[ 0 ].y;
	float fBasePos = pPolys[0].verts[ 0 ].y;
	float offset = fYPos - fBasePos;

	m_pString->SetPosition((float)m_pos.x+fi,(float)m_pos.y-offset);
	CalculateLines();

	return LTTRUE;


}

LTBOOL CLTGUILargeText::OnDown ( )
{
	if (!m_pString || m_nFirstLine >= (m_nNumLines-1) )
		return LTFALSE;
	m_nFirstLine++;
	float fi = (float)m_nIndent * m_fScale;

	LT_POLYGT4*	pPolys = m_pString->GetPolys();

	uint16 nPos = m_nLineOffsets[m_nFirstLine];
	float fYPos = pPolys[nPos].verts[ 0 ].y;
	float fBasePos = pPolys[0].verts[ 0 ].y;
	float offset = fYPos - fBasePos;

	m_pString->SetPosition((float)m_pos.x+fi,(float)m_pos.y-offset);
	CalculateLines();

	return LTTRUE;
}


// Handles the left button down message
LTBOOL CLTGUILargeText::OnLButtonDown(int x, int y)
{
	// Get the control that the click was on
	if (m_pUp && m_pUp->IsOnMe(x,y))
	{
		m_pUp->Select(LTTRUE);
		return LTTRUE;
	}
	if (m_pDown && m_pDown->IsOnMe(x,y))
	{
		m_pDown->Select(LTTRUE);
		return LTTRUE;
	}

	return LTFALSE;
}

// Handles the left button up message
LTBOOL CLTGUILargeText::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	if (m_pUp && m_pUp->IsOnMe(x,y) && m_pUp->IsSelected())
	{
		m_pUp->Select(LTFALSE);
		return OnUp();
	}
	if (m_pDown && m_pDown->IsOnMe(x,y) && m_pDown->IsSelected())
	{
		m_pDown->Select(LTFALSE);
		return OnDown();
	}

	return LTFALSE;
}

// Handles the mouse move message
LTBOOL CLTGUILargeText::OnMouseMove(int x, int y)
{
	if (m_pUp && !m_pUp->IsOnMe(x,y))
		m_pUp->Select(LTFALSE);
	if (m_pDown && !m_pDown->IsOnMe(x,y))
		m_pDown->Select(LTFALSE);

	return LTFALSE;
}



LTBOOL CLTGUILargeText::UseArrows(LTFLOAT fTextureScale, HTEXTURE hUpNormal,  
								  HTEXTURE hUpSelected, HTEXTURE hDownNormal,  HTEXTURE hDownSelected)
{
	if (m_pUp)
	{
		debug_delete(m_pUp);
		m_pUp = LTNULL;
	}
	if (m_pDown)
	{
		debug_delete(m_pDown);
		m_pDown = LTNULL;
	}


	m_pUp = debug_new(CLTGUIButton);
	m_pUp->Create(LTNULL,LTNULL,hUpNormal,hUpSelected);
	m_pUp->SetTextureScale(fTextureScale);
	LTIntPt pos = m_basePos;
	pos.x += m_nFixedWidth;
	m_pUp->SetBasePos(pos);
	m_pUp->SetScale(m_fScale);

	m_pDown = debug_new(CLTGUIButton);
	m_pDown->Create(LTNULL,LTNULL,hDownNormal,hDownSelected);
	m_pDown->SetTextureScale(fTextureScale);
	pos.y += (m_nFixedHeight - m_pDown->GetHeight());
	m_pDown->SetBasePos(pos);
	m_pDown->SetScale(m_fScale);
	
	CalculateSize();

	return LTTRUE;
}

