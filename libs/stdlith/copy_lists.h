
// This module defines a bunch of functions to convert between the different
// list formats (arrays, linked lists, etc).

#ifndef __COPY_LISTS_H__
#define __COPY_LISTS_H__

#ifndef __DYNARRAY_H__
#include "dynarray.h"
#endif

#ifndef __GOODLINKLIST_H__
#include "goodlinklist.h"
#endif

#ifndef __STDLITHDEFS_H__
#include "stdlithdefs.h"
#endif


template<class T>
LTBOOL CopyGListToArray(CGLinkedList<T> &theList, CMoArray<T> &theArray)
{
	GPOS pos;
	uint32 curOut;

	if(!theArray.SetSize(theList.GetSize()))
		return FALSE;

	curOut = 0;
	for(pos=theList.GetHeadPosition(); pos; )
	{
		theArray[curOut] = theList.GetNext(pos);
		curOut++;
	}

	return TRUE;
}


template<class T>
LTBOOL CopyArrayToGList(CMoArray<T> &theArray, CGLinkedList<T> &theList)
{
	uint32 i;

	theList.Term();

	for(i=0; i < theArray.GetSize(); i++)
	{
		theList.Append(theArray[i]);
	}

	return TRUE;
}


#endif


