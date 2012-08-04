//-------------------------------------------------------------------
//
//   MODULE    : CUIPOLYSTRING_IMPL.CPP
//
//   PURPOSE   : implements the CUIPolyString_Impl bridge Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPOLYSTRING_IMPL_H__
#include "cuipolystring_impl.h"
#endif

#ifndef __CUIRENDERSTATE_H__
#include "cuirenderstate.h"
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

//the default size of the poly buffer used for clipping
#define CLIP_BUFFER_LEN 512

//  ---------------------------------------------------------------------------
CUIPolyString_Impl::CUIPolyString_Impl()
{
	
}


//  ---------------------------------------------------------------------------
CUIPolyString_Impl::CUIPolyString_Impl(CUIFont* pFont, 
							 const char* pBuf,
							 float x,
							 float y)
{
	m_Valid		= false;
	m_pFont		= pFont;
	m_pChars	= NULL;
	m_pPolys	= NULL;	

	m_Flags		= 0;
	m_Bold		= 0;
	m_Italic	= 0;

	m_Length	= 0;
	
	m_NumAllocatedChars = 0;
	m_NumAllocatedPolys = 0;
	
	// if there's no font, we can't do much!  String is invalid.
	if (!pFont) return;
	m_CharScreenWidth  = pFont->GetDefCharScreenWidth();
	m_CharScreenHeight = pFont->GetDefCharScreenHeight();

	memcpy(&m_pColors, pFont->GetDefColors(), sizeof(uint32) * 4);

	m_X = x;
	m_Y = y;

	memset(&m_Rect, 0, sizeof(CUIRECT));

	// must be valid before we call settext!
	m_Valid = true;

	if (pBuf) {
		this->SetText(pBuf);	
	}	
}


//  ---------------------------------------------------------------------------
CUIPolyString_Impl::~CUIPolyString_Impl() 
{
	if (m_pChars) delete [] m_pChars;
	m_pChars = NULL;
	if (m_pPolys) delete [] m_pPolys;	
	m_pPolys = NULL;

	m_Valid = false;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::SetPosition(float x, float y)
{
	// apply the new coords to the polygons
	int32 i, v;

	if (!m_Valid) return CUIR_INVALID_POLYSTRING;
	
	float dx = x - m_X;
	float dy = y - m_Y;
		
	for (i=0; i<m_Length; i++) {
		for (v=0; v<4; v++) {
			m_pPolys[i].verts[v].x += dx;
			m_pPolys[i].verts[v].y += dy;
		}
	}
	
	m_X = x;
	m_Y = y;

	GetExtents(&m_Rect);
	
	return CUIR_OK;
}	
	

//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::GetRect(CUIRECT* rect)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	rect->x 	 = m_Rect.x;
	rect->y 	 = m_Rect.y;
	rect->height = m_Rect.height;
	rect->width  = m_Rect.width;
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::GetPosition(float* x, float* y)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	*x = m_X;
	*y = m_Y;
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::GetDims(float* width, float* height)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	*width  = m_Rect.width;
	*height = m_Rect.height;
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString_Impl::SetCharScreenWidth(uint8 width)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	m_CharScreenWidth  = width;

	this->ApplyFont(m_pFont, 0, m_Length, true);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString_Impl::SetCharScreenHeight(uint8 height)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	m_CharScreenHeight = height;

	this->ApplyFont(m_pFont, 0, m_Length, true);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString_Impl::SetCharScreenSize(uint8 height, uint8 width)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	m_CharScreenHeight = height;
	m_CharScreenWidth  = width;

	this->ApplyFont(m_pFont, 0, m_Length, true);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::SetColor(uint32 argb)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	m_pColors[0] = 
		m_pColors[1] = 
			m_pColors[2] = 
				m_pColors[3] = argb;
	
	this->ApplyColors();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::SetColors(uint32 argb0, uint32 argb1,
											  uint32 argb2, uint32 argb3)
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	m_pColors[0] = argb0; 
	m_pColors[1] = argb1;
	m_pColors[2] = argb2;
	m_pColors[3] = argb3;
	
	this->ApplyColors();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void	CUIPolyString_Impl::ApplyColors()
{
	uint32 i,v;

	for (i=0; i<m_Length; i++) {
		for (v=0; v<4; v++) {
			m_pPolys[i].verts[v].rgba.b = (m_pColors[v])       & 0xFF;
			m_pPolys[i].verts[v].rgba.g = (m_pColors[v] >> 8)  & 0xFF;
			m_pPolys[i].verts[v].rgba.r = (m_pColors[v] >> 16) & 0xFF;
			m_pPolys[i].verts[v].rgba.a = (m_pColors[v] >> 24) & 0xFF;
		}
	}
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::SetText(const char* buf)
{
	// make sure we have a font pointer
	if (!m_pFont) {
		return CUIR_NO_FONT;
	}
	// and a non-empty character buffer
	if (!buf) {
		return CUIR_ERROR;
	}
	// is the polystring itself valid?
	if (!m_Valid) {
		return CUIR_INVALID_POLYSTRING;
	}

	m_Length = strlen(buf);
	
	// allocate the characters if they don't exist or the buffer
	// ain't big enough
	if (!m_pChars || m_NumAllocatedChars < m_Length + (uint32)1) {
		if (m_pChars)
			delete [] m_pChars;
		LT_MEM_TRACK_ALLOC(m_pChars = new char[m_Length+1],LT_MEM_TYPE_UI);	// str + NULL
		m_NumAllocatedChars = m_Length+1;
	}

	if (m_Length && !m_pChars) {
		CUI_ERR("Out of Memory\n");
		m_NumAllocatedChars = 0;
		return CUIR_OUT_OF_MEMORY;
	}

	strcpy(m_pChars, buf);

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
	this->ApplyFont(m_pFont, 0, m_Length, true);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void CUIPolyString_Impl::MapCharacter(CUIFont* pFont,
									  uint8 cidx,
									  LT_POLYGT4* pPoly,
									  int32 *w, 
									  uint32 texw, 
									  uint32 texh)
{
	//uint32* pColors		= pFont->GetDefColors();
	uint16*  pFontTable	= pFont->GetFontTable();
	uint32	h			= m_CharScreenHeight;//pFont->GetCharScreenHeight();

	uint32 tx, ty, tw, th;	

	th = pFont->GetCharTexHeight() + 2;
	
	// choose between monospace and proportional
	if ( pFont->IsProportional() ) {
		tw = pFontTable[(cidx) *3];
    	tx = pFontTable[(cidx) *3 + 1];
      	ty = pFontTable[(cidx) *3 + 2];	
		*w = tw * h / th;
	}
	else {
		tw = pFont->GetCharTexWidth() + 2;
    	tx = tw * ((cidx) % (texw/tw)) + 1;
      	ty = th * ((cidx) / (texw/tw)) + 1;
	}

	// vertex colors			
	
	// This was commented out, but it caused the console to fail drawing
	//  As a temporary fix I uncommented it. AdamS should determine the more
	//  proper fix. JpB
	
	
	pDrawPrimInternal->SetRGBA4(pPoly, 
			m_pColors[0],
			m_pColors[1],
			m_pColors[2],
			m_pColors[3]);
	/*
	
	pDrawPrimInternal->SetRGBA4(pPoly, 
			pColors[0],
			pColors[1],
			pColors[2],
			pColors[3]);
	*/
				
	// texture coords.

	// Fonts have some extra spacing added to each letter to prevent texturing artifacts.
	// If the letters are spaced too closely together in the font bitmap, unsightly lines
	// can appear around characters on screen (this is texture blending from the adjacent
	// character).

	pDrawPrimInternal->SetUV4(pPoly, 
			(float)(tx)			/ (float)texw, (float)(ty)		/ (float)texh,
			(float)(tx+tw-1)	/ (float)texw, (float)(ty)		/ (float)texh,
			(float)(tx+tw-1)	/ (float)texw, (float)(ty+th-1)	/ (float)texh,
			(float)(tx)			/ (float)texw, (float)(ty+th-1)	/ (float)texh);

}


//  ---------------------------------------------------------------------------
void CUIPolyString_Impl::MakeBlankPoly(LT_POLYGT4* pPoly)
{
	// tex coords.
	pDrawPrimInternal->SetUVWH(pPoly, 
			0, 0,
			0, 0);
	
	// vertex colors			
	pDrawPrimInternal->SetRGBA(pPoly, 0x00000000);
}


//  ---------------------------------------------------------------------------
void CUIPolyString_Impl::SetPolyXYZ(LT_POLYGT4* pPoly, float cx, float cy, 
									int32 w, int32 h, float em, float bold)
{
	// polygon coords.
	pDrawPrimInternal->SetXY4(pPoly, 
			cx+em,				 cy,
			cx+em+(float)w+bold, cy,
			cx+(float)w+bold,    cy+(float)h,
			cx,					 cy+(float)h);

	// place the text at the front of the view
	pPoly->verts[0].z = 
		pPoly->verts[1].z = 
			pPoly->verts[2].z = 
				pPoly->verts[3].z = SCREEN_NEAR_Z;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString_Impl::SetFont(CUIFont* pFont) 
{
	if (!m_Valid) return CUIR_INVALID_POLYSTRING;

	m_pFont = pFont;
	return this->ApplyFont(pFont, 0, m_Length, false);
}


//	---------------------------------------------------------------------------
//	---------------------------------------------------------------------------
//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString_Impl::ApplyFont(
	CUIFont* pFont, 
	int16 index, 
	int16 num,
	bool  bProcessRemainder)
{
	uint8		cidx;
	uint8		*pFontMap;

	int32		i, len, storedWidth; 
	int32		w,h;
	int32		delta;

	uint32		texturew, textureh;
	uint32		fontFlags;

	bool 		draw,drawblank;

	float		pstrx, pstry;
  	float		cx, cy;
	float		spacing, em, bold;

	const char* str;
	
	HTEXTURE	pFontTex;

	LT_POLYGT4* pArray;

	// sanity checks
	if (!pDrawPrimInternal) return CUIR_ILT_DRAWPRIM_ERROR;
	if (!pFont) return CUIR_NO_FONT;

	if (pFont->IsProportional() && !pFont->GetFontTable())
		return CUIR_INVALID_FONT;

	pFontTex		= pFont->GetTexture();
	storedWidth		= m_CharScreenWidth;//pFont->GetCharScreenWidth();
	w				= storedWidth;					// padding
  	h				= m_CharScreenHeight;//pFont->GetCharScreenHeight(); // padding
	pFontMap		= pFont->GetFontMap();
  	spacing			= pFont->GetDefSpacingH();

	pTexInterface->GetTextureDims(pFontTex, texturew, textureh);
  
	// get some coords
	this->GetPosition(&cx, &cy);
	pstrx = cx;
	pstry = cy;
	
	// set flag attrs.
	fontFlags = pFont->GetAttributes();
	em   = 0;
	bold = 0;
	if (fontFlags & CUI_FONT_ITALIC) em    =	pFont->GetDefSlant();
	if (fontFlags & CUI_FONT_BOLD )  bold  =	pFont->GetDefBold();
	
	// are we applying to all of the string or only part?
  	len = m_Length;
	if (num == 0) num = (uint16)len;
	num = (index + num > len) ? (len) : (index+num);
	
	str      = m_pChars + index;
	pArray	 = m_pPolys;	

	// place this character next to the previous one
	if (index > 0) cx = (int16)(pArray[index-1].verts[1].x + spacing);		
	
	// the main polygon texturing loop 
  	for (i=index; i<num; i++) {

		draw		= true;
		drawblank	= false;

		// apply the font map(if any)
		//if (pFontMap) cidx = pFontMap[*str];
		//else		  cidx = (*str);
		cidx = (*str);

		// handle chars that shouldn't be subtracted (newlines, tabs, etc.)
		switch (cidx) {
			
			case 10:
				draw = false;
				break;

			case 32:
				drawblank = true;
				break;

			default:

				if (!pFontMap) {
					// if there's no fontmap, turn ascii into indices
					//cidx = (*str);
					cidx -= 33;
					if (cidx > 93) draw = false;
				}	
				else {
					cidx = pFontMap[cidx];
				}
		}
		
		if (!draw || drawblank) {			
			// draw a blank, transparent poly for a space
			w = storedWidth;
			this->MakeBlankPoly(&pArray[i]);
		}
		
		if (draw && !drawblank) {
			// set the UV coords
			MapCharacter(pFont, cidx, &pArray[i], &w, texturew, textureh);
		}
		else {
			if (!drawblank) w = 0 - (int16)(bold + spacing);
		}
    	
		// apply the new XYZ coords
		SetPolyXYZ(&pArray[i], cx, cy, w, h, em, bold);

		// increment the x position for the next char.
		cx = cx + int16(w + bold + spacing);			

		// increment our string pointer
    	str++;
  	}  	
	
	// move the polys we did not process so there won't be gaps or overlaps
	// in the string

	if (bProcessRemainder) {	
		if (num<len) {			
			delta = (int32) (pArray[num-1].verts[1].x - pArray[num].verts[0].x);
			
			for (i=num; i<len; i++) {
				pArray[i].verts[0].x += delta;
				pArray[i].verts[1].x += delta;
				pArray[i].verts[2].x += delta;
				pArray[i].verts[3].x += delta;
			}
		}
	}

	// get the extents of the new polystring
	GetExtents(&m_Rect);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::Render(int32 start, int32 end)
{
	if (!m_pPolys) return CUIR_ERROR;
	if (!m_Valid)  return CUIR_INVALID_POLYSTRING;

	//no character so nothing to draw
	if (!m_Length)  return CUIR_OK;
	
	// set up the render state
	CUIRenderState::SetRenderState(m_pFont->GetTexture());
	//this->SetRenderState(m_pFont->GetTexture());
		
	// draw our string

	if (end < 0)
		end = m_Length;

	int32 len = (end - start);
	

	pDrawPrimInternal->DrawPrim(&m_pPolys[start], len);
	

	return CUIR_OK;
}



//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::RenderClipped(CUIRECT* clip, 
										  int32 start, 
										  int32 end)
{
	// cull out polygons that are completely outside the clip rect, and clip
	// the polys that are straddling the rect border
	int drawPolys = 0;

	float minx,maxx,miny,maxy;
	float minu,maxu,minv,maxv;

	int32 i, v;//, startidx, endidx;

	//no character so nothing to draw
	if (!m_Length)  return CUIR_OK;

	if (!m_pPolys)	return CUIR_ERROR;
	if (!clip)		return CUIR_INVALID_POINTER;
	if (!m_Valid)	return CUIR_INVALID_POLYSTRING;


	//allocate a buffer on the stack to avoid dynamic allocations
	LT_POLYGT4 newPolys[CLIP_BUFFER_LEN];
	LT_POLYGT4* pNewPolys = newPolys;
	bool bDynamicBuffer = false;

	int32 len = (end>=0) ? end : m_Length;
	
	for (i=start; i<len; i++) {

		// find the extents of the poly
		// we do this because there's no guarantee that v0 is the 'upper left'
		// since the user can muck about with the polygons all they like for FX
		// and it reduces the number of tests we do later

		minx = m_pPolys[i].verts[0].x;
		maxx = m_pPolys[i].verts[0].x;
		miny = m_pPolys[i].verts[0].y;
		maxy = m_pPolys[i].verts[0].y;

		for (v=1; v<4; v++) {
			if (m_pPolys[i].verts[v].x > maxx) maxx = m_pPolys[i].verts[v].x;
			if (m_pPolys[i].verts[v].x < minx) minx = m_pPolys[i].verts[v].x;
			if (m_pPolys[i].verts[v].y > maxy) maxy = m_pPolys[i].verts[v].y;
			if (m_pPolys[i].verts[v].y < miny) miny = m_pPolys[i].verts[v].y;
		}

		// ok, now about that clipping...

		// case 0:  the poly is completely inside the cliprect
		if ( minx >= clip->x               &&
			 maxx <= clip->x + clip->width &&
			 miny >= clip->y               &&
			 maxy <= clip->y + clip->height ) {

			if (drawPolys >= CLIP_BUFFER_LEN && !bDynamicBuffer)
			{
				ASSERT(drawPolys < CLIP_BUFFER_LEN);

				//not enough room in buffer so allocate a new one dynamically
				bDynamicBuffer = LTTRUE;
				LT_MEM_TRACK_ALLOC(pNewPolys = new LT_POLYGT4[m_Length],LT_MEM_TYPE_UI);
				if (!pNewPolys) return CUIR_OUT_OF_MEMORY;

				//copy from the stack to the newly allocated buffer
				memcpy(pNewPolys, &newPolys, sizeof(LT_POLYGT4) * drawPolys);
				
			}
			memcpy(&pNewPolys[drawPolys], &m_pPolys[i], sizeof(LT_POLYGT4));
			drawPolys++;
			continue;
		}
		
		// case 1:  the poly is completely outside the cliprect
		if ( minx > clip->x + clip->width  ||
			 maxx < clip->x                ||
			 miny > clip->y + clip->height ||
			 maxy < clip->y  ) {

			continue;
		}

		// case 2:  anything left is the tricky case, 
		//			where the poly straddles the border

		if (drawPolys >= CLIP_BUFFER_LEN && !bDynamicBuffer)
		{
			ASSERT(drawPolys < CLIP_BUFFER_LEN);
			
			//not enough room in buffer so allocate a new one dynamically
			bDynamicBuffer = LTTRUE;
			LT_MEM_TRACK_ALLOC(pNewPolys = new LT_POLYGT4[m_Length],LT_MEM_TYPE_UI);
			if (!pNewPolys) return CUIR_OUT_OF_MEMORY;

			//copy from the stack to the newly allocated buffer
			memcpy(pNewPolys, &newPolys, sizeof(LT_POLYGT4) * drawPolys);
			
		}

		// find the u,v extents
		minu = m_pPolys[i].verts[0].u;
		maxu = m_pPolys[i].verts[0].u;
		minv = m_pPolys[i].verts[0].v;
		maxv = m_pPolys[i].verts[0].v;

		for (v=1; v<4; v++) {
			if (m_pPolys[i].verts[v].u > maxu) maxu = m_pPolys[i].verts[v].u;
			if (m_pPolys[i].verts[v].u < minu) minu = m_pPolys[i].verts[v].u;
			if (m_pPolys[i].verts[v].v > maxv) maxv = m_pPolys[i].verts[v].v;
			if (m_pPolys[i].verts[v].v < minv) minv = m_pPolys[i].verts[v].v;
		}

		// add the poly
		memcpy(&pNewPolys[drawPolys], &m_pPolys[i], sizeof(LT_POLYGT4));

		// clip the xy/uv coords
		for (v=0; v<4; v++) {

			if (pNewPolys[drawPolys].verts[v].x < clip->x) {
				// fix the u
				pNewPolys[drawPolys].verts[v].u += (maxu-minu) *
					(clip->x - pNewPolys[drawPolys].verts[v].x) / 
					(maxx - minx);  
				// fix the x
				pNewPolys[drawPolys].verts[v].x = clip->x;
			}

			if (pNewPolys[drawPolys].verts[v].x > clip->x + clip->width) {
				// fix the u
				pNewPolys[drawPolys].verts[v].u -= (maxu-minu) *
					(pNewPolys[drawPolys].verts[v].x - (clip->x + clip->width)) /
					(maxx-minx);  
				// fix the x
				pNewPolys[drawPolys].verts[v].x = clip->x + clip->width;
			}

			if (pNewPolys[drawPolys].verts[v].y < clip->y) {
				// fix the v
				pNewPolys[drawPolys].verts[v].v += (maxv-minv) *
					(clip->y - pNewPolys[drawPolys].verts[v].y) / 
					(maxy - miny);  
				// fix the y
				pNewPolys[drawPolys].verts[v].y = clip->y;
			}

			if (pNewPolys[drawPolys].verts[v].y > clip->y + clip->height) {
				// fix the v
				pNewPolys[drawPolys].verts[v].v -= (maxv-minv) *
					(pNewPolys[drawPolys].verts[v].y - (clip->y + clip->height)) /
					(maxy-miny);  
				// fix the y
				pNewPolys[drawPolys].verts[v].y = clip->y + clip->height;
			}
		}

		drawPolys++;
	}
				
	// don't call drawprim unless we have polys to draw!!
	if (drawPolys) {	
		// set up the render state
		CUIRenderState::SetRenderState(m_pFont->GetTexture());
		//this->SetRenderState(m_pFont->GetTexture());
			
		// draw our clipped string
		pDrawPrimInternal->DrawPrim(pNewPolys, drawPolys);
	}
	
	// clean up after ourselves!
	if (bDynamicBuffer) 
		delete [] pNewPolys;

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString_Impl::GetExtents(CUIRECT* pRect)
{
	int32 i,v;

	if (!pRect)		return CUIR_INVALID_POINTER;
	if (!m_Valid)	return CUIR_INVALID_POLYSTRING;

	float minx = 0;
	float maxx = 0;
	float miny = 0;
	float maxy = 0;

	if (m_pPolys) {
		
		minx = maxx = m_pPolys[0].verts[0].x;
		miny = maxy = m_pPolys[0].verts[0].y;

		for (i=0; i<m_Length; i++) {
			
			for (v=0; v<4; v++) {
				if (m_pPolys[i].verts[v].x > maxx) {
					maxx = m_pPolys[i].verts[v].x;
				}
				if (m_pPolys[i].verts[v].x < minx) {
					minx = m_pPolys[i].verts[v].x;
				}
				if (m_pPolys[i].verts[v].y > maxy) {
					maxy = m_pPolys[i].verts[v].y;
				}
				if (m_pPolys[i].verts[v].y < miny) {
					miny = m_pPolys[i].verts[v].y;
				}
			}
		}
	}

	pRect->x	  = minx;
	pRect->y	  = miny;
	pRect->width  = maxx - minx;
	pRect->height = maxy - miny;

	return CUIR_OK;
}


