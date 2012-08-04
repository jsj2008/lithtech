// ----------------------------------------------------------------------- //
//
// MODULE  : WaveFn.cpp
//
// PURPOSE : Wave functions
//
// CREATED : 10/21/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WaveFn.h"
#include "ParsedMsg.h"


//-------------------------------------------------------------------------------------------
// WAVE FUNCTIONS.
//-------------------------------------------------------------------------------------------
WaveFn GetWaveFn(WaveType waveType)
{
	if(waveType == Wave_Linear)
		return WaveFn_Linear;
	else if(waveType == Wave_Sine)
		return WaveFn_Sine;
	else if(waveType == Wave_SlowOff)
		return WaveFn_SlowOff;
	else
		return WaveFn_SlowOn;
}

WaveType ParseWaveType(const char *pStr)
{
	static CParsedMsg::CToken s_cTok_Linear(WAVESTR_LINEAR);
	static CParsedMsg::CToken s_cTok_Sine(WAVESTR_SINE);
	static CParsedMsg::CToken s_cTok_SlowOff(WAVESTR_SLOWOFF);
	static CParsedMsg::CToken s_cTok_SlowOn(WAVESTR_SLOWON);

	CParsedMsg::CToken cTok_Str(pStr);

	if(cTok_Str == s_cTok_Linear)
	{
		return Wave_Linear;
	}
	else if(cTok_Str == s_cTok_Sine)
	{
		return Wave_Sine;
	}
	else if(cTok_Str == s_cTok_SlowOff)
	{
		return Wave_SlowOff;
	}
	else if(cTok_Str == s_cTok_SlowOn)
	{
		return Wave_SlowOn;
	}
	else
	{
		// Default..
		return Wave_Linear;
	}
}

float WaveFn_Linear(float val)
{
	return val;
}

float WaveFn_Sine(float val)
{
	val = MATH_HALFPI + val * MATH_PI;	// PI/2 to 3PI/2
	val = (float)sin(val);				// 1 to -1
	val = -val;							// -1 to 1
	val = (val + 1.0f) * 0.5f;			// 0 to 1
	return val;
}

float WaveFn_SlowOff(float val)
{
	if(val < 0.5f)
		return WaveFn_Linear(val);
	else
		return WaveFn_Sine(val);
}

float WaveFn_SlowOn(float val)
{
	if(val < 0.5f)
		return WaveFn_Sine(val);
	else
		return WaveFn_Linear(val);
}


