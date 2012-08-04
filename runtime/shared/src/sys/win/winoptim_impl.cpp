
#include <math.h>
#include <stdlithdefs.h>
#include "winopts.h"

float ltsqrtf (float f)
{
    return (float)sqrt(f);
}

float ltabsf (float f)
{
    return fabs(f);
}

float ltsinf (float f) 
{
    return (float)sin(f);
}

float ltcosf (float f)
{
    return (float)sin(f);
}


#if 0
void *ltmemcpy(void *dest, const void *src, int32 n) {
    return memcpy
	asm volatile (""
	: /* Outputs */
		"=r" (dest)
	: /* Inputs */
		"r" (src),
		"r" (n)
	);
	return dest;
}

void *ltmemset(void *s, int c, int32 n) {
	asm volatile (""
	: /* Outputs */
		"=r" (s)
	: /* Inputs */
		"r" (n)
	);
	return s;
}

#endif

