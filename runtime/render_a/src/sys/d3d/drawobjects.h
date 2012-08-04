
// This module controls and routes object rendering to the right functions.

#ifndef __DRAWOBJECTS_H__
#define __DRAWOBJECTS_H__

#ifndef __TAGNODES_H__
#include "tagnodes.h"
#endif

#include <queue>

struct ObjectHandler
{
	// One-time module init and term.
	void		(*m_ModuleInit)();
	void		(*m_ModuleTerm)();
	bool		m_bModuleInitted; // Tracks if the module has been initted yet.
	
	// This is called before each frame.
	void		(*m_PreFrameFn)();
	
	// This is called as objects are tagged visible.  You shouldn't DRAW
	// anything on the screen here.. just queue yourself up.
	void		(*m_ProcessObjectFn)(LTObject *pObject);

	bool		m_bCheckWorldVisibility;

	LTVector	(*m_GetDims)(LTObject *pObject);
};


extern ObjectHandler g_ObjectHandlers[NUM_OBJECTTYPES];

void d3d_InitObjectModules();
void d3d_TermObjectModules();

void d3d_InitObjectQueues();
void d3d_FlushObjectQueues(const ViewParams& Params);

// Object rendering wrapper for sorting
class ObjectDrawer
{
public:
	ObjectDrawer() {};
	ObjectDrawer(const ObjectDrawer &cOther) : 
		m_pObject(cOther.m_pObject), 
		m_pDrawFn(cOther.m_pDrawFn), 
		m_fDistance(cOther.m_fDistance) 
	{}
	ObjectDrawer &operator=(const ObjectDrawer &cOther) {
		m_pObject = cOther.m_pObject;
		m_pDrawFn = cOther.m_pDrawFn;
		m_fDistance = cOther.m_fDistance;
		return *this;
	}
	ObjectDrawer(LTObject *pObject, DrawObjectFn fn, float fDistance) : 
		m_pObject(pObject), 
		m_pDrawFn(fn),
		m_fDistance(fDistance)
	{}
	inline void Draw(const ViewParams& Params) const { m_pDrawFn(Params, m_pObject); };
	// Distance comparison
	inline bool operator<(const ObjectDrawer &cOther) const { 
		if ((m_pObject->m_Flags & FLAG_REALLYCLOSE) == (cOther.m_pObject->m_Flags & FLAG_REALLYCLOSE))
			return (m_fDistance < cOther.m_fDistance);
		else
			return (m_pObject->m_Flags & FLAG_REALLYCLOSE) > (cOther.m_pObject->m_Flags & FLAG_REALLYCLOSE);
	}

public:
	// The object pointer
	LTObject *m_pObject;
	// The function we're going to use to draw this object
	DrawObjectFn m_pDrawFn;
	// Distance to the viewer
	float m_fDistance;
};

// Mixed-Object rendering list for sorting objects in Z order
class ObjectDrawList : public CMoArray<ObjectDrawer>
{
public:
	ObjectDrawList() { };

	// Add an object to draw to the list
	void Add(const ViewParams& Params, LTObject *pObject, DrawObjectFn fn);

	// Draw the list of objects
	// Note : The list of objects is cleared by this function
	void Draw(const ViewParams& Params);

private:
	static float CalcDistance(const LTObject *pObject, const ViewParams& Params);

	typedef priority_queue<ObjectDrawer> TObjectDrawList;
	TObjectDrawList m_aObjects;
};

#endif  // __DRAWOBJECTS_H__


