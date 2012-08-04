#include "bdefs.h"

#include "client_loaderthread.h"
#include "cloaderthread.h"

//implementation of the IClientLoaderThread interface.

class CClientLoaderThread : public IClientLoaderThread {
public:
    declare_interface(CClientLoaderThread);

    //our loader thread object.
    CLoaderThread loaderthread;

    //our accessor.
    CLoaderThread *LoaderThread() {
        return &loaderthread;
    }
};

//allocate and register the implementation.
define_interface(CClientLoaderThread, IClientLoaderThread);


