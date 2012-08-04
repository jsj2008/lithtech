#ifndef __WORLD_CLIENT_H__
#define __WORLD_CLIENT_H__

#ifndef __WORLD_INTERFACE_H__
#include "world_interface.h"
#endif

//
//Generic world interface used by engine client code.
//

class IWorldClient : public IWorld {
  public:
    interface_version_derived(IWorldClient, IWorld, 0);

    //Rendering and client side object management functions might be put here.
};



#endif

