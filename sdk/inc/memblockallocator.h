//--------------------------------------------------------------------------------
// MemBlockAllocator.h
//
// This memory utility class takes a memory block in the constructor and allows
// allocation from that block. This will allow for debug checks for memory overflow
// and verifying that the resulting position matches the desired size. This provides
// a much cleaner interface for handling memory block allocations.
//
//--------------------------------------------------------------------------------
#ifndef __MEMBLOCKALLOCATOR_H__
#define __MEMBLOCKALLOCATOR_H__

//--------------------------------------------------------------------------
// Memory Alignment

//this function takes in an allocation size, and aligns it to the requested number of bytes.
//For arrays this should be used like AlignAllocSize(sizeof(A) * n), not AlignAllocSize(sizeof(A)) * n,
//as arrays will always be accessed usign multiples of A, so the latter just leads to wasted memory (but
//is still guaranteed to be aligned)
inline uint32 AlignAllocSize(uint32 nAllocSize, uint32 nAlignment)
{
	uint32 nExtendedSize = (nAllocSize + nAlignment - 1);
	return nExtendedSize - (nExtendedSize % nAlignment);
}

//this is the same as the above but is designed to align to the default alignment of the current
//platform, and is hard coded to improve performance
inline uint32 AlignAllocSize(uint32 nAllocSize)
{
	static const uint32 knAlignment = 4;
	uint32 nExtendedSize = (nAllocSize + knAlignment - 1);
	return nExtendedSize - (nExtendedSize % knAlignment);
}

//--------------------------------------------------------------------------
// Object Construction

//given a single object, this will call the constructor for it
template <typename T>
inline void ConstructObject(T* pObject)
{
	new (pObject) T;
}

//given a pointer to a list of objects, this will run through the objects and will call the constructor
//for each of the objects
template <typename T>
inline void ConstructObjects(T* pMemBlock, uint32 nNumObjects)
{
	for(uint32 nCurrObj = 0; nCurrObj < nNumObjects; nCurrObj++)
	{
		ConstructObject(pMemBlock + nCurrObj);
	}
}

//--------------------------------------------------------------------------
// Object Destruction

//given a pointer to an object, this will call the destructor for it
template <typename T>
inline void DestructObject(T* pObject)
{
	pObject->~T();
}

//given a pointer to a list of objects, this will run through the objects and will call the deconstructor
//for each of the objects
template <typename T>
inline void DestructObjects(T* pMemBlock, uint32 nNumObjects)
{
	for(uint32 nCurrObj = 0; nCurrObj < nNumObjects; nCurrObj++)
	{
		DestructObject(pMemBlock + nCurrObj);
	}
}

//--------------------------------------------------------------------------
// CMemBlockAllocator

class CMemBlockAllocator
{
public:

	//constructs a memory block allocator given a data block and the size of the block
	CMemBlockAllocator(uint8* pDataBlock, uint32 nBlockSize) :
		m_pDataBlock(pDataBlock),
		m_nAllocOffset(0),
		m_nBlockSize(nBlockSize)
	{
	}

	//called to determine the total memory block size
	uint32 GetBlockSize() const			{ return m_nBlockSize; }

	//called to get the allocation offset
	uint32 GetAllocationOffset() const	{ return m_nAllocOffset; }

    //called to allocate a block of memory from the memory block. This assumes that
	//there is enough room and will assert if there is not
	uint8* Allocate(uint32 nSize)
	{
		//align our allocation size so we don't address objects inefficiently
		uint32 nAlignSize = AlignAllocSize(nSize);

		//validate that we have enough room
		if ( !(m_nAllocOffset + nAlignSize <= GetBlockSize()))
			ASSERT(!"Error: Memory block overflow detected");

		//determine the memory to return
		uint8* pRV = (uint8*)(m_pDataBlock + m_nAllocOffset);

		//advance our allocation size for the next allocation
		m_nAllocOffset += nAlignSize;

		return pRV;
	}

	//called to allocate a collection of objects from the memory block and this will construct
	//the objects in place
	template <typename T>
	T* AllocateObjects(uint32 nNumObjects)
	{
		//get our memory block
		T* pMemBlock = (T*)Allocate(sizeof(T) * nNumObjects);
		ConstructObjects(pMemBlock, nNumObjects);
		return pMemBlock;
	}

	//called to allocate a single object from the memory block and this will construct
	//the objects in place
	template <typename T>
	T* AllocateObject()
	{
		//get our memory block
		T* pMemBlock = (T*)Allocate(sizeof(T));
		ConstructObject(pMemBlock);
		return pMemBlock;
	}

private:

	//the pointer to our data block
	uint8*		m_pDataBlock;

	//the total size of the data block in bytes
	uint32		m_nBlockSize;

	//our current allocation offset in bytes from the head of the data block
	uint32		m_nAllocOffset;
};

//--------------------------------------------------------------------------
// CMemBlockArray
//
//a template class that holds onto an array of objects allocated from a memory block allocator
//and will handle calling the destructor for each allocated object when it goes out of scope or
//has free called on it

template <typename ArrayType, typename IntSizeType = uint32>
class CMemBlockArray
{
public:

	CMemBlockArray() :
		m_pArray(NULL),
		m_nSize(0)
	{}

	~CMemBlockArray()						{ Free(); }

	//called to access the size
	IntSizeType GetSize() const				{ return m_nSize; }

	//called to access the array as a flat list of objects (useful for parameter passing)
	ArrayType*			GetArray()			{ return m_pArray; }
	const ArrayType*	GetArray() const	{ return m_pArray; }

	//called to free the memory associated with this array and destruct the objects
	void Free()
	{
		DestructObjects(m_pArray, m_nSize);
		m_pArray = NULL;
		m_nSize = 0;
	}

	//called to allocate a new array given a memory block allocator
	void Allocate(IntSizeType nNumItems, CMemBlockAllocator& Allocator)
	{
		m_pArray = Allocator.template AllocateObjects<ArrayType>(nNumItems);
		m_nSize = nNumItems;
	}

	//array access
	const ArrayType& operator[](uint32 nIndex) const	
	{
		if (!(nIndex < m_nSize))
			ASSERT(!"Error: Invalid array access");
		return m_pArray[nIndex]; 
	}

	ArrayType& operator[](uint32 nIndex)
	{ 
		if (!(nIndex < m_nSize))
			ASSERT(!"Error: Invalid array access");
		return m_pArray[nIndex]; 
	}

private:

	//our array
	ArrayType*	m_pArray;
	IntSizeType	m_nSize;

	//disable copying
	CMemBlockArray(const CMemBlockArray&);
	CMemBlockArray& operator=(const CMemBlockArray&);
};

#endif
