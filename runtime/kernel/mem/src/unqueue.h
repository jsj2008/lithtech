// ------------------------------------------------------------------------- //
//
// FILE      : U N Q U E U E . H
//
// CREATED   : 10/29/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : Loosly based upon struct banks and moArray's
//
// ------------------------------------------------------------------------- //

#ifndef __UNQUEUE_H__
#define __UNQUEUE_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "stdlterror.h"

#include "undata.h"
#include "ungen.h"

// ------------------------------------------------------------------------- //
// Typedefs
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Externs (Public C data)
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Prototypes (Public C methods)
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //

template <class T>
class CUnboundQueue : public CUnboundGen < T > {

	uint32* CurDelBlockHead;
	uint32* CurDelBlockTail;

	public:
		// init and reset
		ESTDLTResults Init  ();

		// add and remove from the queue
		ESTDLTResults Dequeue (T* NewElem);

		void PrintUnqueueStats ();
};

// ------------------------------------------------------------------------- //
// NAME: Init
// ------------------------------------------------------------------------- //
template <class T>
ESTDLTResults
CUnboundQueue<T>::Init () {

	CUnboundGen<T>::Init();

	// init the block head pointers
	CurDelBlockHead = BlockListHead;

	// init the block tail pointers
	CurDelBlockTail = CurInsBlockTail;

	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: Dequeue
// ------------------------------------------------------------------------- //
template <class T>
inline ESTDLTResults
CUnboundQueue<T>::Dequeue (T* NewElem) {
	// Make sure dequeing doesn't overrun past the start of the queue
	if (FirstElem == LastElem) { return (STDLT_END); }

	// copy the contents of first element to new element
	memcpy (NewElem, FirstElem, sizeof(T));

	// Free this block if we just dequeued the last element in it
	if ((uint32 *)(++FirstElem) == CurDelBlockTail) {
		// inc the DelBlockHead (before we free the info needed to do so ;-)
		if (CurDelBlockHead != BlockListHead) { return (STDLT_ERROR); }
		CurDelBlockHead = (uint32 *)(*CurDelBlockHead);

		// free this block and move everything along
		g_pUnboundData->ReturnOldBlock (BlockListHead);

		BlockListHead = CurDelBlockHead;
		CurDelBlockTail  = CurDelBlockHead + BlockSizeInWords;

		FirstElem = (T *)(CurDelBlockHead + 1);
	}
	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: PrintUnqueueStats
// ------------------------------------------------------------------------- //
template <class T>
void
CUnboundQueue<T>::PrintUnqueueStats () {
	CUnboundGen<T>::PrintUnGenStats();
}

#endif  // __UNQUEUE_H__