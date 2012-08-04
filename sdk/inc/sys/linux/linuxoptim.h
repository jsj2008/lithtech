#ifndef __LINUXOPTIM_H__
#define __LINUXOPTIM_H__

#define ltmemcpy	memcpy
#define ltmemset	memset

#define ltsqrtf(x)	(float)sqrtf(x)
// Define for one over ltsqrtf (or reciprocal squareroot)
#define ltoosqrtf(_x) (1.0f/ltsqrtf(_x))

// Defines for using floating point versions of trig functions 
#define ltcosf(x)	(float)cos(x)
#define ltacosf(x)	(float)acos(x)
#define ltsinf(x)	(float)sin(x)
#define lttanf(x)	(float)tan(x)
#define lttanhf(x)	(float)tanh(x)
#define ltatanf(x)	(float)atan(x)
#define ltlogf(x)	(float)log(x)
#define ltlog2f(x)	(float)log2(x)
#define ltlog10f(x)	(float)log10(x)
#define ltpowf(x,y)	(float)pow(x,y)
#define ltfabsf(x)	(float)fabs(x)
#define ltfloorf(x)	(float)floor(x)
#define ltceilf(x)	(float)ceil(x)
#define ltfmodf(x,y) (float)fmod(x,y)

// Defines for number conversions
#define ltfptosi(x)	(int32)(x)
#define ltfptoui(x)	(uint32)(x)
#define ltsitofp(x)	(float)(x)
#define ltuitofp(x)	(float)(x)

#endif // __LINUXOPTIM_H__

