//-----------------------------------------------------------------------------
// ClientFXVertexDeclMgr.h
// 
// Provides the definition for a manager used by the client fx system
// in order to create a single instance of vertex declarations, which 
// simplifies object creation, reduces object weight, and allows for
// faster object creation.
//
//-----------------------------------------------------------------------------

#ifndef __CLIENTFXVERTEXDECLMGR_H__
#define __CLIENTFXVERTEXDECLMGR_H__

//for HVERTEXDECL
#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif

//vertex format for a vertex that has a 3d position, single texture, color,
//and a full tangent space
struct STexTangentSpaceVert
{
	//the position of this vertex in world/object space
	LTVector	m_vPos;

	//the packed color of this vertex
	uint32		m_nPackedColor;

	//the UV coordinates of this vertex
	LTVector2	m_vUV;

	//the full tangent space of this vertex in world/object space
	LTVector	m_vNormal;
	LTVector	m_vTangent;
	LTVector	m_vBinormal;
};

class CClientFXVertexDeclMgr
{
public:

	CClientFXVertexDeclMgr();
	~CClientFXVertexDeclMgr();

	//called to initialize this object. This will return true if all vertex formats
	//were properly initialized
	bool	Init();

	//called to destroy all of the objects associated with this
	void	Term();

	//provides the vertex declaration for the textured tangent space vertex. Note that this
	//object should not be held onto, and if it is, a reference should be added to it
	HVERTEXDECL		GetTexTangentSpaceDecl()			{ return m_hTexTangentSpace; }

private:

	//the vertex declaration for the textured tangent space declaration
	HVERTEXDECL		m_hTexTangentSpace;
};

//the global manager for all of the vertex declarations. This is not a singleton because it needs
//to be known when it is creation, and created/terminated at the scope of this module
extern CClientFXVertexDeclMgr g_ClientFXVertexDecl;

#endif
