#ifndef __TAGNODES_H__
#define __TAGNODES_H__

class LTPolyGrid;
class ViewParams;

#ifndef __DE_OBJECTS_H__
#include "de_objects.h"
#endif

#ifndef __COMMON_STUFF_H__
#include "common_stuff.h"
#endif

#define MAX_VISIBLE_MODELS			128
#define MAX_VISIBLE_SPRITES			128
#define MAX_VISIBLE_WORLDMODELS		64
#define MAX_TAGNODES_VISIBLE_LIGHTS	64
#define MAX_VISIBLE_POLYGRIDS		32
#define MAX_VISIBLE_LINESYSTEMS		32
#define MAX_VISIBLE_PARTICLESYSTEMS	64
#define MAX_VISIBLE_CANVASES		32
#define MAX_VISIBLE_VOLUMEEFFECTS	128

class VisibleSet;
class ObjectDrawList;

typedef void (*DrawObjectFn)(const ViewParams& Params, LTObject *pObject);

class BaseObjectSet
{
public:

				BaseObjectSet()
				{
					m_Link.m_pData = this;
					m_nObjects = 0;
					m_pObjects = NULL;
					m_pSetName = "";
					m_nMaxObjects = 0;
				}

	// Note: you don't have to call Init.. calling Init just sets the set name and
	// adds it to the VisibleSet's list.
	bool		Init(VisibleSet *pVisibleSet, const char *pSetName);

	inline bool	IsEmpty()	{return m_nObjects == 0;}

	void		Add(LTObject *pObject)
	{
		if(m_nObjects < m_nMaxObjects)
		{
			m_pObjects[m_nObjects] = pObject;
			m_nObjects++;
		}
		else
		{
			AddDebugMessage(1, "Set '%s' overflowed", m_pSetName);
		}
	}

	void		ClearSet()
	{
		m_nObjects = 0;
	}

	void		Draw(const ViewParams& Params, DrawObjectFn fn);
	void		Queue(ObjectDrawList& DrawList, const ViewParams& Params, DrawObjectFn DrawFn);

public:

	LTObject	**m_pObjects;
	uint32		m_nObjects;


protected:

	LTLink		m_Link;	// For the VisibleSet's list.
	uint32		m_nMaxObjects;
	const char	*m_pSetName;
};


template<int maxObjects>
class ObjectSet : public BaseObjectSet
{
public:

				ObjectSet()
				{
					m_nMaxObjects = maxObjects;
					m_pObjects = m_ObjectArray;
				}

public:

	LTObject		*m_ObjectArray[maxObjects];
};


class AllocSet : public BaseObjectSet
{
public:

				AllocSet();
				~AllocSet();

	bool		Init(VisibleSet *pVisibleSet, char *pSetName, uint32 defaultMax);
	void		Term();

	LTObject		**m_pArray;

};

class VisibleSet
{
public:

							VisibleSet();

	bool					Init();
	void					Term();

	void					ClearSet();

// The set of things visible from the leaf.
public:

	// The list of sets in rendering order.
	LTLink					m_Sets;

	AllocSet				m_SolidModels;
	AllocSet				m_TranslucentModels;

	AllocSet				m_TranslucentSprites;
	AllocSet				m_NoZSprites;

  	AllocSet				m_SolidWorldModels;
  	AllocSet				m_TranslucentWorldModels;
  
	AllocSet				m_Lights;

	AllocSet				m_SolidPolyGrids;
	AllocSet				m_EarlyTranslucentPolyGrids;
	AllocSet				m_TranslucentPolyGrids;

	AllocSet				m_LineSystems;

	AllocSet				m_ParticleSystems;

	AllocSet				m_SolidVolumeEffects;
	AllocSet				m_TranslucentVolumeEffects;

	AllocSet				m_SolidCanvases;
	AllocSet				m_TranslucentCanvases;
};


// ------------------------------------------------------------------------------ //
// Functions.
// ------------------------------------------------------------------------------ //

void d3d_TagVisibleLeaves(const ViewParams& Params);

VisibleSet* d3d_GetVisibleSet();

#endif  // __TAGNODES_H__



