#include "precompile.h"
#include "renderstylemap.h"

#define NO_GLOW_RS_FILENAME		"rs\\noglow.ltb"

//-------------------------------------
//Interfaces

#include "iltrenderstyles.h"
static ILTRenderStyles* g_pIRenderStyles;
define_holder(ILTRenderStyles, g_pIRenderStyles);


CRenderStyleMap::CRenderStyleMap() :
	m_pDefault(NULL),
	m_pNoGlow(NULL)
{
}

CRenderStyleMap::~CRenderStyleMap()
{
	FreeList();
}

//frees all of the render styles in the list along with the default
void CRenderStyleMap::FreeList()
{
	//clean up our default
	if(m_pDefault)
	{
		g_pIRenderStyles->FreeRenderStyle(m_pDefault);
		m_pDefault = NULL;
	}

	//clean up the no glow
	if(m_pNoGlow)
	{
		g_pIRenderStyles->FreeRenderStyle(m_pNoGlow);
		m_pNoGlow = NULL;
	}

	//now remove all of our list elements
	for(TRSMap::const_iterator it = m_Map.begin(); it != m_Map.end(); it++)
	{
		g_pIRenderStyles->FreeRenderStyle(it->m_pSource);
		g_pIRenderStyles->FreeRenderStyle(it->m_pMapTo);
	}

	//free the list
	TRSMap EmptyList;
	EmptyList.swap(m_Map);
}

//adds a render style to the list of mappings
bool CRenderStyleMap::AddRenderStyle(const char* pszSrc, const char* pszMapTo)
{
	//load up both
	SMapElement Item;
	Item.m_pSource = g_pIRenderStyles->LoadRenderStyle(pszSrc);

	//make sure it worked
	if(!Item.m_pSource)
		return false;

	//load up the map to
	Item.m_pMapTo = g_pIRenderStyles->LoadRenderStyle(pszMapTo);

	//see if that worked
	if(!Item.m_pMapTo)
	{
		g_pIRenderStyles->FreeRenderStyle(Item.m_pSource);
		return false;
	}

	//success
	m_Map.push_back(Item);

	return true;
}

//set the default render style
bool CRenderStyleMap::SetDefaultRenderStyle(const char* pszFilename)
{
	//free our previous one
	if(m_pDefault)
	{
		g_pIRenderStyles->FreeRenderStyle(m_pDefault);
	}

	m_pDefault = g_pIRenderStyles->LoadRenderStyle(pszFilename);

	return (m_pDefault != NULL);
}

//set the no glow render style
bool CRenderStyleMap::SetNoGlowRenderStyle(const char* pszFilename)
{
	//free our previous one
	if(m_pNoGlow)
	{
		g_pIRenderStyles->FreeRenderStyle(m_pNoGlow);
	}

	m_pNoGlow = g_pIRenderStyles->LoadRenderStyle(pszFilename);

	return (m_pNoGlow != NULL);
}


//access the default render style
CRenderStyle* CRenderStyleMap::GetNoGlowRenderStyle() const
{
	return m_pNoGlow;
}

//given one render style, it will map it to the appropriate one
CRenderStyle* CRenderStyleMap::MapRenderStyle(const CRenderStyle* pRS) const
{
	//run through looking for a map
	for(TRSMap::const_iterator it = m_Map.begin(); it != m_Map.end(); it++)
	{
		if(it->m_pSource == pRS)
			return it->m_pMapTo;
	}

	//no match
	return m_pDefault;
}
