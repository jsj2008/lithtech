
#ifndef __POLYGRID_H__
#define __POLYGRID_H__

#ifndef __DE_OBJECTS_H__
#include "de_objects.h"
#endif


// Setup a polygrid.
bool pg_Init(LTPolyGrid *pGrid, uint32 width, uint32 height, uint32 nPGFlags, bool* pValidVerts);
void pg_Term(LTPolyGrid *pGrid);

#endif  // __POLYGRID_H__


