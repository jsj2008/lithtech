#ifndef __BITMAPFONT_H
#define __BITMAPFONT_H

class CBitmapFont
{
public:
	
	CBitmapFont();
	~CBitmapFont();

	LTBOOL			Init (ILTClient* pClientDE, char* strBitmapName);
	LTBOOL			IsValid()						{ return !!m_hFontSurface; }

	char*			GetImageName()					{ return m_strImageName; }
	HSURFACE		GetFontSurface()				{ return m_hFontSurface; }
	uint32			GetFontHeight()					{ return m_nFontHeight; }
	int				GetCharPos (int nChar)			{ if (nChar < 0 || nChar > 255) return 0; return m_nCharPos[nChar]; }
	int				GetCharWidth (int nChar)		{ if (nChar < 0 || nChar > 255) return 0; return m_nCharWidth[nChar]; }

	virtual char*	GetClassName() = 0;				// override to return font class name

protected:

	void			InitCharPositions();

	virtual void	InitCharWidths() = 0;

protected:
	
	ILTClient*	m_pClientDE;
	char		m_strImageName[256];
	HSURFACE	m_hFontSurface;
	uint32		m_nFontHeight;
	int			m_nCharPos[256];
	int			m_nCharWidth[256];
};

#endif
