//////////////////////////////////////////////////////////////////////////////
// Tools for packing stuff for the PC renderer

#ifndef __PCRENDERTOOLS_H__
#define __PCRENDERTOOLS_H__

inline uint32 ConvVecPCColor(const LTVector &vColor, float fAlpha)
{
	uint32 nResult;

	nResult  = ((uint32)(fAlpha)   & 0xFF) << 24;
	nResult |= ((uint32)(vColor.x) & 0xFF) << 16;
	nResult |= ((uint32)(vColor.y) & 0xFF) << 8;
	nResult |= ((uint32)(vColor.z) & 0xFF);

	return nResult;
}

#endif //__PCRENDERTOOLS_H__