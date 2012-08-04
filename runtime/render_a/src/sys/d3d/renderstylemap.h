#ifndef __RENDERSTYLEMAP_H__
#define __RENDERSTYLEMAP_H__

class CRenderStyle;

//class that handles mapping one render style to another, and if it can't find it will map to
//the default
class CRenderStyleMap
{
public:

	CRenderStyleMap();
	~CRenderStyleMap();

	//frees all of the render styles in the list along with the default
	void	FreeList();

	//adds a render style to the list of mappings
	bool	AddRenderStyle(const char* pszSrc, const char* pszMapTo);

	//set the default render style
	bool	SetDefaultRenderStyle(const char* pszFilename);

	//set the no glow render style
	bool	SetNoGlowRenderStyle(const char* pszFilename);

	//access the default render style
	CRenderStyle*	GetNoGlowRenderStyle() const;

	//given one render style, it will map it to the appropriate one
	CRenderStyle*	MapRenderStyle(const CRenderStyle* pRS) const;	

private:

	struct SMapElement
	{
		CRenderStyle* m_pSource;
		CRenderStyle* m_pMapTo;
	};

	typedef vector<SMapElement> TRSMap;

	//the map of the render styles
	TRSMap			m_Map;

	//the default render style if no other match is found
	CRenderStyle*	m_pDefault;

	//the render style to use when an object shouldn't glow
	CRenderStyle*	m_pNoGlow;

};

#endif
