
// This module defines the device independent model render objects...

#ifndef __GENRENOBJ_MODEL_H__
#define __GENRENOBJ_MODEL_H__

#include "renderobject.h"
#include "renderstyle.h"

// ----------------------------------------------------------------
//  base class for things that models can render
// ----------------------------------------------------------------
class CDIModelDrawable : public CRenderObject
{  
public :
	CDIModelDrawable()							{ }
	virtual ~CDIModelDrawable()					{ }

	LightingMaterial		m_Material;
    
	virtual uint32			GetVertexCount()	{ return 0; };
	virtual uint32			GetPolyCount()		{ return 0; };

	virtual bool Load(const char* szFileName)	{ return false; }
	virtual bool LoadMaterial(ILTStream & file)	{ return false; } 
};

//  ----------------------------------------------------------------
//  Render Object Handle for RigidMesh
//  ----------------------------------------------------------------
class CDIRigidMesh : public CDIModelDrawable
{
public :
	CDIRigidMesh()			{ m_Type = eRigidMesh; }
	virtual					~CDIRigidMesh() { }
};

//  ----------------------------------------------------------------
//  Render Object handle for Skeletally deformed mesh
//  ----------------------------------------------------------------
class CDISkelMesh  : public CDIModelDrawable
{
public :
	CDISkelMesh()			{ m_Type = eSkelMesh; }
	virtual					~CDISkelMesh() { }
};

//  ----------------------------------------------------------------
//  Render Object handle for Skeletally deformed mesh
//  ----------------------------------------------------------------
class CDIVAMesh  : public CDIModelDrawable
{
public :
	CDIVAMesh()				{ m_Type = eVAMesh; }
	virtual					~CDIVAMesh() { }
};

#endif // __GENRENOBJ_MODEL_H__


