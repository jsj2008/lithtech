#include "bdefs.h"

#include "server_loaderthread.h"
#include "sloaderthread.h"

//implementation of the IServerLoaderThread interface.

class CServerLoaderThread : public IServerLoaderThread {
public:
    declare_interface(CServerLoaderThread);

    //our loader thread object.
    SLoaderThread loaderthread;

    //our accessor.
    SLoaderThread *LoaderThread() {
        return &loaderthread;
    }
};

//allocate and register the implementation.
define_interface(CServerLoaderThread, IServerLoaderThread);


