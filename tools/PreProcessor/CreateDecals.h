#ifndef __CREATEDECALS_H__
#define __CREATEDECALS_H__

class CEditRegion;

//handles instantiating all of the decals. If we are importing geometry, it will also
//handle creating the geometry for the decals
bool CreateDecals(CEditRegion* pRegion, bool bCreateGeometry);

#endif