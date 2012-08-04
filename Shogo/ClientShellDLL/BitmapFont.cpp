#include "clientheaders.h"
#include "BitmapFont.h"

CBitmapFont::CBitmapFont()
{
	m_pClientDE = LTNULL;
	memset (m_strImageName, 0, 256);
	m_hFontSurface = LTNULL;
	m_nFontHeight = 0;
	memset (m_nCharPos, 0, 256 * sizeof(int));
	memset (m_nCharWidth, 0, 256 * sizeof(int));
}

CBitmapFont::~CBitmapFont()
{
	if (m_pClientDE && m_hFontSurface) m_pClientDE->DeleteSurface (m_hFontSurface);
}

LTBOOL CBitmapFont::Init (ILTClient* pClientDE, char *strBitmapName)
{
	if (!pClientDE || !strBitmapName) return LTFALSE;

	m_pClientDE = pClientDE;

	strncpy (m_strImageName, strBitmapName, 255);

	m_hFontSurface = m_pClientDE->CreateSurfaceFromBitmap (strBitmapName);
	if (!m_hFontSurface) return LTFALSE;

	uint32 nWidth = 0;
	m_pClientDE->GetSurfaceDims (m_hFontSurface, &nWidth, &m_nFontHeight);

	InitCharWidths();
	InitCharPositions();

	return LTTRUE;
}

void CBitmapFont::InitCharPositions()
{
	int nPos = 0;
	for (int i = 0; i < 256; i++)
	{
		m_nCharPos[i] = nPos;
		nPos += m_nCharWidth[i];
	}
}