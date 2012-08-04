#include "bdefs.h"
#include "texturestring.h"
#include "texturestringimage.h"
#include "memblockallocator.h"

//----------------------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------------------

//the memory category all of our allocations should go into 
#define MEMORY_CATEGORY			LT_MEM_TYPE_TEXTURE


//----------------------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------------------

//given a string and an index into that string that the current character is on, this
//will determine if that current character is a word break (this takes the full string
//so that it can perform multiple character checks in certain languages)
static bool CanBreakLineAt(const wchar_t* pszString, uint32 nIndex)
{
	//assume english for now
	return (iswspace(pszString[nIndex]) != 0);
}

//given a string and an index, this will determine if it is a hard line break, or a forced line break.
// (such as '\n')
static bool IsHardLineBreak(const wchar_t* pszString, uint32 nIndex)
{
	//only handle the '\n' character for now
	return (pszString[nIndex] == (wchar_t)'\n');
}

//given a string, a row ending position, and a row starting position, this will scan backwards from
//the row ending and return the first character that can be used as a line break. If no break can
//be found, it will break at the last character
static uint32 FindLineBreak(const wchar_t* pszString, uint32 nRowStart, uint32 nRowEnd)
{
	//run backwards starting at the end and find the first split, but don't bother to check
	//the first character as a split as that would cause a completely empty row
	for(int32 nCurrChar = nRowEnd - 1; nCurrChar >= (int32)nRowStart; nCurrChar--)
	{
		//see if this is a break
		if(CanBreakLineAt(pszString, nCurrChar))
			return nCurrChar;
	}
	
	//unable to find an adequate break, just break at the last character
	return nRowEnd;
}

//----------------------------------------------------------------------------------------
// CTextureString
//----------------------------------------------------------------------------------------

CTextureString::CTextureString() :
	m_pCharacters(NULL),
	m_nNumCharacters(0),
	m_pszString(NULL),
	m_rExtents(0, 0, 0, 0)
{
}

CTextureString::~CTextureString()
{
	FreeData();
}

//called to create a string given a string and an associated font
LTRESULT CTextureString::Create(const wchar_t* pszString, const CFontInfo& Font)
{
	//free any existing string data
	FreeData();

	//we need to create a new bitmap font that contains the glyphs for that string and that font
	CTextureStringImage* pNewImage = CTextureStringImage::Allocate();

	if(!pNewImage)
		return LT_OUTOFMEMORY;

	//add a reference to that object
	pNewImage->IncRef();

	//now create the bitmap font for the string
	if(!pNewImage->CreateBitmapFont(pszString, Font))
	{
		if(pNewImage)
		{
			pNewImage->DecRef();
			pNewImage = NULL;
		}

		return LT_ERROR;
	}

	//setup this object using the other object's bitmap font
	LTRESULT result = InternalCreate(pszString, pNewImage);

		//free up our reference to this image (creation already added our ref count for us
		if(pNewImage)
		{
			pNewImage->DecRef();
			pNewImage = NULL;
		}

	return result;
}

//called to create a string given a string and an existing texture string to create it from
LTRESULT CTextureString::CreateSubstring(const wchar_t* pszString, const CTextureString& CreateFrom)
{
	//free any existing string data
	FreeData();

	//setup this object using the other object's bitmap font
	return InternalCreate(pszString, CreateFrom.m_TextureImage);
}

//internal creation called once a string and a bitmap font image have been properly
//setup
LTRESULT CTextureString::InternalCreate(const wchar_t* pszString, CTextureStringImage* pTextureImage)
{
	//first off setup any string data
	if(!AllocateString(pszString))
		return LT_OUTOFMEMORY;

	// steal the texture image from the other string
	m_TextureImage = pTextureImage;
	if(m_TextureImage == NULL)
	{
		//we have a null texture image. This is bad if we have any characters
		if(m_nNumCharacters > 0)
		{
			FreeData();
			return LT_INVALIDPARAMS;
		}
		else
		{
			//we have no characters to worry about
			return LT_OK;
		}
	}

	//now run through and patch each character to the appropriate glyph, we must fail if any glyph is not
	//found
	for(uint32 nCurrChar = 0; nCurrChar < m_nNumCharacters; nCurrChar++)
	{
		//cache some information
		wchar_t cChar					= pszString[nCurrChar];
		CTextureStringChar& TexChar		= m_pCharacters[nCurrChar];

		//and now setup the texture character to an initial position and associate it with a glyph
		TexChar.m_nXPos = 0;
		TexChar.m_nYPos = 0;
		TexChar.m_pGlyph = m_TextureImage->GetGlyph(cChar);

		//check for a missing glyph
		if(TexChar.m_pGlyph == NULL)
		{
			FreeData();
			return LT_INVALIDPARAMS;
		}
	}

	//so far so good. Now just place the characters
	if(!ClearFormatting())
	{
		FreeData();
		return LT_ERROR;
	}

	return LT_OK;
}


//frees everything associated with this string. This discards all characters, image data, etc.
void CTextureString::FreeData()
{
	FreeString();

	//and release our texture image
	m_TextureImage = NULL;

	//reset our extents
	m_rExtents.Init(0, 0, 0, 0);
}

//applies word wrapping to the string so that all characters should be in the range of 0..nWidth in 
//the X axis and in the range [0...+inf] in the Y axis. This undoes any previous word wrapping or
//formatting
bool CTextureString::WordWrap(uint32 nWidth)
{
	//fail if we have no associated glyphs
	if(GetTextureImage() == NULL)
	{
		return (m_nNumCharacters == 0);
	}

	//keep track of our current position
	uint32 nCurrX = 0;
	uint32 nCurrY = 0;

	//determine the height of a single row of text
	uint32 nRowHeight = GetTextureImage()->GetRowHeight();

	//the index into the character where this current row started
	uint32 nRowStart = 0;

	//this flag indicates that the character needs to be reset. This is set when a line break
	//is encountered so that the next character can begin on a new line
	bool bResetRow = false;

	//all characters should default to being visible. Then as they break rows or 
	//other rules are applied, they can be made invisible
	uint32 nCurrChar;
	for(nCurrChar = 0; nCurrChar < m_nNumCharacters; nCurrChar++)
	{
		m_pCharacters[nCurrChar].m_bVisible = true;
	}

	//we now need to lay out each character
	for(nCurrChar = 0; nCurrChar < m_nNumCharacters; nCurrChar++)
	{
		//see if we are following a line break and need to reset the row
		if(bResetRow)
		{
			nCurrX = 0;
			nCurrY += nRowHeight;

			//our next row will start on the 
			nRowStart = nCurrChar;

			//clear the flag so it won't do it again
			bResetRow = false;
		}

		//see if this character will fit onto this row
		CTextureStringChar* pChar = &m_pCharacters[nCurrChar];
		uint32 nCurrCharWidth = pChar->m_pGlyph->m_nTotalWidth;

		//see if this is a hard line break and we have to move onto the next line
		if(IsHardLineBreak(m_pszString, nCurrChar))
		{
			//we need to reset the row
			bResetRow = true;

			//we also want to make this line break character invisible
			m_pCharacters[nCurrChar].m_bVisible = false;
		}
		//see if we are too wide (note the >= is because 0 is counted, so the limit is not included)
		//but we must also ensure that at least one character is placed per line
        else if((nCurrChar != nRowStart) && (nCurrX + nCurrCharWidth >= nWidth))
		{
			//we are too wide, lets move down. We need to determine where we can do a break by scanning
			//backwards on this row and determining where to split, but only if it isn't a hard line break
			uint32 nSplitChar = FindLineBreak(m_pszString, nRowStart + 1, nCurrChar);
			
			//update our cursor position to begin on the next row
			nCurrX = 0;
			nCurrY += nRowHeight;

			//and update our row beginning character
			nRowStart = nSplitChar + 1;

			//and we now need to move all the characters at the break onto the next line
			for(uint32 nMoveChar = nSplitChar + 1; nMoveChar < nCurrChar; nMoveChar++)
			{
				CTextureStringChar* pMoveChar = &m_pCharacters[nMoveChar];
				pMoveChar->m_nXPos = nCurrX;
				pMoveChar->m_nYPos = nCurrY;

				//move the cursor past this character
				nCurrX += pMoveChar->m_pGlyph->m_nTotalWidth;
			}
			
			//also make the split character invisible
			m_pCharacters[nSplitChar].m_bVisible = false;

		}

		//now that we know there is room, place the character
		pChar->m_nXPos = nCurrX;
		pChar->m_nYPos = nCurrY;

		//update the cursor position
		nCurrX += nCurrCharWidth;
	}

	//update our new extents
	UpdateExtents();

	//success
	return true;
}


//calling this will undo any formatting associated with the string and will place characters in
//a single row
bool CTextureString::ClearFormatting()
{
	//currently this is simple, just apply word wrapping on an infinite length row. This gives
	//us a single row of spaced text
	return WordWrap(0xFFFFFFFF);
}

//called to get information about a character. This assumes that the index is within range
void CTextureString::GetCharInfo(uint32 nIndex, int32& nX, int32& nY, bool& bVisible, const CTextureStringGlyph*& pGlyph)
{
	if ( !(nIndex < GetStringLength()))
		ASSERT(!"Error: Invalid index into the character info.");

	nX			= m_pCharacters[nIndex].m_nXPos;
	nY			= m_pCharacters[nIndex].m_nYPos;
	pGlyph		= m_pCharacters[nIndex].m_pGlyph;
	bVisible	= m_pCharacters[nIndex].m_bVisible;
}

//given a character index, this will return the string space bounding rectangle of that string
LTRect2n CTextureString::GetCharRect(uint32 nIndex)
{
	if ( !(nIndex < GetStringLength()) )
		ASSERT(!"Error: Invalid index into the character info.");
	
	LTRect2n rRect;
	rRect.Left()	= m_pCharacters[nIndex].m_nXPos;
	rRect.Top()		= m_pCharacters[nIndex].m_nYPos;
	rRect.Right()	= rRect.Left() + m_pCharacters[nIndex].m_pGlyph->m_nTotalWidth;
	rRect.Bottom()	= rRect.Top() + GetTextureImage()->GetRowHeight();

	return rRect;
}

//called to update the extents of the string
void CTextureString::UpdateExtents()
{
	//just run through and find a bounding box that encompasses all the characters
	if(m_nNumCharacters == 0)
	{
		m_rExtents.Init(0, 0, 0, 0);
		return;
	}

	//we have at least one character, so default to that character's box
	m_rExtents = GetCharRect(0);
	
	//now extend it to include all characters
	for(uint32 nCurrChar = 1; nCurrChar < m_nNumCharacters; nCurrChar++)
	{
		LTRect2n rCharRect = GetCharRect(nCurrChar);

		//expand our extents
		m_rExtents = m_rExtents.GetUnion(rCharRect);
	}
}


//called when the reference count goes to zero
void CTextureString::Free()
{
	delete this;
}

//called to allocate a string. This will allocate the characters, the string, and handle copying
//them over
bool CTextureString::AllocateString(const wchar_t* pszString)
{
	//make sure we don't already have a string allocated
	FreeString();

	if(!pszString)
		return false;

	//we now want to create a string using the other string as a base. This is done by first allocating
	//our characters, matching them to the appropriate glyphs, and then ordering them
	m_nNumCharacters = LTStrLen(pszString);

	//allocate our main memory block
	uint32 nMemBlockSize = 0;
	nMemBlockSize += AlignAllocSize(sizeof(CTextureStringChar) * m_nNumCharacters);
	nMemBlockSize += AlignAllocSize(sizeof(wchar_t) * m_nNumCharacters + 1);
	
	uint8* pMemBlock;
	LT_MEM_TRACK_ALLOC(pMemBlock = new uint8[nMemBlockSize], MEMORY_CATEGORY);
	if(!pMemBlock)
	{
		FreeString();
		return false;
	}

	CMemBlockAllocator Allocator(pMemBlock, nMemBlockSize);

	//allocate the new buffer of characters
	m_pCharacters = Allocator.AllocateObjects<CTextureStringChar>(m_nNumCharacters);
	m_pszString = Allocator.AllocateObjects<wchar_t>(m_nNumCharacters + 1);

	LTStrCpy(m_pszString, pszString, m_nNumCharacters + 1);

	//sanity check on our memory block
	if ( !(Allocator.GetAllocationOffset() == Allocator.GetBlockSize()))
		ASSERT(!"Error: Incorrect usage of texture string memory block");

	//success
	return true;
}

//frees all string data allocated by the allocate string
void CTextureString::FreeString()
{
	//our data is just in the characters, so destruct and just free that block
	DestructObjects(m_pCharacters, m_nNumCharacters);
	delete [] (uint8*)m_pCharacters;

	//and clear our fields
	m_pCharacters		= NULL;
	m_pszString			= NULL;
	m_nNumCharacters	= 0;
}

