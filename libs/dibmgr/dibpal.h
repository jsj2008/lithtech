/****************************************************************************
;
;	 MODULE:		DIBPAL (.H)
;
;	PURPOSE:		DIB Palette Class
;
;	HISTORY:		02/18/96  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Inc.
;
****************************************************************************/


#ifndef _DIBPAL_H_
#define _DIBPAL_H_


// Structures...

typedef	struct	DIB_LOGPAL256_struct
{
	WORD			version;
	WORD			numEntries;
	PALETTEENTRY	pes[256];

} DIB_LOGPAL256;


// Classes...

class CDibPal
{
	// Member functions

public:
	CDibPal();
	~CDibPal() { Term(); }

	BOOL					Init(PALETTEENTRY* pPes, DWORD flags = 0);
	BOOL					Init(BYTE* pRgbs, DWORD flags = 0);
	BOOL					Init(RGBQUAD* pQuads, DWORD flags = 0);
	BOOL					Init(RGBTRIPLE* pTrips, DWORD flags = 0);
	BOOL					Init(const char* sFile, DWORD flags = 0);
	BOOL					Init(BYTE *pData, DWORD dataLen, int type, DWORD flags = 0);
	void					Term();

	BOOL					IsValid() { return(!!m_hPal); }

	BOOL					InitPal(const char* sFile, DWORD flags = 0);
	BOOL					InitPcx(const char* sFile, DWORD flags = 0);
	BOOL					InitBmp(const char* sFile, DWORD flags = 0);
	BOOL					InitRes(const char* sFile, DWORD flags = 0);
	
	BOOL					InitPcx(BYTE *pData, DWORD dataLen, DWORD flags = 0);

	HPALETTE				GetHandle() { return(m_hPal); }
	PALETTEENTRY*			GetPes() { return(m_logPal.pes); }
	DWORD					GetFlags() { return(m_dwFlags); }
	POSITION				GetPos() { return(m_pos); }

	void					SetPos(POSITION pos) { m_pos = pos; }

	BOOL					IsIdentity() { return(m_bIdentity); }

	static	BOOL			IsPaletteDevice();


private:
	void					MakeIdentity();

	static void				ClearSystemPalette();


	// Member variables...

private:
	HPALETTE				m_hPal;
	DIB_LOGPAL256			m_logPal;
	DWORD					m_dwFlags;
	BOOL					m_bIdentity;
	POSITION				m_pos;
};


// Inlines...

inline CDibPal::CDibPal()
{
	m_hPal      = NULL;
	m_bIdentity = FALSE;
	m_pos       = NULL;
}


// EOF...

#endif


