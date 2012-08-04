// ----------------------------------------------------------------------- //
//
// MODULE  : Globals.h
//
// PURPOSE : Global data
//
// CREATED : 4/26/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// global constants
const int MAX_OBJECT_ARRAY_SIZE = 32;

#define	COORD_CENTER	0x80000000

#if defined(_CLIENTBUILD)

// Version to use with dinput.
#define DIRECTINPUT_VERSION  0x0800

inline BOOL PtInRect(RECT* pRect, int x, int y)
{
	if((x >= pRect->right) || (x < pRect->left) ||
	   (y >= pRect->bottom) || (y < pRect->top))
	   return FALSE;

	return TRUE;
}

#endif // _CLIENTBUILD

// Helper classes

template<class TYPE>
class CRange : public TVector2<TYPE>
{
	public:

		CRange() { }
		CRange(TYPE tMin, TYPE tMax) { x = tMin; y = tMax; }
		
		const CRange& operator = ( const TVector2<TYPE> &vec2 )
		{ 
			x = vec2.x;
			y = vec2.y; 

			return *this;
		}

		void Set(TYPE tMin, TYPE tMax) { x = tMin; y = tMax; }

		TYPE GetMin() { return x; }
		TYPE GetMax() { return y; }
};

// Miscellaneous functions...

template< class T >
inline T MinAbs( T a, T b)
{
    return (T)fabs(a) < (T)fabs(b) ? a : b;
}

template< class T >
inline T MaxAbs( T a, T b)
{
    return (T)fabs(a) > (T)fabs(b) ? a : b;
}

template< class T >
inline T Clamp( T val, T min, T max )
{
	return LTMIN( max, LTMAX( val, min ));
}

#endif  // _GLOBALS_H_
