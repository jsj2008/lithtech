/*
	Banked list template - Container class which provides static arrays of items
	to avoid dynamic allocations & maintain cache coherency

	Notes :
		- The banks are in a doubly linked list because they need to be removed
			quickly
		- LTList/LTLink isn't used because it's not type safe and stores the
			data through another pointer
		- The size of the banks are implemented for a 32-bit word size.
			This may need adjustment for other platforms.  (hint, hint)
		- This class is not const-friendly (yet)
		- This stores tail & size information in the list because its usage is
			not going to include continuous append operations.
		- The "tail" of the list is the last item, not the item after the last
*/

#ifndef __BANKEDLIST_H__
#define __BANKEDLIST_H__

// Forward declarations of sub-classes to keep VC sane
template <typename T>
class CBankedListBank;

template <typename T>
class CBankedListIter;

// The main Banked List class
template <typename T>
class CBankedList
{
    friend class CBankedListIter<T>;
    friend class CBankedListBank<T>;
public:
	CBankedList();
	~CBankedList();

	// Internal types
	typedef T t_Type;
	typedef CBankedList<t_Type> t_This;

	// External types
	typedef CBankedListIter<t_Type> t_Iter;
	enum t_NewOp { eNewOp_Pack, eNewOp_Append };

	// Initialize the list
	void Init();
	// Terminate the cached banks
	void Term();

	// Get the head of the list
	t_Iter GetHead();
	// Get the tail of the list (the last item)
	t_Iter GetTail();

	// Remove an item from the list (and return the next item, unless iPos is invalid)
	t_Iter Remove(const t_Iter &iPos);
	// Find an item in the list based on a pointer (O(n/k_SizeOfBank) for a packed list)
	t_Iter Find(t_Type *pPtr);
	// Remove an item from the list (Remove(Find(pPtr)))
	void Remove(t_Type *pPtr);
	// Append an item to the list
	void Append(const t_Type &_Item);
	// Delete an item from the list (Will call the dtor if bCallDtor is set)
	void Delete(t_Type *pPtr, bool bCallDtor = true);
	// Create a new item in the list (Will call the ctor if bCallCtor is set)
	T *New(t_NewOp eOp = eNewOp_Pack, bool bCallCtor = true);

	// Are you empty?
	bool IsEmpty();
	// How many items are currently in the list
	uint32 GetSize();
	// How much memory are you taking up?
	uint32 GetFootprint();
	// How much of the banks are actually in use? (returns a number (0..1])
	float GetUsage();

	// Make sure at least nMinItems items are available
	bool PreAlloc(uint32 nMinItems);

	// Get rid of the empty list
	void Flush();

private:
	enum { k_SizeOfBank = sizeof(uint32) * 8 };

	// Private bank type
	typedef CBankedListBank<t_Type> t_Bank;

	// Delete a bank list
	void DeleteBankList(t_Bank *pHead);
	// Move a bank to the empty list
	void MoveToEmptyList(t_Bank *pBank);
	// Get a "new" bank, either via the empty list, or via new, and add it to the end of the main list
	t_Bank *GetNewBank();

	t_Bank *m_pBankHead, *m_pBankTail;

	t_Bank *m_pEmptyHead;

	uint32 m_nSize;
};

// An iterator for the list (bidirectional)
// Note : Use CBankedList::t_Iter for the actual iterator.
template <typename T>
class CBankedListIter
{
    friend class CBankedList<T>;
public:
	typedef CBankedList<T> t_Parent;
	typedef CBankedListIter<T> t_This;
	typedef T t_Type;

	CBankedListIter();
	CBankedListIter(const t_This &iOther);

	// Are you actually pointing at something?
	bool IsValid();
	// Get what this iterator is currently pointing at
	t_Type &DeRef();
	// Move to the next item
	void Next();
	// Move to the previous item
	void Prev();
	// Copy another iterator
	void Copy(const t_This &iOther);
	// Operators overloads...
	inline t_This &operator ++() { Next(); return *this; }
	inline t_This &operator --() { Prev(); return *this; }
	inline t_This &operator =(const t_This &iOther) { Copy(iOther); return *this; }
	inline t_Type &operator *() { return DeRef(); }
	inline operator int() { return IsValid(); }
private: // Intended to be called from CBankedList<T>
	typedef CBankedListBank<T> t_Bank;
	// ctor for a new iterator, pointing at a location
	CBankedListIter(t_Bank *pBank, uint32 nIndex);
	// What were you pointing at again?
	t_Bank *GetBank();
	uint32 GetIndex();
private:
	t_Bank *m_pCurBank;
	uint32 m_nIndex;
};

#include "BankedList_impl.h"

#endif //__BANKEDLIST_H__
