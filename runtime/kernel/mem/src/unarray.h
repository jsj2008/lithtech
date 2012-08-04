// ------------------------------------------------------------------------- //
//
// FILE      : U N A R R A Y . H
//
// CREATED   : 11/05/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : Loosly based upon struct banks and moArray's
//
// ------------------------------------------------------------------------- //

#ifndef __UNARRAY_H__
#define __UNARRAY_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "stdlterror.h"

#include "undata.h"
#include "ungen.h"

// ------------------------------------------------------------------------- //
// Typedefs & defines
// ------------------------------------------------------------------------- //
#define LTUnArrayNone = -1

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //

template < class T >
class CUnboundArray : public CUnboundGen < T > {
	public:
		ESTDLTResults Grow(uint32 ElemCnt);
		void PrintUnarrayStats ();
};

// ------------------------------------------------------------------------- //
// NAME: PrintUnarrayStats
// ------------------------------------------------------------------------- //
template <class T>
void
CUnboundArray<T>::PrintUnarrayStats () {
	CUnboundGen<T>::PrintUnGenStats();
}

#endif  // __UNARRAY_H__