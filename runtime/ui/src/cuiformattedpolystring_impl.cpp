//-------------------------------------------------------------------
//
//   MODULE    : CUIFORMATTEDPOLYSTRING_IMPL.CPP
//
//   PURPOSE   : implements the CUIFormattedPolyString_Impl text 
//				 class.
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------



#ifndef __CUIFORMATTEDPOLYSTRING_IMPL_H__
#include "cuiformattedpolystring_impl.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif


// use the internal drawprim interface
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);

static ILTTexInterface *pTexInterface = NULL;
define_holder(ILTTexInterface, pTexInterface);


//  ---------------------------------------------------------------------------
CUIFormattedPolyString_Impl::CUIFormattedPolyString_Impl(CUIFont* pFont, 
							 const char* pText,
							 float x,
							 float y,
							 CUI_ALIGNMENTTYPE alignment)
{
	m_Valid		= false;
	m_pFont		= pFont;
	m_pChars	= NULL;
	m_pPolys	= NULL;	
	m_pLetters	= NULL;

	m_Alignment	= alignment;

	m_Flags		= 0;
	m_Bold		= 0;
	m_Italic	= 0;

	m_WordCount = 0;
	m_LineCount = 0;
	
	m_NumAllocatedChars = 0;
	m_NumAllocatedPolys = 0;

	m_Length	= 0;

	if (!pFont) return; // string will be invalid
	m_CharScreenWidth  = pFont->GetDefCharScreenWidth();
	m_CharScreenHeight = pFont->GetDefCharScreenHeight();

	memcpy(&m_pColors, pFont->GetDefColors(), sizeof(uint32) * 4);

	m_X = x;
	m_Y = y;

	memset(&m_Rect, 0, sizeof(CUIRECT));

	m_WrapWidth		= 0;

	// must be valid before we call settext!
	m_Valid = true;

	if (pText) {
		this->SetText(pText);	
	}	

}


//  ---------------------------------------------------------------------------
CUIFormattedPolyString_Impl::~CUIFormattedPolyString_Impl() 
{
	// ascii (handled by superclass)
	//if (m_pChars)	delete [] m_pChars;

	// rendering (handled by superclass)
	//if (m_pPolys)	delete [] m_pPolys;	

	// formatting
	if (m_pLetters)	delete [] m_pLetters;
	m_pLetters = NULL;

	m_Valid = false;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetCharScreenWidth(uint8 width)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	m_CharScreenWidth  = width;

	this->Parse();
	this->ApplyXYZ();

	GetExtents(&m_Rect);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetCharScreenHeight(uint8 height)
{
	if (!m_Valid)	return CUIR_INVALID_POLYSTRING;

	m_CharScreenHeight = height;

	this->Parse();
	this->ApplyXYZ();

	GetExtents(&m_Rect);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetCharScreenSize(uint8 height, uint8 width)
{
	if (!m_Valid)	return CUIR_INVALID_POLYSTRING;

	m_CharScreenHeight = height;
	m_CharScreenWidth  = width;

	this->Parse();
	this->ApplyXYZ();

	GetExtents(&m_Rect);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIFormattedPolyString_Impl::SetFont(CUIFont* pFont) 
{
	if (!m_Valid)	return CUIR_INVALID_POLYSTRING;

	if (pFont) {
		m_pFont = pFont;

		this->Parse();
		this->ApplyFont();
		this->ApplyXYZ();

		GetExtents(&m_Rect);

		return CUIR_OK;
	}

	return CUIR_NO_FONT;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIFormattedPolyString_Impl::SetAlignmentH(CUI_ALIGNMENTTYPE align)
{
	if (!m_Valid)	return CUIR_INVALID_POLYSTRING;

	m_Alignment = align;

	this->ApplyXYZ();

	GetExtents(&m_Rect);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIFormattedPolyString_Impl::SetWrapWidth(uint16 wrap)
{
	if (!m_Valid)	return CUIR_INVALID_POLYSTRING;

	m_WrapWidth = wrap;

	this->Parse();
	this->ApplyXYZ();

	GetExtents(&m_Rect);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIFormattedPolyString_Impl::SetText(const char* pBuf)
{
	// make sure we have a font pointer
	if (!m_pFont) {
		return CUIR_NO_FONT;
	}
	// and a non-empty character buffer
	if (!pBuf) {
		return CUIR_ERROR;
	}
	// is the font valid
	if (!m_Valid) {
		return CUIR_INVALID_POLYSTRING;
	}

	m_Length = strlen(pBuf);
	
	// allocate the characters if they don't exist or the buffer
	// ain't big enough
	if (!m_pChars || m_NumAllocatedChars < m_Length + (uint32)1) {
		if (m_pChars) 
			delete [] m_pChars;
		LT_MEM_TRACK_ALLOC(m_pChars = new char[m_Length+1],LT_MEM_TYPE_UI);	// str + NULL
		m_NumAllocatedChars = m_Length+1;

		if (m_pLetters)
			delete [] m_pLetters;
		LT_MEM_TRACK_ALLOC(m_pLetters = new uint8[m_NumAllocatedChars*2],LT_MEM_TYPE_UI);
	}

	if (m_Length && (!m_pChars || !m_pLetters)) {
		CUI_ERR("Out of Memory\n");
		m_NumAllocatedChars = 0;
		return CUIR_OUT_OF_MEMORY;
	}

	strcpy(m_pChars, pBuf);

	// allocate the polys if they don't exist or the array
	// ain't big enough
	if (!m_pPolys || m_NumAllocatedPolys < m_Length) {
		if (m_pPolys) 
			delete [] m_pPolys;
		LT_MEM_TRACK_ALLOC(m_pPolys = new LT_POLYGT4[m_Length],LT_MEM_TYPE_UI);
		m_NumAllocatedPolys = m_Length;
		memset(m_pPolys, 0, sizeof(LT_POLYGT4) * m_NumAllocatedPolys);
	}
	
	if (m_Length && !m_pPolys) {
		CUI_ERR("Out of Memory\n");
		m_NumAllocatedPolys = 0;
		return CUIR_OUT_OF_MEMORY;
	}

	// ask the font to set up the polygons
	// they can be manipulated later
	this->Parse();
	this->ApplyFont();
	this->ApplyXYZ();
	this->ApplyColors();
	
	// get the extents of the new polystring
	GetExtents(&m_Rect);

	return CUIR_OK;
}



//  ---------------------------------------------------------------------------
//  ---------------------------------------------------------------------------
//
//		APPLY FUNCTIONS
//
//  ---------------------------------------------------------------------------
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
void CUIFormattedPolyString_Impl::Parse()
{
	// this function goes through a string and calculates word boundaries
	// and line breaks

	// m_pWords[] = | word start | word end | pixel width |
	
	// m_pLines[] = | word index of line end | wrapped? |

	// m_pLetters = | table index | screen width |


	// sanity check -- don't bother with an empty string
	if (!m_Length) return;


	int32 i ,line_width, old_wordcount;
	uint8 cidx;

	CUIFont*	pFont		= m_pFont;
	uint8*		pFontMap	= m_pFont->GetFontMap();
	uint16*		pFontTable	= m_pFont->GetFontTable();

	float sh, cth, ctw;
	float storedw = m_CharScreenWidth;
	
	sh = m_CharScreenHeight;

	cth = m_pFont->GetCharTexHeight();

	m_WordCount = m_LineCount = 0;
    line_width = 0;

	// see which FX to apply
	uint32 bd = 0;
	uint32 em = 0;
	uint32 sp = m_pFont->GetDefSpacingH();

	if (m_Flags & CUI_FONT_BOLD)	bd = m_Bold;
	if (m_Flags & CUI_FONT_ITALIC)	em = m_Italic;

	bool bNoDraw;
	bool bNewline		= false;
	bool bWhitespace	= true;
	bool bClamp			= false;

	memset(m_pWords, 0, sizeof(uint16) * MAX_POLYSTRING_WORDS);
	memset(m_pLines, 0, sizeof(uint16) * MAX_POLYSTRING_LINES);
	memset(m_pLetters, 0, sizeof(uint8) * m_NumAllocatedChars * 2);

	old_wordcount = 0;

	for (i=0; i<m_Length; i++) 
	{

		////////////////////////////////////////////////////////////
		//	store the width of each character
		//	get the char index
		//	get the width of the character's texture

		cidx = m_pChars[i];
		
		switch (cidx) {

			case 10:
				bNoDraw  = true;
				bNewline = true;
				cidx = 254;
				break;

			case 32:
				bNoDraw = true;
				cidx = 255;
				break;

			default:
				if (pFontMap) cidx = pFontMap[cidx];
				bNoDraw = false;
		}

		char* pStr = &m_pChars[i];

		// get the width of the character's texture
		m_pLetters[i*2] = cidx;



		if (pFontTable && !bNoDraw)
		{
			// if there's a font table, this means a proportional font, and
			// a few extra calculations
			ctw = pFontTable[cidx * 3];
			m_pLetters[i*2+1]  = ( uint8 )((( float )ctw * sh / cth ) + 0.5f );
		}
		else
		{
			if (bNewline)
			{
				m_pLetters[i*2+1]  = (uint8)(0.5f);
			}
			else
			{
				// Proportial fonts need white space calculation  
				if (pFontTable)
				{
					m_pLetters[i*2+1]  = ( uint8 )((( float )m_CharScreenWidth * sh / cth ) + 0.5f );
				}
				else
				{
					m_pLetters[i*2+1]  = (uint8)(m_CharScreenWidth);
				}
			}
		}

		////////////////////////////////////////////////////////////
		//	get the word breaks -- may only be necessary for 
		//	justified text

		if (bNoDraw) {
			if (!bWhitespace) { // we hit whitespace after a letter
				bWhitespace = true;
				m_pWords[m_WordCount * 3 + 1] = i-1;	
				m_WordCount++;
				if (m_WordCount >= MAX_POLYSTRING_WORDS-1) {
					ASSERT(0);
					bClamp = true;
				}
			}
		}
		else {
			if (bWhitespace) { // we hit a letter after whitespace
				bWhitespace = false;
				m_pWords[m_WordCount * 3] = (uint16)i;
				
			}
		}

		if (bClamp) break;

		////////////////////////////////////////////////////////////
		// calculate newlines and wraps

		line_width += m_pLetters[i*2+1] + bd + sp + ( em > 0 ? em : 0);

		// need to also handle the case where there's a really long word that
		// is wider than m_WrapWidth

		if ( m_WrapWidth  && (line_width >= m_WrapWidth) ) {

 
            if (old_wordcount == m_WordCount) {
                // insert an artificial word break!
                bWhitespace = true;
                m_pWords[m_WordCount * 3 + 1] = i-1;    
                i--;
                m_WordCount++;
				if (m_WordCount >= MAX_POLYSTRING_WORDS-1) {
					ASSERT(0);
					bClamp = true;
				}


				old_wordcount = m_WordCount;
			}
			m_pLines[m_LineCount*2]		= m_WordCount > 0 ? m_WordCount - 1 : 0;
			m_pLines[m_LineCount*2 + 1]	= 1;

			if (bClamp) break;

			line_width = 0;

			// recalc the line width for the overshoot chars
			if (old_wordcount != m_WordCount) {
				for (int q=m_pWords[m_pLines[m_LineCount*2] * 3 + 1]; q<=i; q++) {
					line_width += m_pLetters[q*2+1] + bd + sp + ( em > 0 ? em : 0);
				}
			}
			m_LineCount++;
			if (m_LineCount >= MAX_POLYSTRING_LINES-1) {
				ASSERT(0);
				bClamp = true;
			}
			
			old_wordcount = m_WordCount;
		}

		if (bClamp) break;

		if (bNewline) {
			bNewline = false;
	
			m_pLines[m_LineCount*2]		= m_WordCount > 0 ? m_WordCount - 1 : 0;
			m_pLines[m_LineCount*2 + 1]	= 0;

			line_width = 0;
			m_LineCount++;
			if (m_LineCount >= MAX_POLYSTRING_LINES-1) {
				ASSERT(0);
				bClamp = true;
			}
			old_wordcount = m_WordCount;
		}

		if (bClamp) break;
	}

	m_pWords[m_WordCount * 3 + 1] = i-1;
	m_WordCount++;

	// last (or only) line
	m_pLines[m_LineCount*2]		= m_WordCount - 1;
	m_pLines[m_LineCount*2 + 1]	= 0;
	m_LineCount++;

	// revisit the words, and calculate pixel widths
	uint32 w, p;

	for (i=0; i<m_WordCount; i++) {
		w=0;

		for (p=m_pWords[i*3]; p<m_pWords[i*3+1]; p++) {
			w += m_pLetters[p*2+1] + bd + sp + ( em > 0 ? em : 0);
		}

		m_pWords[i*3+2] = w;
	}
}


//  ---------------------------------------------------------------------------
void CUIFormattedPolyString_Impl::ApplyXYZ()
{
	int32	lines, i;
	CUI_ALIGNMENTTYPE	align;

	float   sx,sy,sw,sh;		// screen
	float	cspw, cth;		// character-texture
	float	leftover, linewidth;

	int8	em = 0;
	int8	bd = 0;


	// sanity check -- don't bother with an empty string
	if (!m_Length) return;


	uint8*	pFontMap   = m_pFont->GetFontMap();
	uint16*	pFontTable = m_pFont->GetFontTable(); 

	// see which FX to apply
	if (m_Flags & CUI_FONT_BOLD)	bd = m_Bold;
	if (m_Flags & CUI_FONT_ITALIC)	em = m_Italic;

	cth  = (float) (m_pFont->GetCharTexHeight() + 2);

	cspw = m_pFont->GetDefSpacingH();
	
	sh = m_CharScreenHeight;

	int32 start_idx, stop_idx, start_word, line_start;
	int32 stop_word, current_word;

	// for each line
	sy = m_Y;

	start_word	 = 0;
	start_idx	 = 0;
	stop_idx	 = 0;
	line_start	 = 0;
	current_word = 0;

	for (lines=0; lines<m_LineCount; lines++) {

		// handle the newline vs. wordwrap of justified strings
		if (m_Alignment == CUI_HALIGN_JUSTIFY && !m_pLines[lines*2+1]) {
			align = CUI_HALIGN_LEFT;
		}
		else align = m_Alignment;

		// m_pWords[] = | word start | word end | pixel width |
		// m_pLines[] = | word index of line end | wrapped? |
		// m_pLetters = | table index | screen width |

		start_idx = m_pWords[start_word * 3];
		stop_idx  = m_pWords[ m_pLines[lines*2] * 3 + 1];

		// calc the width of the line
		linewidth = 0;
		for (i=start_idx; i<=stop_idx; i++) {
			linewidth += m_pLetters[i*2 + 1] + bd + cspw + ( em > 0 ? em : 0);
		}

		leftover = m_WrapWidth - linewidth;

		if (align == CUI_HALIGN_JUSTIFY) {
			stop_word = m_pLines[lines*2];
			leftover = leftover / (float)(stop_word - start_word);
		}

		sx = 0;//m_X; //0;
	
		switch (align) {
		
			// be a little bit sneaky, and reuse more code 
			case CUI_HALIGN_RIGHT:
				//sx = leftover/2;
				sx -= linewidth/2;

			case CUI_HALIGN_CENTER:
				sx -= linewidth/2;

			case CUI_HALIGN_LEFT:
			case CUI_HALIGN_JUSTIFY:
				sx += m_X;
				
				//char* pStr;

				for (i=line_start; i<=stop_idx; i++) {

					//pStr = &m_pChars[i];

					sw = m_pLetters[i*2 + 1];

					// set the polygon coords
					pDrawPrimInternal->SetXY4(&m_pPolys[i],
						sx + em,           sy,
						sx + sw + bd + em, sy,
						sx + sw + bd,      sy + sh,
						sx,                sy + sh);

					m_pPolys[i].verts[0].z = 
						m_pPolys[i].verts[1].z =
							m_pPolys[i].verts[2].z = 
								m_pPolys[i].verts[3].z = SCREEN_NEAR_Z;


					if (i >= start_idx) {
						sx += sw + bd + cspw + ( em > 0 ? em : 0);

						if (m_Alignment == CUI_HALIGN_JUSTIFY) {
							
							// we need to add a fraction of the leftover if we ended a word
							if (i>m_pWords[current_word * 3 + 1]) {
								if (align == CUI_HALIGN_JUSTIFY) {
									sx += leftover;
								}
								current_word++;
							}	
						}
					}			
				}

				if (line_start < stop_idx) current_word++;
				line_start = i;
				break;
		}

		start_word = m_pLines[lines*2] + 1;
		sy += sh;
	}
}


//  ---------------------------------------------------------------------------
void CUIFormattedPolyString_Impl::ApplyFont()
{
	uint8		cidx;
	
	int32		i; 
	uint32		texw, texh;
	uint32		tx, ty, tw, th;	


	// sanity check -- don't bother with an empty string
	if (!m_Length) return;


	HTEXTURE	pFontTex	= m_pFont->GetTexture();
	uint16*		pFontTable	= m_pFont->GetFontTable();
	uint8*		pFontMap	= m_pFont->GetFontMap();

	pTexInterface->GetTextureDims(pFontTex, texw, texh);
  	
	th = m_pFont->GetCharTexHeight() + 2;

	// the main polygon texturing loop 
	for (i=0; i<m_Length; i++) {
		
		// get the initial character index
		cidx = m_pLetters[i*2];

		switch (cidx) {
			
			case 254:  // newline
			case 255:  // space
				this->MakeBlankPoly(&m_pPolys[i]);
				break;

			default:
				
				// choose between monospace and proportional
				if ( m_pFont->IsProportional() ) {
					tw = pFontTable[(cidx) *3];
    				tx = pFontTable[(cidx) *3 + 1];
      				ty = pFontTable[(cidx) *3 + 2];	
				}
				else {
					tw = m_pFont->GetCharTexWidth() + 2;
    				tx = tw * ((cidx) % (texw/tw)) + 1;
      				ty = th * ((cidx) / (texw/tw)) + 1;
				}

				// texture coords.

				// Fonts have some extra spacing added to each letter to prevent texturing artifacts.
				// If the letters are spaced too closely together in the font bitmap, unsightly lines
				// can appear around characters on screen (this is texture blending from the adjacent
				// character).

				pDrawPrimInternal->SetUV4(&m_pPolys[i], 
						(float)(tx)			/ (float)texw, (float)(ty)		/ (float)texh,
						(float)(tx+tw-1)	/ (float)texw, (float)(ty)		/ (float)texh,
						(float)(tx+tw-1)	/ (float)texw, (float)(ty+th-1)	/ (float)texh,
						(float)(tx)			/ (float)texw, (float)(ty+th-1)	/ (float)texh);

		}
	}
}

