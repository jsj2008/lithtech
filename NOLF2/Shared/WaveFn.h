// ----------------------------------------------------------------------- //
//
// MODULE  : WaveFn.h
//
// PURPOSE : Wave functions
//
// CREATED : 10/21/02
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WAVEFN_H__
#define __WAVEFN_H__

#define WAVESTR_LINEAR	"LINEAR"
#define WAVESTR_SINE	"SINE"
#define WAVESTR_SLOWOFF	"SLOWOFF"
#define WAVESTR_SLOWON	"SLOWON"

// Wave types.
typedef enum
{
	Wave_Linear=0,
	Wave_Sine,
	Wave_SlowOff,
	Wave_SlowOn,
	NUM_WAVETYPES
} WaveType;

typedef float (*WaveFn)(float val);


// Wave functions, pass in a # 0-1 and they return a number 0-1.
float WaveFn_Linear(float val);
float WaveFn_Sine(float val);
float WaveFn_SlowOff(float val);
float WaveFn_SlowOn(float val);

// Guaranteed to return a value function (one of the wave functions).
WaveFn GetWaveFn(WaveType type);

// Should be used by objects allowing the wave type to be specified.
WaveType ParseWaveType(const char *pStr);

#endif // __WAVEFN_H__