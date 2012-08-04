//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __DIB_BUFFER_HEADER__
#define __DIB_BUFFER_HEADER__


	#include "oldtypes.h"

	class CDibImageBuffer
	{
	public:
		
		CDibImageBuffer();
		~CDibImageBuffer();
		
		BOOL	CreateSurface( DWORD width, DWORD height, DWORD depth, HDC hDC );
		BOOL	DestroySurface();

		BOOL	SetPalette( BYTE *pPalette );
		BOOL	Display( HDC hDC, CRect rect, CPoint offset );


	// Accessors.
	public:

		DWORD			Width()		{ return m_Width; }
		DWORD			Height()	{ return m_Height; }
		DWORD			Pitch()		{ return m_Pitch; }
		DWORD			Depth()		{ return m_BitDepth; }
		
		BYTE*			Buf8()		{ ASSERT( m_BitDepth==8 );  return (BYTE*)m_pBuffer; }
		WORD*			Buf16()		{ ASSERT( m_BitDepth==16 ); return (WORD*)m_pBuffer; }


	private:
		
		DWORD					m_Width;
		DWORD					m_Height;
		DWORD					m_Pitch;
		DWORD					m_BitDepth;

		BYTE					m_Palette[768];
		
		void					*m_pBuffer;
		HBITMAP					m_hBitmap;
		HPALETTE				m_hPalette, m_hOldPalette;
		HDC						m_hDC;

		struct {
			BITMAPINFOHEADER	bmiHeader;
			RGBQUAD				m_Colors[256];
		} m_BitmapInfo;
	
	};



#endif // __DIB_BUFFER_HEADER__
