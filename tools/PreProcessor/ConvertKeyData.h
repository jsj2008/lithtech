#ifndef __CONVERTKEYDATA_H__
#define __CONVERTKEYDATA_H__

class CEditRegion;

// given a world, it will go through finding keyframers
// Key data for these keyframers will be packed into
// blind object data chunks, and all key objects will be
// removed from the world
bool ConvertKeyData( CEditRegion* region, CPreMainWorld* world );


#endif // __CONVERTKEYDATA_H__
