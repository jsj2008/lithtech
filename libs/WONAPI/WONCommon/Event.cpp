#ifndef __WON_SINGLETHREADED__

#if defined(WIN32)
#include "Event_Windows.cpp"
#elif defined(_LINUX)
#include "Event_Linux.cpp"
#endif

#endif // __WON_SINGLETHREADED__

