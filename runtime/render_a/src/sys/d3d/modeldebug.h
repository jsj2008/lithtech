// a collection of various debug utilities for the model rendering. This allows for rendering of
// model data in various ways to get a visual representation

#ifndef __MODELDEBUG_H__
#define __MODELDEBUG_H__

//external dependencies
class ModelInstance;
class CRelevantLightList;

//container class for all the debug functions to prevent global namespace clutter
namespace NModelDebug
{
	//draws a box around the model of the size of the model dimensions
	void DrawModelBox(ModelInstance* pInstance);

	//draws all lights touching a model
	void DrawTouchingLights(const CRelevantLightList& LightList);

	//draws the skeleton of a model
	void DrawModelSkeleton(ModelInstance* pInstance);

	//draws the vertex normals of a model
	void DrawModelVertexNormals(ModelInstance* pInstance);

	//draws the obbs of a model
	void DrawModelOBBS( ModelInstance * pInstance );
};


#endif