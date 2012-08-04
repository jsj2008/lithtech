//----------------------------------------------------------------------------------------------------
// LTTextureString.h
//
// Provides the implementation for the ILTTextureString interface. For more information on the
// usage and functions, see the ILTTextureString header.
//
//----------------------------------------------------------------------------------------------------
#ifndef __LTTEXTURESTRING_H__
#define __LTTEXTURESTRING_H__

#ifndef __ILTTEXTURESTRING_H__
#	include "ilttexturestring.h"
#endif

class CLTTextureString :
	public ILTTextureString
{
public:

	//interface support
	declare_interface(CLTTextureString);


	CLTTextureString();
	~CLTTextureString();

	//----------------------------
	// Custom font registration ( loads a font file that isn't already installed )
	virtual HCUSTOMFONTFILE	RegisterCustomFontFile(const char* pszRelResource);
	virtual LTRESULT		UnregisterCustomFontFile(HCUSTOMFONTFILE hCustomFontFile);


	//-----------------------------
	// String Creation
	virtual HTEXTURESTRING	CreateTextureString(const wchar_t* pszString, const CFontInfo& Font);
	virtual HTEXTURESTRING	CreateTextureSubstring(const wchar_t* pszString, HTEXTURESTRING hSrcTexture);

	//-----------------------------
	// String Formatting
	virtual LTRESULT		WordWrapString(HTEXTURESTRING hString, uint32 nWidth);
	virtual LTRESULT		RecreateTextureString(HTEXTURESTRING hString, const wchar_t* pszString, const CFontInfo& Font);
	virtual LTRESULT		RecreateTextureSubstring(HTEXTURESTRING hString, const wchar_t* pszString, HTEXTURESTRING hSrcTexture);

	//-----------------------------
	// Property Access
	virtual LTRESULT		GetStringExtents(HTEXTURESTRING hString, LTRect2n& rExtents);
	virtual LTRESULT		GetStringLength(HTEXTURESTRING hString, uint32* pnLength);
	virtual LTRESULT		GetString(HTEXTURESTRING hString, wchar_t* pszBuffer, uint32 nBufferLen);
	virtual LTRESULT		GetFont(HTEXTURESTRING hString, CFontInfo& Font);		
	virtual LTRESULT		GetTextureImage(HTEXTURESTRING hString, HTEXTURE& hTexture);
	virtual LTRESULT		GetCharRect(HTEXTURESTRING hString, uint32 nCharIndex, 
										LTRect2n& rPlacementRect,
										LTRect2n& rBlackBox,
										LTVector2f& vUVPos,
										LTVector2f& vUVDims);

	//-----------------------------
	// Reference Management
	virtual void			ReleaseTextureString(HTEXTURESTRING hString);
	virtual void			AddRefTextureString(HTEXTURESTRING hString);
	
	//-----------------------------
	// Rendering
	virtual LTRESULT		SetupTextRendering( ILTDrawPrim* pDrawPrim);
	virtual LTRESULT		RenderString(	HTEXTURESTRING hString,
											ILTDrawPrim* pDrawPrim, 
											const LTVector2f& vAnchor, 
											uint32 nColor					= 0xFFFFFFFF,
											const LTVector2f& vAnchorScale	= LTVector2f(0.0f, 0.0f), 
											const LTVector2f& vGround		= LTVector2f(1.0f, 0.0f),
											const LTVector2f& vDown			= LTVector2f(0.0f, 1.0f),
											const LTVector2f& vStretch		= LTVector2f(1.0f, 1.0f),
											bool bSnapAnchor				= true);

	virtual LTRESULT		RenderStringClipped(HTEXTURESTRING hString,
												ILTDrawPrim* pDrawPrim, 
												const LTRect2n& rClipRect,
												const LTVector2f& vAnchor, 
												uint32 nColor					= 0xFFFFFFFF,
												const LTVector2f& vAnchorScale	= LTVector2f(0.0f, 0.0f),
												bool bSnapAnchor				= true);

	virtual LTRESULT		RenderSubString(HTEXTURESTRING hString,
											const wchar_t* pszString,
											ILTDrawPrim* pDrawPrim, 
											const LTVector2f& vAnchor, 
											uint32 nColor					= 0xFFFFFFFF,
											const LTVector2f& vGround		= LTVector2f(1.0f, 0.0f),
											const LTVector2f& vDown			= LTVector2f(0.0f, 1.0f),
											const LTVector2f& vStretch		= LTVector2f(1.0f, 1.0f));

	virtual LTRESULT		RenderSubStringClipped(	HTEXTURESTRING hString,
													const wchar_t* pszString,
													ILTDrawPrim* pDrawPrim, 
													const LTRect2n& rClipRect,
													const LTVector2f& vAnchor, 
													uint32 nColor					= 0xFFFFFFFF);
};

#endif
