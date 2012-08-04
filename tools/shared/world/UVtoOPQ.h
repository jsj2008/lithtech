#ifndef __UVTOOPQ_H__
#define __UVTOOPQ_H__

// sets up OPQs based on uv coordinates for each vertex
// takes 3 vertex positions as well as their UV coordinates
// takes an array of 6 floats, 2 per vertex for the first 3 verts plus 
// the width and height of the texture
bool ConvertUVToOPQ(	LTVector *pos, const float* coords, 
						const uint32 texWidth, const uint32 texHeight,
						LTVector& O, LTVector& P, LTVector& Q);

#endif
