#ifndef __NOISE_H__
#define __NOISE_H__


// given a 3D point, return a value in the range -1 to 1 (always returns 0 at integer lattice points)
float Noise( const LTVector& pos );


#endif // __NOISE_H__
