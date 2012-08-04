
// This module defines some routines for cleaning up model geometry
// and precalculating data.

#ifndef __MODEL_CLEANUP_H__
#define __MODEL_CLEANUP_H__



	// Removes duplicate vertices and bad triangles.
	void gn_CleanupGeometry(Model *pModel);

	// Functions to setup the vertex and poly normals.
	BOOL gn_BuildModelVertexNormals(Model *pModel);

	// Remove unused vertices in the model.
	void gn_RemoveUnusedVertices(Model *pModel);


#endif  // __MODEL_CLEANUP_H__



