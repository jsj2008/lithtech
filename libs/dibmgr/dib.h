/****************************************************************************
;
;	 MODULE:		DIB (.H)
;
;	PURPOSE:		DIB (Device Independent Bitmap) Class
;
;	HISTORY:		02/18/96  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Inc.
;
****************************************************************************/


#ifndef _DIB_H_
#define _DIB_H_


// These have been removed from the engine and replaced with size specific types.
// However, they have not yet been removed from the tools sl we need to setup
// a compatability with the two usages
#undef DWORD
#define DWORD unsigned int
#undef SDWORD
#define SDWORD int
#undef WORD
#define WORD unsigned short
#undef SWORD
#define SWORD short
#undef BYTE
#define BYTE unsigned char

// Defines...

#define DIB_DT_RAW			1
#define DIB_DT_BMP			2
#define DIB_DT_PCX			3
#define DIB_DT_RID			4
#define DIB_DT_PID			5

#define IDF_RLECOMPRESSED	0x00000020
#define IDF_KEYINDEX		0x00000100


// Externs...

class CDibPal;


// Structures...

typedef struct DIB_BMI256_struct
{
	BITMAPINFOHEADER	hdr;
	RGBQUAD				colors[256];

} DIB_BMI256;

typedef struct DIB_PCXRGB_struct
{
	BYTE		red;
	BYTE		green;
	BYTE		blue;

} DIB_PCXRGB;

typedef struct DIB_PCXHDR_struct
{
	char 		manufacturer;
	char 		version;
	char 		encoding;
	BYTE		bitsPerPixel;
	short 		x0, y0, x1, y1;
	short 		xDPI, yDPI;
	DIB_PCXRGB	pal16[16];
	char 		reserved;
	char 		planes;
	short 		bytesPerLine;
	short 		paletteInfo;
	short 		xScreenSize, yScreenSize;
	char 		filler[54];

} DIB_PCXHDR;


// Classes...

class CDib
{
	// Member functions...

public:
	CDib();
	~CDib() { Term(); }

	BOOL					Init(HDC hDC, int width, int height, int depth = 8, DWORD flags = 0);
	BOOL					Init(BYTE* pBytes, HDC hDC, int width, int height, int depth = 8, DWORD flags = 0);
	BOOL					Init(BYTE* pBytes, int type, HDC hDC, DWORD flags = 0);
	BOOL					Init(const char* sFile, HDC hDC, DWORD flags = 0);
	BOOL					Init(HDC hDC, CDib *pDib, CDibPal *pPal);
	void					Term();

	BOOL					IsValid() { return(!!m_hBmp && !!m_pBytes && !!m_pLines); }

	BOOL					InitBmp(BYTE* pBytes, HDC hDC, DWORD flags = 0);
	BOOL					InitPcx(BYTE* pBytes, HDC hDC, DWORD flags = 0);
	BOOL					InitRid(BYTE* pBytes, HDC hDC, DWORD flags = 0);
	BOOL					InitPid(BYTE* pBytes, HDC hDC, DWORD flags = 0);
	BOOL					InitBmp(const char* sFile, HDC hDC, DWORD flags = 0);
	BOOL					InitPcx(const char* sFile, HDC hDC, DWORD flags = 0);
	BOOL					InitRid(const char* sFile, HDC hDC, DWORD flags = 0);
	BOOL					InitPid(const char* sFile, HDC hDC, DWORD flags = 0);
	BOOL					InitRes(const char* sFile, HDC hDC, DWORD flags = 0);

	BYTE*					GetBytes() { return(m_pBytes); }
	WORD*					GetBuf16() { return ((WORD*)m_pBytes); }
	HBITMAP					GetBitmap() { return (m_hBmp); }

	int						GetWidth() { return(m_nWidth); }
	int						GetHeight() { return(m_nHeight); }
	int						GetDepth() { return(m_nDepth); }
	int						GetPitch() { return(m_nPitch); }
	int						GetStride() { return(m_nStride); }
	BYTE					GetPixel(int x, int y) { return(m_pBytes[m_pLines[y] + x]); }
	DWORD					GetBufferSize() { return(m_nPitch * m_nHeight); }
	DWORD					GetFlags() { return(m_dwFlags); }
	DWORD					GetIndex(int y) { return(m_pLines[y]); }
	DWORD					GetIndex(int x, int y) { return(m_pLines[y] + x); }
	BYTE*					GetAddress(int y) { return(&m_pBytes[m_pLines[y]]); }
	BYTE*					GetAddress(int x, int y) { return(&m_pBytes[m_pLines[y]] + x); }
	POSITION				GetPos() { return(m_pos); }
	CDibPal*				GetPalette() { return(m_pPal); }

	void					SetPixel(int x, int y, BYTE pix) { m_pBytes[m_pLines[y] + x] = pix; }
	void					SetPos(POSITION pos) { m_pos = pos; }
	void					SetTransparent(BOOL bTrans) { m_bTransparent = bTrans; }
	void					SetPalette(CDibPal* pPal, BOOL bOwner = FALSE);

	BOOL					Resize(HDC hDC, int width, int height, int depth = 8, DWORD flags = 0);
	BOOL					Scale(int nNewWidth, int nNewHeight, int nNewDepth, DWORD flags = 0);

	void					Invert();
	void					Mirror();
	void					Mirvert() { Invert(); Mirror(); }

	int						Blt(HDC hDC) { return(Blt(hDC, 0, 0)); }
	int						Blt(HDC hDC, int xDst, int yDst);
	int						Blt(HDC hDC, int xDst, int yDst, int cxWidth, int cyHeight);
	int						Blt(CDib* pDib, int xDst, int yDst);
	int						StretchBlt(HDC hDC, int xDst, int yDst, int cxDstWidth, int cyDstHeight, 
								int xSrc, int ySrc, int cxSrcWidth, int cySrcHeight, DWORD dwROP = SRCCOPY);

	void					Fill(BYTE pix);
	void					Clear() { Fill(0); }
	void					FillRect(RECT* pRect, DWORD dwColor);
	void					FillRect(int xDest, int yDest, RECT* pSrcRect, DWORD dwColor);

	BYTE*					Lock() { return(GetBytes()); }
	void					Unlock() { }

	BOOL					IsStrideless() { return(m_nStride == 0); }
	BOOL					IsTransparent() { return(m_bTransparent); }
	BOOL					IsPaletteOwner() { return(m_bPalOwner); }

	BOOL					Save(const char* sFile, CDibPal* pPal = NULL);

private:
	BOOL					Save8(const char* sFile, CDibPal* pPal = NULL);


	// Member variables...

private:
	DIB_BMI256				m_bmi;
	HBITMAP					m_hBmp;
	BYTE*					m_pBytes;
	DWORD*					m_pLines;
	DWORD					m_dwFlags;
	int						m_nWidth;
	int						m_nHeight;
	int						m_nDepth;
	int						m_nPitch;
	int						m_nStride;
	POSITION				m_pos;
	BOOL					m_bTransparent;
	BOOL					m_bPalOwner;
	CDibPal*				m_pPal;
};


// Inlines...

inline CDib::CDib()
{
	m_hBmp      = NULL;
	m_pBytes    = NULL;
	m_pLines    = NULL;
	m_dwFlags   = 0;
	m_nWidth    = 0;
	m_nHeight   = 0;
	m_nPitch    = 0;
	m_nStride   = 0;
	m_pos       = NULL;
	m_bPalOwner = FALSE;
	m_pPal      = NULL;
}

inline int CDib::Blt(HDC hDC, int xDst, int yDst)
{
	ASSERT( (m_nDepth == 8) || (m_nDepth == 16) || (m_nDepth == 24) );
	
	if( m_nDepth == 8 )
	{
		return(StretchDIBits(hDC,
				xDst, yDst, m_nWidth, m_nHeight,
				0, 0, m_nWidth, m_nHeight,
				m_pBytes, (BITMAPINFO*)&m_bmi, DIB_PAL_COLORS, SRCCOPY));
	}
	else
	{
		return(StretchDIBits(hDC,
				xDst, yDst, m_nWidth, m_nHeight,
				0, 0, m_nWidth, m_nHeight,
				m_pBytes, (BITMAPINFO*)&m_bmi, DIB_RGB_COLORS, SRCCOPY));
	}
}

inline int CDib::Blt(HDC hDC, int xDst, int yDst, int cxWidth, int cyHeight)
{
	ASSERT( (m_nDepth == 8) || (m_nDepth == 16) || (m_nDepth == 24) );

	if( m_nDepth == 8 )
	{	
		return(StretchDIBits(hDC,
				 xDst, yDst, cxWidth, cyHeight,
				 0, 0, m_nWidth, m_nHeight,
				 m_pBytes, (BITMAPINFO*)&m_bmi, DIB_PAL_COLORS, SRCCOPY));
	}
	else
	{
		return(StretchDIBits(hDC,
				 xDst, yDst, cxWidth, cyHeight,
				 0, 0, m_nWidth, m_nHeight,
				 m_pBytes, (BITMAPINFO*)&m_bmi, DIB_RGB_COLORS, SRCCOPY));
	}
}

inline int CDib::StretchBlt(HDC hDC, int xDst, int yDst, int cxDstWidth, int cyDstHeight, 
							int xSrc, int ySrc, int cxSrcWidth, int cySrcHeight, DWORD dwROP)
{
	ASSERT( (m_nDepth == 8) || (m_nDepth == 16) || (m_nDepth == 24) );

	return(StretchDIBits(hDC,
			 xDst, yDst, cxDstWidth, cyDstHeight,
			 xSrc, ySrc, cxSrcWidth, cySrcHeight,
			 m_pBytes, (BITMAPINFO*)&m_bmi, (m_nDepth == 8) ? DIB_PAL_COLORS : DIB_RGB_COLORS, dwROP));
}


// EOF...

#endif
