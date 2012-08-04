// ------------------------------------------------------------------------- //
//
// FILE      : U N G E N . H
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

#ifndef __UNGEN_H__
#define __UNGEN_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "stdlterror.h"

#include "undata.h"

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
class CUnboundGen {

protected:
	uint32* BlockListHead;
	uint32  BlockSizeInWords;
	uint32  BlockExcess;

	T* FirstElem;
	T* LastElem;
	T* CurElem;

	// Current Block to inqueue into
	uint32* CurInsBlockHead;
	uint32* CurInsBlockTail;
	// current Block we are iterating through
	uint32* CurItrBlockHead;
	uint32* CurItrBlockTail;

	public:
		// init and reset
		ESTDLTResults Init  ();
		ESTDLTResults Reset ();
		ESTDLTResults Done  ();

		// add to the queue
		T* Inqueue (T* NewElem);

		// queue iterator
		T* Begin ();
		T* Next  ();
		T* End   ();

		// print debug info
		void PrintUnGenStats ();
};

// ------------------------------------------------------------------------- //
// NAME: Init
// ------------------------------------------------------------------------- //
template <class T>
ESTDLTResults
CUnboundGen<T>::Init () {

	// If we still have storage, calling init is an error.  Should cal reset
	/*
	if (BlockListHead != NULL) {
		RECOVERABLE_ERROR("CUnboundGen::Init",
		"Called init with memory still allocated (call reset).\n");
		return (STDLT_ERROR);
	}
	*/

	// Get the starting block
	if ((BlockListHead = g_pUnboundData->GetNewBlock ()) == NULL) {
		return (STDLT_ERROR);
	}
	// BlockSizeInWords needs to be an exact multipule of
	// the structure size.  So, take the allocator's size and
	// come up with the largest size that is less than the
	// total block size, but still a multipule of struct size
	// The -4 is for the 1 word next pointer we keep.
	int totalSize = g_pUnboundData->GetBlockSize();
	BlockExcess = (totalSize -4) % sizeof(T);
	BlockSizeInWords = (totalSize - BlockExcess)/4;

	// init the block head pointers
	CurInsBlockHead = CurItrBlockHead = BlockListHead;

	// init the elem pointers
	FirstElem = CurElem = LastElem = (T *)(CurInsBlockHead + 1);

	// init the current insert block
	*CurInsBlockHead = (uint32)NULL;
	CurInsBlockTail  = CurInsBlockHead + BlockSizeInWords;

	// init the block tail pointers
	CurItrBlockTail = CurInsBlockTail;

	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: Reset
// ------------------------------------------------------------------------- //
template <class T>
ESTDLTResults
CUnboundGen<T>::Reset () {
	// Tear Everything down
	Done();

	// set everything up again
	return (Init());
}

// ------------------------------------------------------------------------- //
// NAME: Done
// ------------------------------------------------------------------------- //
template <class T>
ESTDLTResults
CUnboundGen<T>::Done () {
	uint32* tmpBlock;

	// Free up all storage
	while (BlockListHead != (uint32 *)NULL) {
		tmpBlock = BlockListHead;
		BlockListHead = (uint32 *)(*BlockListHead);
		g_pUnboundData->ReturnOldBlock (tmpBlock);
	}
	return (STDLT_OK);
}

// ------------------------------------------------------------------------- //
// NAME: Inqueue
// ------------------------------------------------------------------------- //
template <class T>
inline T*
CUnboundGen<T>::Inqueue (T* NewElem) {
	// copy the info in newElem into the lastElem
	T* retVal = LastElem;
	memcpy (LastElem, NewElem, sizeof(T));

	// inc the last elem, if we are at the end of a block
	// WARNING: for thread safty, the setting of LastElem must be the last
	// thing done before we return!
	if ((uint32 *)++LastElem == CurInsBlockTail) {
		// Get the new block
		if ((*CurInsBlockHead=(uint32)g_pUnboundData->GetNewBlock())
		   == (uint32)NULL) {
		   return (NULL);
		}

		// set up new block to be the current block
		CurInsBlockHead = (uint32 *)(*CurInsBlockHead);

		// init the current insert block
		*CurInsBlockHead = (uint32)NULL;
		CurInsBlockTail  = CurInsBlockHead + BlockSizeInWords;

		LastElem = (T *)(CurInsBlockHead + 1);
	}
	// } else {
	//	LastElem++;
	// }
	return (retVal);
}

// ------------------------------------------------------------------------- //
// NAME: Begin()
// ------------------------------------------------------------------------- //
template <class T>
inline T*
CUnboundGen<T>::Begin () {
	// set up head and tail we need to walk queue
	CurItrBlockHead = BlockListHead;
	CurItrBlockTail = CurItrBlockHead + BlockSizeInWords;
	CurElem         = (T *)(CurItrBlockHead + 1);
	// if this is an empty list then CurElem can start out
	// being the last block: return end
	if (CurElem == LastElem) return (NULL);

	return (CurElem);
}

// ------------------------------------------------------------------------- //
// NAME: Next()
// ------------------------------------------------------------------------- //
template <class T>
inline T*
CUnboundGen<T>::Next () {
	// if this is an empty list then CurElem can start out
	// being the last block: return end
	if (CurElem == LastElem) return (NULL);

	if ((uint32 *)++CurElem == CurItrBlockTail) {
		// move to next block if there is one
		if (*CurItrBlockHead == (uint32)NULL) return (NULL);
		CurItrBlockHead = (uint32 *)(*CurItrBlockHead);
		CurElem = (T *)(CurItrBlockHead + 1);

		// set up tail we need to walk queue
		CurItrBlockTail = CurItrBlockHead + BlockSizeInWords;
	} 

	// if we incremented of the end to where we are inserting
	// then return end
	if (CurElem == LastElem) 
		return (NULL);
	else 
		return (CurElem);
}
		
// ------------------------------------------------------------------------- //
// NAME: End()
// ------------------------------------------------------------------------- //
template <class T>
inline T*
CUnboundGen<T>::End () {
	if (CurElem == LastElem)
		return (NULL);
	else 
		return (CurElem);
}

#define DEBUG_OUT(x) printf(x)

// ------------------------------------------------------------------------- //
// NAME: PrintUnGenStats()
// ------------------------------------------------------------------------- //
template <class T>
inline void
CUnboundGen<T>::PrintUnGenStats () {
	char tmpString[256];

	sprintf (tmpString, "Number of elements per block = %d\n",
		(BlockSizeInWords*4)/sizeof(T));
	DEBUG_OUT(tmpString);

	sprintf (tmpString, "amount of excess per block = %d\n",BlockExcess);
	DEBUG_OUT(tmpString);

	// have the allocator dump it's stats
	g_pUnboundData->PrintUndataStats ();
}

#endif  // __UNQUEUE_H__
