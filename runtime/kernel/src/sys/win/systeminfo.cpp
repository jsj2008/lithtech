#include "bdefs.h"
#include "systeminfo.h"

uint32 CSystemInfo::GetProcessorCount()
{
	// when more calls are added to CSystemInfo, may want to make the
	// SYSTEM_INFO struct static so we don't need to call it more than
	// necessary.
	SYSTEM_INFO aSystemInfo;
	GetSystemInfo(&aSystemInfo);
	return aSystemInfo.dwNumberOfProcessors;
}