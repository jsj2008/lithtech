//------------------------------------------------------------------
//
//  FILE      : DtxViewView.h
//
//  PURPOSE   :	interface of the CDtxViewDoc class
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __DTXVIEWDOC_H__
#define __DTXVIEWDOC_H__

#pragma once

#include "TextureProp.h"



class DtxImageBuffer
{
public:

	DtxImageBuffer()
		: m_Image(NULL),
		  m_Alpha(NULL),
	      m_Width(0),
		  m_Height(0),
		  m_Bytes(0)
	{
	}

	~DtxImageBuffer()
	{
		Term();
	}

	void		Init(unsigned Width, unsigned Height);
	void		Term();

public:

	uint32*		m_Image;
	uint32*		m_Alpha;
	unsigned	m_Width;
	unsigned	m_Height;
	unsigned	m_Bytes;
};



class CDtxViewDoc : public CDocument
{
protected:

	// create from serialization only
	CDtxViewDoc();
	DECLARE_DYNCREATE(CDtxViewDoc)

public:

	virtual ~CDtxViewDoc();

	virtual BOOL 		OnNewDocument();
    virtual BOOL 		OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL 		OnSaveDocument(LPCTSTR lpszPathName);
    virtual void 		SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);

#ifdef _DEBUG
	virtual void 		AssertValid() const;
	virtual void 		Dump(CDumpContext& dc) const;
#endif

	// Is this a valid texture?
	bool				IsValidTexture() const			{ return m_TextureProp.m_pTexture != NULL; }

	// Get the texture properties as a string.
	bool				GetImageInfoString(CString *pText) const;

	// Returns the current size of the mips for resizing the display window.
	bool				GetDocumentSize(CSize *pSize) const;

	// toggle the alpha channel display
	void				ToggleViewAlphaChannel()		{ m_ViewAlphaChannel = !m_ViewAlphaChannel; }

	// toggle the mipmap level display
	void				ToggleViewMipmapLevels()		{ m_ViewMipmapLevels = !m_ViewMipmapLevels; }

	// render the scene
	void				DrawTexture(CDC *pDC);

protected:

	bool				CreateBuffers();
	void				DestroyBuffers();

protected:

	TextureProp			m_TextureProp; // texture data and settings
	DtxImageBuffer		m_ImageBuffers[MAX_DTX_MIPMAPS];

	bool				m_ViewAlphaChannel;
	bool				m_ViewMipmapLevels;

	unsigned			m_DocTotalWidth;	// total width including alpha
	unsigned			m_DocTotalHeight;	// total height including mips
	unsigned			m_DocImageWidth;	// only first image width
	unsigned			m_DocImageHeight;	// only first image height

protected:

	DECLARE_MESSAGE_MAP()
};



#endif // __DTXVIEWDOC_H__
