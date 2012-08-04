// ----------------------------------------------------------
// renderobject.h
// lithtech (c) 2000 
//
// A CRenderObject is a device dependant object that exists in the
// renderer.
// RenderObject is a virtual base class - All RenderObjects should derive from this class.
// ----------------------------------------------------------
#ifndef __RENDEROBJECT_H__
#define __RENDEROBJECT_H__

class CRenderObject
{
public:
	enum RENDER_OBJECT_TYPES {		// List of valid Render Object Types...
		eInvalid		= 0,		// Note: If these values change - need to update the packers...
		eDebugLine		= 1,		//A standard line
		eDebugPolygon	= 2,		//A standard polygon
		eDebugText		= 3,		//A text string
		eRigidMesh		= 4,		//Rigid mesh model LOD piece
		eSkelMesh		= 5,		//Skeletally animated model LOD piece
		eVAMesh			= 6,		//Vertex animated model LOD piece
		eNullMesh		= 7			//This is for LODs that don't have any geometry associated with them
	};

protected :
    RENDER_OBJECT_TYPES m_Type;
    
public:
	// RenderObjects can be destroyed (on some platforms, anyway). This method should
	//	be used to re-load/create all the lost parts (this is basically for ALT-TAB)...
	virtual void ReCreateObject()									{ }
	virtual void FreeDeviceObjects()								{ }

	virtual void Render()											{ } // Virtual function for drawing...

	RENDER_OBJECT_TYPES GetType()									{ return m_Type; }

    // a little kludge here to exclude this stuff which isn't needed.
    CRenderObject()													{ m_pNext = NULL; }
	virtual ~CRenderObject()										{ }
    
 
	CRenderObject*	GetNext()										{ return m_pNext; }
	void			SetNext(CRenderObject* pNext)					{ m_pNext = pNext; }


private:
	// This puffs - but this is so that these things can be contained in a list (since we aren't using STL at this point)...
	CRenderObject*	m_pNext;

};

#endif 










