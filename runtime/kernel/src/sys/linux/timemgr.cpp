
#include "timemgr.h"
#include <sys/time.h>
#include <unistd.h>

float time_GetTime()
{
    // tv_sec val from our very first gettimeofday() call
    static long startSeconds = 0;

    // get current time
    timeval curTimeval;
    gettimeofday(&curTimeval, NULL);

    // initialize startSeconds on first call
    if (startSeconds == 0)
        startSeconds = curTimeval.tv_sec;

    return (float)(curTimeval.tv_sec - startSeconds + curTimeval.tv_usec * 0.000001f);
}

uint32 timeGetTime()
{
	timeval curTimeval;
	gettimeofday(&curTimeval, NULL);
	return (curTimeval.tv_sec * 1000) + (curTimeval.tv_usec / 1000);
}
