#ifndef __FIXEDPOINT_H__
#define __FIXEDPOINT_H__

// This header contains data types and routines for performing fixed point math.
// The original code was written by Chris Hecker (checker@netcom.com).  Additional
// modifications by Scott H. Pultz

inline int RoundFloatToInt(float f)
{
	int nResult;

	__asm
	{
		fld f
		fistp nResult
	}
	return nResult;
}

typedef long fixed28_4;
typedef long fixed16_16;
typedef unsigned long fixed16_16us;

#define FIXED28_4_ONE	16

inline fixed28_4 FloatToFixed28_4( float fValue )
{
	return (fixed28_4)(fValue * 16.0f);
}

inline fixed28_4 FloatToFixed28_4_Fast( float fValue )
{
	return (fixed28_4)RoundFloatToInt(fValue * 16.0f);
}

inline float Fixed28_4ToFloat( fixed28_4 fipValue )
{
	return (float)(fipValue / 16.0f);
}

inline long Fixed28_4ToLong( fixed28_4 fipValue )
{
	return (long)(fipValue/16);
}

inline fixed16_16 FloatToFixed16_16( float fValue )
{
	return (fixed16_16)(fValue * 65536.0f);
}

inline fixed16_16 FloatToFixed16_16_Fast( float fValue )
{
	return (fixed16_16)RoundFloatToInt(fValue * 65536.0f);
}

inline float Fixed16_16ToFloat( fixed16_16 fipValue )
{
	return (float)(fipValue / 65536.0f);
}

inline uint16 Fixed16_16ToWord( fixed16_16 fipValue )
{
	return (uint16)(fipValue >> 16);
}

inline fixed28_4 Fixed28_4Mul( fixed28_4 A, fixed28_4 B )
{
	// could make this asm to prevent overflow

	// 28.4 * 28.4 = 24.8 / 16 = 28.4
	return (fixed28_4)((A * B) / 16);
}

inline long Ceil28_4( fixed28_4 fipValue )
{
	long lReturnValue;
	long lNumerator = fipValue - 1 + 16;

	if ( lNumerator >= 0 )
	{
		lReturnValue = lNumerator/16;
	}
	else
	{
		// deal with negative numerators correctly
		lReturnValue = -((-lNumerator)/16);
		lReturnValue -= ((-lNumerator) % 16) ? 1 : 0;
	}
	return lReturnValue;
}

#endif