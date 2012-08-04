//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// Defines world structures and functions internal to DirectEngine.

#ifndef __DE_MAINWORLD_H__
#define __DE_MAINWORLD_H__

class WorldModelInstance;
class WorldPoly;
class WorldData;
class WorldBsp;
class Node;

class PortalView;

#ifndef __WORLD_TREE_H_
#include "world_tree.h"
#endif

#ifndef __LIGHT_TABLE_H__
#include "light_table.h"
#endif

// World flags.
#define WORLD_VISED         (1<<1)  // The world has vis data.

// Used to get and set HPOLYs.
#define GET_HPOLY_INDICES(hPoly, worldIndex, polyIndex) \
{\
    (worldIndex) = (hPoly).m_nWorldIndex; \
    (polyIndex) = (hPoly).m_nPolyIndex; \
}

#define SET_HPOLY_INDICES(hPoly, worldIndex, polyIndex)\
{\
	(hPoly).m_nWorldIndex = (worldIndex); \
	(hPoly).m_nPolyIndex = (polyIndex); \
}
    

// ---------------------------------------------------------------- //
// Load/unload functions.
// ---------------------------------------------------------------- //

// Transform the specified world model.  If bPartial is TRUE, it only transforms the
// root node's center (this can be used for world models using box physics).
void w_TransformWorldModel(WorldModelInstance *pModel, LTMatrix *pTransform, bool bPartial);

Node* w_NodeForIndex(Node *pList, uint32 listSize, int index);

void w_SetPlaneTypes(Node *pNodes, uint32 nNodes, bool bUsePlaneTypes);

//----------------------------------------------------------------//
//Inline functions.
//----------------------------------------------------------------//

void w_DoLightLookup(CLightTable &pTable, const LTVector *pPos, LTRGBColor *pRGB);

#endif  // __DE_MAINWORLD_H__
