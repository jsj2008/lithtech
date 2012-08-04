//------------------------------------------------------------------
//
//  FILE      : Geometry.h
//
//  PURPOSE   : Includes all the geometry file headers and
//              defines useful stuff.
//
//  CREATED   : 1st May 1996
//
//  COPYRIGHT : Monolith Productions Inc. 1996-99
//
//------------------------------------------------------------------

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__


// Includes....
#ifndef __LTVECTOR_H__
#include "ltvector.h"
#endif

#ifndef __LTMATRIX_H__
#include "ltmatrix.h"
#endif


// Defines....
typedef CMoArray< LTVector >        CVectorArray;

// Lots of defines used all over.
#define MAX_POLYSIDE_POINTS         100
#define POINT_SIDE_EPSILON          0.1
#define INTERSECTPLANES_EPSILON     1.0e-3




#endif










