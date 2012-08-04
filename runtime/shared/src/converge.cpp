
#include "bdefs.h"
#include "converge.h"



float Converge(float testMin, float testMax, ConvergeCB cb, void *pUser, 
	uint32 nIterations, uint32 nMaxSubDivs)
{
	float t, curMin, curMax, subDivSpan, prevVal, curVal;
	float bestMin, bestMax, bestMinVal, bestMaxVal, bestOverallVal;
	uint32 iIteration, iSubDiv;


	bestOverallVal = -100000.0f;

	// Keep moving in on smaller and smaller ranges, then just return.
	curMin = testMin;
	curMax = testMax;	
	for(iSubDiv=0; iSubDiv < nMaxSubDivs; iSubDiv++)
	{
		subDivSpan = (curMax - curMin) / (float)nIterations;
		if(subDivSpan < 0.0000001f)
			break;

		bestMin = curMin;
		bestMax = curMax;

		bestMinVal = cb(curMin, pUser);
		bestMaxVal = cb(curMax, pUser);

		prevVal = bestMinVal;
		for(iIteration=1; iIteration <= nIterations; iIteration++)
		{
			curVal = cb(curMin + (float)iIteration * subDivSpan, pUser);

			// Is this the best span?
			if((curVal + prevVal) > (bestMinVal + bestMaxVal))
			{
				bestMin = curMin + (float)(iIteration-1) * subDivSpan;
				bestMax = curMin + (float)(iIteration) * subDivSpan;
				bestMinVal = prevVal;
				bestMaxVal = curVal;
				bestOverallVal = (curVal + prevVal) * 0.5f; // Current best solution..
			}

			prevVal = curVal;
		}

		curMin = bestMin;
		curMax = bestMax;
	}

	// Return the middle of the range.
	return (curMin + curMax) * 0.5f;
}




