#ifndef __WORLD_SERVER_H__
#define __WORLD_SERVER_H__

#ifndef __WORLD_INTERFACE_H__
#include "world_interface.h"
#endif

//
//Generic world interface used by engine server code.
//

class IWorldServer : public IWorld {
  public:
    interface_version_derived(IWorldServer, IWorld, 0);

    //
    //Server side object managment code might be put here.
    //


    //returns true if the given position is outside the world.
    virtual bool IsOutsideWorld(const LTVector &position) = 0;

};



#endif

