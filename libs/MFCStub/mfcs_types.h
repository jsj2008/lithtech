// mfcs_types.h - Types for the MFC stub

#ifndef __MFCS_TYPES_H__
#define __MFCS_TYPES_H__

#include "stdlith.h"

// string types
typedef const char *LPCTSTR;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef char *LPTSTR;

// Signed version of CMoArray
template <class T>
class CSignedMoArray : public CMoArray<T>
{
public:
	int32 GetSize() const { return (int32)CMoArray<T>::GetSize(); }
};

// CArray replacement (the second parameter is ignored)
template <class T, class A>
class CArray : public CSignedMoArray<T>
{
};

// Rename position objects
#define POSITION LPOS

// Lists and typedef'ed arrays
typedef CLinkedList<void *> CPtrList;
typedef CSignedMoArray<uint32> CDWordArray;

#endif // __MFCS_TYPES_H__