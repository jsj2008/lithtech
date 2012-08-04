#include "bdefs.h"
#include "lttexturestring.h"
#include "texturestring.h"
#include "iltdrawprim.h"
#include "customfontfilemgr.h"
#include "de_objects.h"

// interface database
define_interface(CLTTextureString, ILTTextureString);

// use the internal drawprim interface
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);

//-------------------------------------------------------------------------------------
// CLTTextureString
//-------------------------------------------------------------------------------------

CLTTextureString::CLTTextureString()
{
}

CLTTextureString::~CLTTextureString()
{
}

//----------------------------
// Custom font registration
HCUSTOMFONTFILE CLTTextureString::RegisterCustomFontFile(const char* pszRelResource)
{
	return (HCUSTOMFONTFILE)CCustomFontFileMgr::GetSingleton().RegisterCustomFontFile(pszRelResource);
}

LTRESULT CLTTextureString::UnregisterCustomFontFile(HCUSTOMFONTFILE hCustomFontFile)
{
	if(!CCustomFontFileMgr::GetSingleton().UnregisterCustomFontFile(hCustomFontFile))
	{
		return LT_ERROR;
	}

	return LT_OK;
}


//-----------------------------
// String Creation
HTEXTURESTRING CLTTextureString::CreateTextureString(const wchar_t* pszString, const CFontInfo& Font)
{
	//parameter check
	if(!pszString)
		return INVALID_HTEXTURESTRING;

	//allocate a new texture string
	CTextureString* pNewString;
	LT_MEM_TRACK_ALLOC(pNewString = new CTextureString, LT_MEM_TYPE_UI);

	if(!pNewString)
		return INVALID_HTEXTURESTRING;

	//setup the string
	if(pNewString->Create(pszString, Font) != LT_OK)
	{
		delete pNewString;
		return INVALID_HTEXTURESTRING;
	}

	//add a reference to it since we are passing it out of the function
	pNewString->IncRef();

	return pNewString;
}

HTEXTURESTRING CLTTextureString::CreateTextureSubstring(const wchar_t* pszString, HTEXTURESTRING hSrcString)
{
	//parameter check
	if(!pszString || !hSrcString)
		return INVALID_HTEXTURESTRING;

	//allocate a new texture string
	CTextureString* pNewString;
	LT_MEM_TRACK_ALLOC(pNewString = new CTextureString, LT_MEM_TYPE_UI);

	if(!pNewString)
		return INVALID_HTEXTURESTRING;

	//setup the string
	if(pNewString->CreateSubstring(pszString, *hSrcString) != LT_OK)
	{
		delete pNewString;
		return INVALID_HTEXTURESTRING;
	}

	//add a reference to it since we are passing it out of the function
	pNewString->IncRef();

	return pNewString;
}

//-----------------------------
// String Formatting
LTRESULT CLTTextureString::WordWrapString(HTEXTURESTRING hString, uint32 nWidth)
{
	if(!hString)
		return LT_INVALIDPARAMS;

	if(!hString->WordWrap(nWidth))
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CLTTextureString::RecreateTextureString(HTEXTURESTRING hString, const wchar_t* pszString, const CFontInfo& Font)
{
	if(!hString || !pszString)
		return LT_INVALIDPARAMS;

	return hString->Create(pszString, Font);
}

LTRESULT CLTTextureString::RecreateTextureSubstring(HTEXTURESTRING hString, const wchar_t* pszString, HTEXTURESTRING hSrcTexture)
{
	if(!hString || !pszString || !hSrcTexture)
		return LT_INVALIDPARAMS;

	return hString->CreateSubstring(pszString, *hSrcTexture);
}

//-----------------------------
// Property Access

LTRESULT CLTTextureString::GetStringExtents(HTEXTURESTRING hString, LTRect2n& rExtents)
{
	if(!hString)
		return LT_INVALIDPARAMS;

	rExtents = hString->GetExtents();
	return LT_OK;
}


LTRESULT CLTTextureString::GetStringLength(HTEXTURESTRING hString, uint32* pnLength)
{
	if(!hString || !pnLength)
		return LT_INVALIDPARAMS;

	*pnLength = hString->GetStringLength();
	return LT_OK;
}

LTRESULT CLTTextureString::GetString(HTEXTURESTRING hString, wchar_t* pszBuffer, uint32 nBufferLen)
{
	if(!hString || !pszBuffer || (nBufferLen == 0))
		return LT_INVALIDPARAMS;

	wcsncpy(pszBuffer, hString->GetString(), nBufferLen);
	pszBuffer[nBufferLen - 1] = (wchar_t)'\0';
	return LT_OK;
}

LTRESULT CLTTextureString::GetFont(HTEXTURESTRING hString, CFontInfo& Font)
{
	if(!hString || !hString->GetTextureImage())
		return LT_INVALIDPARAMS;

	Font = hString->GetTextureImage()->GetFont();
	return LT_OK;
}

LTRESULT CLTTextureString::GetTextureImage(HTEXTURESTRING hString, HTEXTURE& hTexture)
{
	if(!hString || !hString->GetTextureImage())
		return LT_INVALIDPARAMS;

	hTexture = hString->GetTextureImage()->GetTexture();

	//add a reference if it is valid
	if(hTexture)
	{
		hTexture->SetRefCount(hTexture->GetRefCount() + 1);
	}

	return LT_OK;
}

LTRESULT CLTTextureString::GetCharRect(HTEXTURESTRING hString, uint32 nCharIndex, 
										LTRect2n& rPlacementRect,
										LTRect2n& rBlackBox,
										LTVector2f& vUVPos,
										LTVector2f& vUVDims)
{
	if(!hString || (nCharIndex >= hString->GetStringLength()))
		return LT_INVALIDPARAMS;

	//get the information about the character
	int32 nX, nY;
	const CTextureStringGlyph* pGlyph;
	bool bVisible;

	hString->GetCharInfo(nCharIndex, nX, nY, bVisible, pGlyph);

	//now fill in all the structures

	//start with the placement rectangle
	rPlacementRect = hString->GetCharRect(nCharIndex);

	//now copy over the black box
	rBlackBox = pGlyph->m_rBlackBox;

	//and finally the UV values
	vUVPos.Init(pGlyph->m_fU, pGlyph->m_fV);
	vUVDims.Init(pGlyph->m_fTexWidth, pGlyph->m_fTexHeight);

	//success
	return LT_OK;
}

//-----------------------------
// Reference Management
void CLTTextureString::ReleaseTextureString(HTEXTURESTRING hString)
{
	hString->DecRef();
}

void CLTTextureString::AddRefTextureString(HTEXTURESTRING hString)
{
	if(hString)
		hString->IncRef();
}

//-----------------------------
// Rendering

//given a character, this will determine if this is a hard break character
static bool IsHardLineBreak(wchar_t cChar)
{
	//only handle the '\n' character for now
	return (cChar == (wchar_t)'\n');
}

//utility function that will calculate the anchor scale offset given a string and the anchor scale
static LTVector2f GetAnchorScaleOffset(HTEXTURESTRING hString, const LTVector2f& vAnchorScale, bool bSnapAnchor)
{
	//precalculate information about the placement of the anchor in the string space
	LTRect2n rStringExtents = hString->GetExtents();
	LTVector2f vOffset(	vAnchorScale.x * rStringExtents.GetWidth(),
						vAnchorScale.y * rStringExtents.GetHeight());

	if(bSnapAnchor)
	{
		vOffset.x = floorf(vOffset.x);
		vOffset.y = floorf(vOffset.y);
	}

	return vOffset;
}

//utility rendering function for the rendering of non-clipped strings that will fill in the provided draw
//prim polygon, given the character position, the glyph, and the appropriate alignment vectors
static void SetupStringQuad(int32 nX, int32 nY, const CTextureStringGlyph* pGlyph,
							uint32 nColor, const LTVector2f& vAnchor, 
							const LTVector2f& vAnchorScaleOffset, 
							const LTVector2f& vGround, const LTVector2f& vDown,
							const LTVector2f& vStretch,
							LT_POLYGT4& Quad)
{
	//we now need to figure out the point at which this quad needs to be placed in string space
	LTVector2f vStringSpace;
	vStringSpace.x = (float)(nX + pGlyph->m_rBlackBox.Left());
	vStringSpace.y = (float)(nY + pGlyph->m_rBlackBox.Top());

	//now we convert this into screenspace
	LTVector2f vAnchorSpace = vStringSpace - vAnchorScaleOffset;

	//rotate, and apply the final offset to get screenspace
	LTVector2f vScreenSpace = vAnchorSpace.x * vGround + vAnchorSpace.y * vDown + vAnchor;


	//we now have the upper left point of the vertex, so we now need to generate the right and
	//down vectors, which we can then use to setup the vertex positions
	// (these vectors are at half scale to identify the center position)
	LTVector2f vCharRight = vGround * (float)pGlyph->m_rBlackBox.GetWidth() * 0.5f;
	LTVector2f vCharDown  = vDown * (float)pGlyph->m_rBlackBox.GetHeight() * 0.5f;

	//find the center of the character
	LTVector2f vCenter = vScreenSpace + vCharRight + vCharDown;

	//scale the vectors based on the stretch vector
	vCharRight *= vStretch.x;
	vCharDown *= vStretch.y;
	LTVector2f vCharDiag	 = vCharRight + vCharDown;

	//fill in the vertex position

	pDrawPrimInternal->SetXY4( &Quad,
										vCenter.x - vCharDiag.x, vCenter.y - vCharDiag.y,
										vCenter.x + vCharDiag.x, vCenter.y - vCharDiag.y,
										vCenter.x + vCharDiag.x, vCenter.y + vCharDiag.y,
										vCenter.x - vCharDiag.x, vCenter.y + vCharDiag.y );
	// Put in front 
	Quad.verts[0].z = 
		Quad.verts[1].z = 
			Quad.verts[2].z = 
				Quad.verts[3].z = SCREEN_NEAR_Z;


	//setup the UV coordinates
	pDrawPrimInternal->SetUVWH(&Quad, pGlyph->m_fU, pGlyph->m_fV, pGlyph->m_fTexWidth, pGlyph->m_fTexHeight);

	//and setup the color for this character
	pDrawPrimInternal->SetRGBA( &Quad, nColor );	
	
}



//utility rendering function for the rendering of non-clipped strings that will fill in the provided draw
//prim polygon, given the character position, the glyph, and the appropriate alignment vectors
static bool SetupClippedStringQuad(	int32 nX, int32 nY, const CTextureStringGlyph* pGlyph,
									uint32 nColor, const LTVector2f& vAnchor, 
									const LTVector2f& vAnchorScaleOffset, 
									const LTRect2f& rFloatClip,
									LT_POLYGT4& Quad)
{
	//we now need to figure out the point at which this quad needs to be placed in string space
	LTVector2f vStringSpace;
	vStringSpace.x = (float)(nX + pGlyph->m_rBlackBox.Left());
	vStringSpace.y = (float)(nY + pGlyph->m_rBlackBox.Top());

	//now we convert this into screenspace
	LTVector2f vScreenSpace = vStringSpace - vAnchorScaleOffset + vAnchor;

	//form a rectangle where this is placed
	LTRect2f rChar(	vScreenSpace.x, vScreenSpace.y, 
					vScreenSpace.x + (float)pGlyph->m_rBlackBox.GetWidth(),
					vScreenSpace.y + (float)pGlyph->m_rBlackBox.GetHeight());

	//now we need to see if they overlap, if not, then just ignore this character
	if(!rChar.Overlaps(rFloatClip))
		return false;

	//find the intersection between these two rectangles
	LTRect2f rIntersect = rChar.GetIntersection(rFloatClip);

	//fill in the vertex position to be the intersection of the two rectangles
	pDrawPrimInternal->SetXY4( &Quad,
										rIntersect.Left(), rIntersect.Top(),
										rIntersect.Right(), rIntersect.Top(),
										rIntersect.Right(), rIntersect.Bottom(),
										rIntersect.Left(), rIntersect.Bottom() );
	// Put in front 
	Quad.verts[0].z = 
		Quad.verts[1].z = 
			Quad.verts[2].z = 
				Quad.verts[3].z = SCREEN_NEAR_Z;

	//determine the U and V values for the clipped vertices
	float fULeft	= pGlyph->m_fU + ((rIntersect.Left()	- rChar.Left()) / rChar.GetWidth()) * pGlyph->m_fTexWidth;
	float fURight	= pGlyph->m_fU + ((rIntersect.Right()	- rChar.Left()) / rChar.GetWidth()) * pGlyph->m_fTexWidth;
	float fVTop		= pGlyph->m_fV + ((rIntersect.Top()		- rChar.Top()) / rChar.GetHeight()) * pGlyph->m_fTexHeight;
	float fVBottom	= pGlyph->m_fV + ((rIntersect.Bottom()	- rChar.Top()) / rChar.GetHeight()) * pGlyph->m_fTexHeight;

	//setup the UV coordinates
	pDrawPrimInternal->SetUVWH(&Quad, fULeft, fVTop, fURight - fULeft, fVBottom - fVTop);


	//and setup the color for this character
	pDrawPrimInternal->SetRGBA( &Quad, nColor );

	return true;
}



LTRESULT CLTTextureString::SetupTextRendering( ILTDrawPrim* pDrawPrim )
{
	//parameter check
	if(!pDrawPrim)
		return LT_INVALIDPARAMS;

	pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	pDrawPrim->SetFillMode(DRAWPRIM_FILL);

	// use color from texture and polygon	
	pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	
	pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	return LT_OK;

}




LTRESULT CLTTextureString::RenderString(HTEXTURESTRING hString,
										ILTDrawPrim* pDrawPrim, 
										const LTVector2f& vAnchor, 
										uint32 nColor,
										const LTVector2f& vAnchorScale, 
										const LTVector2f& vGround,
										const LTVector2f& vDown,
										const LTVector2f& vStretch,
										bool bSnapAnchor)
{
	//we need to run through and render each quad as a textured quad

	//check the parameters
	if(!hString || !pDrawPrim || !hString->GetTextureImage())
		return LT_INVALIDPARAMS;

	//a buffer of quads to reduce the number of times we have to call into the draw prim
	const uint32 knNumQuads = 64;
    LT_POLYGT4	QuadBuffer[knNumQuads];

	//get the length of this string
	uint32 nNumChars = hString->GetStringLength();
	uint32 nCurrChar = 0;

	//precalculate information about the placement of the anchor in the string space
	LTVector2f vAnchorScaleOffset = GetAnchorScaleOffset(hString, vAnchorScale, bSnapAnchor);

	//setup the texture associated with this string
	HTEXTURE hTexture = hString->GetTextureImage()->GetTexture();

	//run through all characters in the string
	while(nCurrChar < nNumChars)
	{
		//fill up the quad buffer
		uint32 nUsedQuads = 0;

		for(; (nCurrChar < nNumChars) && (nUsedQuads < knNumQuads); nCurrChar++)
		{
			//build up the rectangle of where in the string this character is placed
			int32 nCharX, nCharY;
			bool bVisible;
			const CTextureStringGlyph* pGlyph;

			hString->GetCharInfo(nCurrChar, nCharX, nCharY, bVisible, pGlyph);

			//just ignore this character if it is invisible
			if(!bVisible)
				continue;

			//actually setup our quad
			SetupStringQuad(nCharX, nCharY, pGlyph, nColor, vAnchor, vAnchorScaleOffset, 
							vGround, vDown, vStretch, QuadBuffer[nUsedQuads]);			

			//move onto the next quad
			nUsedQuads++;
		}

		//we need to render those quads that got batched up
		if(nUsedQuads > 0)
		{
			//the texture is cleared after each draw prim, so set it up before each batch
			pDrawPrim->SetTexture(hTexture);

			//render the batch
			pDrawPrim->DrawPrim(QuadBuffer, nUsedQuads);

			//and make sure it is definately cleared to avoid any issues
			pDrawPrim->SetTexture(NULL);
		}
	}

	//success
	return LT_OK;
}




LTRESULT CLTTextureString::RenderStringClipped(	HTEXTURESTRING hString,
												ILTDrawPrim* pDrawPrim, 
												const LTRect2n& rClipRect,
												const LTVector2f& vAnchor, 
												uint32 nColor,
												const LTVector2f& vAnchorScale,
												bool bSnapAnchor)
{
	//check the parameters
	if(!hString || !pDrawPrim || !hString->GetTextureImage())
		return LT_INVALIDPARAMS;

	//a buffer of quads to reduce the number of times we have to call into the draw prim
	const uint32 knNumQuads = 64;
    LT_POLYGT4	QuadBuffer[knNumQuads];

	//get the length of this string
	uint32 nNumChars = hString->GetStringLength();
	uint32 nCurrChar = 0;

	//precalculate information about the placement of the anchor in the string space
	LTVector2f vAnchorScaleOffset = GetAnchorScaleOffset(hString, vAnchorScale, bSnapAnchor);

	//setup the texture associated with this string
	HTEXTURE hTexture = hString->GetTextureImage()->GetTexture();

	//convert our integer clipping rectangle to a floating point one
	LTRect2f rFloatClip((float)rClipRect.Left(), (float)rClipRect.Top(), 
						(float)rClipRect.Right(), (float)rClipRect.Bottom());

	//run through all characters in the string
	while(nCurrChar < nNumChars)
	{
		//fill up the quad buffer
		uint32 nUsedQuads = 0;

		for(; (nCurrChar < nNumChars) && (nUsedQuads < knNumQuads); nCurrChar++)
		{
			//build up the rectangle of where in the string this character is placed
			int32 nCharX, nCharY;
			bool bVisible;
			const CTextureStringGlyph* pGlyph;

			hString->GetCharInfo(nCurrChar, nCharX, nCharY, bVisible, pGlyph);

			//just skip over invisible characters
			if(!bVisible)
				continue;

			//actually setup our quad
			if(SetupClippedStringQuad(	nCharX, nCharY, pGlyph, nColor, vAnchor, vAnchorScaleOffset, 
										rFloatClip, QuadBuffer[nUsedQuads]))
			{
				//move onto the next quad
				nUsedQuads++;
			}
		}

		//we need to render those quads that got batched up
		if(nUsedQuads > 0)
		{
			//the texture is cleared after each draw prim, so set it up before each batch
			pDrawPrim->SetTexture(hTexture);

			//render the batch
			pDrawPrim->DrawPrim(QuadBuffer, nUsedQuads);

			//and make sure it is definately cleared to avoid any issues
			pDrawPrim->SetTexture(NULL);
		}
	}

	//success
	return LT_OK;
}

LTRESULT CLTTextureString::RenderSubString(	HTEXTURESTRING hString,
											const wchar_t* pszSubString,
											ILTDrawPrim* pDrawPrim, 
											const LTVector2f& vAnchor, 
											uint32 nColor,
											const LTVector2f& vGround,
											const LTVector2f& vDown,
											const LTVector2f& vStretch)
{
	//we need to run through and render each quad as a textured quad

	//check the parameters
	if(!hString || !pDrawPrim || !hString->GetTextureImage())
		return LT_INVALIDPARAMS;

	//a buffer of quads to reduce the number of times we have to call into the draw prim
	const uint32 knNumQuads = 64;
	LT_POLYGT4	QuadBuffer[knNumQuads];

	//get the length of this string
	uint32 nNumChars = LTStrLen(pszSubString);
	uint32 nCurrChar = 0;

	//setup the texture associated with this string
	HTEXTURE hTexture = hString->GetTextureImage()->GetTexture();

	//the current position
	int32 nCurrX = 0;
	int32 nCurrY = 0;

	//run through all characters in the string
	while(nCurrChar < nNumChars)
	{
		//fill up the quad buffer
		uint32 nUsedQuads = 0;

		for(; (nCurrChar < nNumChars) && (nUsedQuads < knNumQuads); nCurrChar++)
		{
			//handle hard line breaks
			if(IsHardLineBreak(pszSubString[nCurrChar]))
			{
				//just reset the X and move down a line
				nCurrX = 0;
				nCurrY += hString->GetTextureImage()->GetRowHeight();
				continue;
			}

			//find our appropriate glyph
			const CTextureStringGlyph* pGlyph = hString->GetTextureImage()->GetGlyph(pszSubString[nCurrChar]);

			//bail if we didn't find it
			if(!pGlyph)
				continue;

			//actually setup our quad
			SetupStringQuad(nCurrX, nCurrY, pGlyph, nColor, vAnchor, LTVector2f(0.0f, 0.0f), 
							vGround, vDown, vStretch, QuadBuffer[nUsedQuads]);			

			//move onto our next character
			nCurrX += pGlyph->m_nTotalWidth;

			//move onto the next quad
			nUsedQuads++;
		}

		//we need to render those quads that got batched up
		if(nUsedQuads > 0)
		{
			//the texture is cleared after each draw prim, so set it up before each batch
			pDrawPrim->SetTexture(hTexture);

			//render the batch
			pDrawPrim->DrawPrim(QuadBuffer, nUsedQuads);

			//and make sure it is definately cleared to avoid any issues
			pDrawPrim->SetTexture(NULL);
		}
	}

	//success
	return LT_OK;
}

LTRESULT CLTTextureString::RenderSubStringClipped(	HTEXTURESTRING hString,
													const wchar_t* pszSubString,
													ILTDrawPrim* pDrawPrim, 
													const LTRect2n& rClipRect,
													const LTVector2f& vAnchor, 
													uint32 nColor)
{
	//check the parameters
	if(!hString || !pDrawPrim || !hString->GetTextureImage())
		return LT_INVALIDPARAMS;

	//a buffer of quads to reduce the number of times we have to call into the draw prim
	const uint32 knNumQuads = 64;
	LT_POLYGT4	QuadBuffer[knNumQuads];

	//get the length of this string
	uint32 nNumChars = LTStrLen(pszSubString);
	uint32 nCurrChar = 0;

	//setup the texture associated with this string
	HTEXTURE hTexture = hString->GetTextureImage()->GetTexture();

	//convert our integer clipping rectangle to a floating point one
	LTRect2f rFloatClip((float)rClipRect.Left(), (float)rClipRect.Top(), 
						(float)rClipRect.Right(), (float)rClipRect.Bottom());

	//the current position
	int32 nCurrX = 0;
	int32 nCurrY = 0;

	//run through all characters in the string
	while(nCurrChar < nNumChars)
	{
		//fill up the quad buffer
		uint32 nUsedQuads = 0;

		for(; (nCurrChar < nNumChars) && (nUsedQuads < knNumQuads); nCurrChar++)
		{
			//handle hard line breaks
			if(IsHardLineBreak(pszSubString[nCurrChar]))
			{
				//just reset the X and move down a line
				nCurrX = 0;
				nCurrY += hString->GetTextureImage()->GetRowHeight();
				continue;
			}

			//find our appropriate glyph
			const CTextureStringGlyph* pGlyph = hString->GetTextureImage()->GetGlyph(pszSubString[nCurrChar]);

			//bail if we didn't find it
			if(!pGlyph)
				continue;

			//actually setup our quad
			bool bVisible = SetupClippedStringQuad(	nCurrX, nCurrY, pGlyph, nColor, vAnchor, LTVector2f(0.0f, 0.0f), 
													rFloatClip, QuadBuffer[nUsedQuads]);

			//advance our current position
			nCurrX += pGlyph->m_nTotalWidth;

			//advance to the next quad if appropriate
			if(bVisible)
			{
				//move onto the next quad
				nUsedQuads++;
			}
		}

		//we need to render those quads that got batched up
		if(nUsedQuads > 0)
		{
			//the texture is cleared after each draw prim, so set it up before each batch
			pDrawPrim->SetTexture(hTexture);

			//render the batch
			pDrawPrim->DrawPrim(QuadBuffer, nUsedQuads);

			//and make sure it is definately cleared to avoid any issues
			pDrawPrim->SetTexture(NULL);
		}
	}

	//success
	return LT_OK;
}