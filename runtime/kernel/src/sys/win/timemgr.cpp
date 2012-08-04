
#include "bdefs.h"
#include "timemgr.h"


static uint32 g_TimeBase;


// It does this so the timer won't start out at really large numbers (otherwise, the numbers
// will be less accurate and the timer will recycle faster).
// At the windows timer rate, it would take 49.71 days for the timer to recycle itself.
class TimeInit
{
	public:

		TimeInit()
		{
			// Check to see if we are running on NT (or 2000 or later)...
			// and call timeBeginPeriod(1) to set timer resolution to 1ms.
			// (this only needs to be done for NT).
			OSVERSIONINFO osvi;
			ZeroMemory( &osvi, sizeof( osvi ) );
			osvi.dwOSVersionInfoSize = sizeof( osvi );    
			GetVersionEx( &osvi );
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
				timeBeginPeriod(1); // NT needs this for accurate-to-1-ms timer resolution

			g_TimeBase = timeGetTime();
		}
		~TimeInit()
		{
			// Terminate 1ms timer resolution if we are running on NT
			OSVERSIONINFO osvi;
			ZeroMemory( &osvi, sizeof( osvi ) );
			osvi.dwOSVersionInfoSize = sizeof( osvi );    
			GetVersionEx( &osvi );
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
				timeEndPeriod(1); // terminate 1ms timer resolution
		}

} _g_TimeInit;


float time_GetTime()
{
	return (float)(timeGetTime() - g_TimeBase) * (1.0f / 1000.0f);
}

uint32 time_GetMSTime()
{
	return (timeGetTime() - g_TimeBase);
}




