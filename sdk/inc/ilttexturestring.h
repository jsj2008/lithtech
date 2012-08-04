//----------------------------------------------------------------------------------------------------
// ILTTextureString.h
//
// Provides the definition for the ILTTextureString interface. This interface is used to create and
// manage texture strings. These texture strings are used for the creation of textures that contain
// glyphs used for rendering. For more information on the usage see the associated documentation.
//
//----------------------------------------------------------------------------------------------------

#ifndef __ILTTEXTURESTRING_H__
#define __ILTTEXTURESTRING_H__

#ifndef __LTMODULE_H__
#	include "ltmodule.h"
#endif


//for the TEXTURE INTERFACE definition
#ifndef __ILTTEXINTERFACE_H__
#	include "ilttexinterface.h"
#endif


// These mimic the wingdi.h defines not wanting to include windows.h here windows defines are without the LT prefix

#define LTANSI_CHARSET            0
#define LTDEFAULT_CHARSET         1
#define LTSYMBOL_CHARSET          2
#define LTSHIFTJIS_CHARSET        128
#define LTHANGEUL_CHARSET         129
#define LTHANGUL_CHARSET          129
#define LTGB2312_CHARSET          134
#define LTCHINESEBIG5_CHARSET     136
#define LTOEM_CHARSET             255
#if(WINVER >= 0x0400)
#define LTJOHAB_CHARSET           130
#define LTHEBREW_CHARSET          177
#define LTARABIC_CHARSET          178
#define LTGREEK_CHARSET           161
#define LTTURKISH_CHARSET         162
#define LTVIETNAMESE_CHARSET      163
#define LTTHAI_CHARSET            222
#define LTEASTEUROPE_CHARSET      238
#define LTRUSSIAN_CHARSET         204
#define LTMAC_CHARSET             77
#define LTBALTIC_CHARSET          186

#endif /* WINVER >= 0x0400 */



//forward declarations
class ILTDrawPrim;

//This will determine the size of an array assuming it has at least a single element
#define LTARRAYSIZE(a)		(sizeof(a) / (sizeof((a)[0])))


//CFontInfo
// Provides the definition for a font that can be used for creation of texture strings. 
class CFontInfo
{
public:

	//the different flags that can be set on a font
	enum	{	kStyle_Bold			= 0x01,
				kStyle_Italic		= 0x02,
			};

	//The default font constructor
	CFontInfo() :
		m_nHeight(0),
		m_nStyle(0),
		m_lfCharSet(LTDEFAULT_CHARSET)
	{
		LTStrCpy(m_szTypeface, L"", LTARRAYSIZE(m_szTypeface));
	}

	//a constructor that allows the specification of the parameters
	CFontInfo(const wchar_t * pszTypeface, uint32 nHeight, uint8 lfCharSet = LTDEFAULT_CHARSET, uint32 nStyle = 0) :
		m_nHeight(nHeight),
		m_nStyle(nStyle),
		m_lfCharSet(lfCharSet)
	{
		LTStrCpy(m_szTypeface, pszTypeface, LTARRAYSIZE(m_szTypeface));
	}

	//utility function that determines if a style flag is set on the font
	bool	IsStyleSet(uint32 nStyle) const			{ return (m_nStyle & nStyle) != 0; }

	//utility function that sets a particular style
	void	SetStyle(uint32 nStyle)					{ m_nStyle |= nStyle; }

	//utility function that clears a specified style
	void	ClearStyle(uint32 nStyle)				{ m_nStyle &= ~nStyle; }

	//the name of the typeface associated with this font. The 32 character limit actually
	//comes from windows limits that state that a font face can only be 32 characters including
	//the null terminator.
	enum	{ knMaxTypefaceLen	= 32	};
	wchar_t	m_szTypeface[knMaxTypefaceLen];

	//the height of this font in pixels. The font is guaranteed not to go higher than this
	uint32	m_nHeight;

	//the style bits of this font
	uint32	m_nStyle;

	// Character set defined in wingdi.h - default is the machines character set 
	uint8		m_lfCharSet;
};

//definition of the texture string handle which is what represents a reference
class CTextureString;
typedef CTextureString*				HTEXTURESTRING;
#define INVALID_HTEXTURESTRING		((HTEXTURESTRING)NULL)

//definition of the custom texture file handle that represents an installed custom font file
//that is a resource
class CCustomFontFile;
typedef CCustomFontFile*			HCUSTOMFONTFILE;
#define INVALID_HCUSTOMFONTFILE		((HCUSTOMFONTFILE)NULL)

//ILTTextureString, the interface for managing texture strings
class ILTTextureString :
	public IBase
{
public:

	//interface support
	interface_version(ILTTextureString, 0);

	//lifetime operators
	ILTTextureString()				{}
	virtual ~ILTTextureString()		{}

	//----------------------------
	// Custom font registration

	//given a relative resource file name, this will register the file with the font system so that
	//it can be used. This handle must be released with the corresponding function. This installed
	//font will only be accessible to this application for this run, and therefore must be done each
	//run. Any font faces within the font file are only valid after this registration and before
	//it is unregistered. This will return a NULL handle if it fails.
	virtual HCUSTOMFONTFILE	RegisterCustomFontFile(const char* pszRelResource) = 0;

	//given a handle to a custom font file, this will unregister it. No font type faces from this
	//file should be used after this is called, and the handle should not be used again.
	virtual LTRESULT		UnregisterCustomFontFile(HCUSTOMFONTFILE hCustomFontFile) = 0;

	//-----------------------------
	// String Creation

	//given a unicode string, this will create a texture string that contains all the letters
	//within the string and can be used for rendering. On failure it will return an invalid texture
	//string handle. This will use all the font parameters specified to determine the size
	virtual HTEXTURESTRING	CreateTextureString(const wchar_t* pszString, const CFontInfo& Font) = 0;
	
	//given an existing texture string this will create a new string using the original source string.
	//This will fail if the source string does not contain all the glyphs necessary to render the desired
	//string
	virtual HTEXTURESTRING	CreateTextureSubstring(const wchar_t* pszString, HTEXTURESTRING hSrcString) = 0;

	//-----------------------------
	// String Formatting

	//given a string, this will perform word wrapping on the string, breaking it at language appropriate
	//boundaries or where it must to make the text fit. This wraps only on a width and can extend to an
	//arbitrary height. If the string is modified, the word wrapping constraint will have to be reapplied
	virtual LTRESULT		WordWrapString(HTEXTURESTRING hString, uint32 nWidth) = 0;

	//given a string, this will change the text and font associated with the string. This will discard
	//any formatting data such as word wrap.
	virtual LTRESULT		RecreateTextureString(HTEXTURESTRING hString, const wchar_t* pszString, const CFontInfo& Font) = 0;

	//given a string, this will recreate it as a substring. This will lose any other formatting data such
	//as word wrap.
	virtual LTRESULT		RecreateTextureSubstring(HTEXTURESTRING hString, const wchar_t* pszString, HTEXTURESTRING hSrcTexture) = 0;

	//-----------------------------
	// Property Access

	//this fills in the rectangle that encompasses the string. This rectangle can be used as is, or
	//if the full dimensions of the string are desired, the width and height of this rectangle can
	//be used.
	virtual LTRESULT		GetStringExtents(HTEXTURESTRING hString, LTRect2n& rExtents) = 0;

	//called to access the length of the associated texture string
	virtual LTRESULT		GetStringLength(HTEXTURESTRING hString, uint32* pnLength) = 0;

	//called to access the string associated with a texture string
	virtual LTRESULT		GetString(HTEXTURESTRING hString, wchar_t* pszBuffer, uint32 nBufferLen) = 0;
	
	//called to access the font associated with a texture string
	virtual LTRESULT		GetFont(HTEXTURESTRING hString, CFontInfo& Font) = 0;		

	//called to get the actual texture associated with the font. This is useful for custom rendering of
	//strings on the world. This texture must be released through the texture management interface
	virtual LTRESULT		GetTextureImage(HTEXTURESTRING hString, HTEXTURE& hTexture) = 0;

	//called to get the positioning data associated with a character. This data includes the placement
	//rectangle which is the string space position of the character. Then there is the black box which
	//is the area that actually defines where the glyphs should be rendered and is relevant to the placement
	//rectangle. Finally the upperleft UV position is given in texture space along with the width
	//and height. This data can be used for performing custom rendering of characters
	virtual LTRESULT		GetCharRect(HTEXTURESTRING hString, uint32 nCharIndex, 
										LTRect2n& rPlacementRect,
										LTRect2n& rBlackBox,
										LTVector2f& vUVPos,
										LTVector2f& vUVDims) = 0;

	//-----------------------------
	// Reference Management

    //this will release a reference to a texture string. The passed in handle should not be used after this
	//call has been made
	virtual void			ReleaseTextureString(HTEXTURESTRING hString) = 0;
	
	//this will add a reference to a texture string. This should be done if an additional texture string
	//handle will be held onto for a long period of time or by a different object.
	virtual void			AddRefTextureString(HTEXTURESTRING hString) = 0;
	
	//-----------------------------
	// Rendering

	//given a draw prim interface, this will setup all the states to handle standard rendering of
	//texture strings.
	virtual LTRESULT		SetupTextRendering( ILTDrawPrim* pDrawPrim) = 0;

	//this will render a specified texture string using the specified point as an anchor. This anchor
	//can have a placement scale which can range from [0..1] inclusive. These scales represent the anchor
	//placement in normalized string coordinates. Therefore specifying a scale of (0, 0) will place the
	//anchor at the upper left, (.5, .5) will place the anchor at the center of the string, and so on.
	//In addition a ground vector can be specified to control the orientation of the string around that
	//anchor point. The down vector defines the Y axis around the anchor point for rendering. This will setup
	//the texture for rendering, but assumes that all other states have been properly setup on the drawprim
	//interface previously. Note that the orientation does not have to be normalized or othogonaly
	//and can therefore control scaling and shearing. The stretch vector scales each individual character
	//without changing relative positions or orientations.
	virtual LTRESULT		RenderString(	HTEXTURESTRING hString,
											ILTDrawPrim* pDrawPrim, 
											const LTVector2f& vAnchor, 
											uint32 nColor					= 0xFFFFFFFF,
											const LTVector2f& vAnchorScale	= LTVector2f(0.0f, 0.0f), 
											const LTVector2f& vGround		= LTVector2f(1.0f, 0.0f),
											const LTVector2f& vDown			= LTVector2f(0.0f, 1.0f),
											const LTVector2f& vStretch		= LTVector2f(1.0f, 1.0f),
											bool bSnapAnchor				= true) = 0;

	//this renders the specified string in a similar manner to the above function. However, it does not
	//allow for tilting, shearing, or scaling of the string and allows for the specifying of a clipping
	//rectangle that will be used to clip where the string is rendered to.
	virtual LTRESULT		RenderStringClipped(	HTEXTURESTRING hString,
													ILTDrawPrim* pDrawPrim, 
													const LTRect2n& rClipRect,
													const LTVector2f& vAnchor, 
													uint32 nColor					= 0xFFFFFFFF,
													const LTVector2f& vAnchorScale	= LTVector2f(0.0f, 0.0f),
													bool bSnapAnchor				= true) = 0;

	//this will render a sub string of the provided string. Any characters not found in the source string
	//will be skipped. This does support hard line breaks. This method of rendering sub strings is much
	//less efficient than actually creating substring objects, so if it is an infrequently changing value,
	//substring objects should be used instead. The formatting parameters are the same as the RenderString
	//function, but the anchor scale is not provided since the dimensions of this substring are not known.
	virtual LTRESULT		RenderSubString(HTEXTURESTRING hString,
											const wchar_t* pszString,
											ILTDrawPrim* pDrawPrim, 
											const LTVector2f& vAnchor, 
											uint32 nColor					= 0xFFFFFFFF,
											const LTVector2f& vGround		= LTVector2f(1.0f, 0.0f),
											const LTVector2f& vDown			= LTVector2f(0.0f, 1.0f),
											const LTVector2f& vStretch		= LTVector2f(1.0f, 1.0f)) = 0;

	//this will render a sub clipped string of the provided string. Any characters not found in the source string
	//will be skipped. This does support hard line breaks. This method of rendering sub strings is much
	//less efficient than actually creating substring objects, so if it is an infrequently changing value,
	//substring objects should be used instead. The formatting parameters are the same as the RenderStringClipped
	//function, but the anchor scale is not provided since the dimensions of this substring are not known.
	virtual LTRESULT		RenderSubStringClipped(	HTEXTURESTRING hString,
													const wchar_t* pszString,
													ILTDrawPrim* pDrawPrim, 
													const LTRect2n& rClipRect,
													const LTVector2f& vAnchor, 
													uint32 nColor				= 0xFFFFFFFF) = 0;

};

#endif
