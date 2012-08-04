
#ifndef __CONVERGE_H__
#define __CONVERGE_H__


	typedef float (*ConvergeCB)(float testVal, void *pUser);


	// The Converge function tries to find the best value within the specified range.
	// It calls a user-specified callback which should return higher numbers for better
	// values, and will try to locate the best possible value.
	float Converge(float testMin, float testMax, ConvergeCB cb, void *pUser, 
		uint32 nIterations, uint32 nMaxSubDivs);


#endif

