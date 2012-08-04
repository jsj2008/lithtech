//
//The purpose of this file is to hard-code references to interface implementations
//that are compiled in libs and linked into the main executable.
//
//This is necessary in some situations, because if an interface implemenation
//is devoid of any staticly linked references to the outside world, the linker
//will not link in the global instance of the implementation, and therefore the
//entire implementation class itself.
//
//If you create an interface implementation in a LIB and are having the
//problem that holders to the interface are NULL, add a line for your 
//interface implementation here in the form of a link_to_implementation() macro 
//statement.
//
//The macro takes 2 parameters, the first being the name of the actual implementation
//class (the one used as first parameter in define_interface or instantiate_interface),
//the second being the instance name of your implementation.  If you have
//not specified a instance name, you are most likely using the default instance
//name of "Default", used without quotes.
//
//Your interface or implementation header files do NOT need be included here
//to use the link_to_implementation macro.  Adding/Needing those headers here
//is a sure sign that you are doing something wrong.
//

#include "ltmodule.h"
#include "libltinfo.h"		// for LIBLTINFO_OUTPUT_REDIRECTION switch

#if !defined(DE_SERVER_COMPILE)
    //
    // Not stand-alone server.
    //

        //
        // PC build
        //

        //UI interfaces are defined in a lib.
        link_to_implementation(CUIFontManager, Default);
		link_to_implementation(CUIWidgetManager, Default);
		link_to_implementation(CUIMessageQueue,	Default);

        //D3D render LIB implements interfaces.
		#ifdef __D3D
		link_to_implementation(CSysTexInterface, Default);
	    link_to_implementation(D3DRenderStyles, Default);
		#endif // __D3D

        //Remote Communication Contex LIB implements interfaces??

#ifdef LIBLTINFO_OUTPUT_REDIRECTION
		link_to_implementation(COutputRedir,					Default);
#endif // LIBLTINFO_OUTPUT_REDIRECTION
 

#else
 

    //
    // standalone server
    //

//	#if defined(__LINUX)
//            link_to_implementation(CServerConsoleState,		Default);
//            link_to_implementation(CServerLoaderThread,		Default);
//            link_to_implementation(CWorldServerBSP,			Default);
//            link_to_implementation(LTCollisionMgr,          Server);
//            link_to_implementation(CLTPhysicsServer,        Server);
//            link_to_implementation(CCompress,               Default);
//            link_to_implementation(CLTCommonServer,         Server);
//    #endif

#ifdef LIBLTINFO_OUTPUT_REDIRECTION
		link_to_implementation(COutputRedir,					Default);
#endif // LIBLTINFO_OUTPUT_REDIRECTION
 

#endif // DE_SERVER_COMPILE

