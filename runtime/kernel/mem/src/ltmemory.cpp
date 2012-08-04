#include "bdefs.h"
#include "iltmemory.h"

// ------------------------------------------------------------------------
// memory interface implementation
// ------------------------------------------------------------------------
class CLTMemory : 
	public ILTMemory 
{
public:

	CLTMemory() :
		m_nAllocInfoRefCount(0)
	{
	}

	//sets up information about any allocations called before the matching call to
	//ReleaseAllocInfo. This information allows the engine to properly track memory
	//through its interfaces when tracking is enabled
	virtual void	SetAllocInfo(const char* pszFilename, uint32 nLine, uint32 nMemoryType)
	{
		#ifdef LTMEMTRACK
			//save our variables for the future allocation if we aren't already in a block
			if(m_nAllocInfoRefCount == 0)
			{
				m_nAllocLine = nLine;
				m_nAllocType = nMemoryType;

				LTStrCpy(m_pszAllocFile, pszFilename, sizeof(m_pszAllocFile));
			}

			m_nAllocInfoRefCount++;

		#endif
	}

	//Releases the allocation information so that other lines can use it to specify their
	//data
	virtual void	ReleaseAllocInfo()
	{
		#ifdef LTMEMTRACK
			//decrement our block count
			if(m_nAllocInfoRefCount > 0)
			{
				m_nAllocInfoRefCount--;
			}
			else
			{
				assert(!"Found mismatched calls to SetAllocInfo and ReleaseAllocInfo");
			}

		#endif
	}

	//called to acquire a block of memory from the engine. It will return NULL if no memory
	//could be allocated.
	virtual void*	Allocate(uint32 nSize)
	{
		#ifdef LTMEMTRACK

			//make sure that someone has locked the allocation information, otherwise
			//we will only have erronous information to report
			if(m_nAllocInfoRefCount)
			{
				//setup our allocation information
				LTMemTrackAllocStart(m_nAllocLine, m_pszAllocFile, m_nAllocType); 

				//allocate the memory
				void* pRV = dalloc(nSize); 

				//release our memory
				LTMemTrackAllocEnd();

				return pRV;
			}
			else
			{
				//this is untracked game code memory, set it up so we can track it coming from
				//here
				LTMemTrackAllocStart(__LINE__, __FILE__, LT_MEM_TYPE_UNKNOWN); 

				//allocate the memory
				void* pRV = dalloc(nSize); 

				//release our memory
				LTMemTrackAllocEnd();

				return pRV;
			}

		#else
			//we aren't tracking memory so just allocate
			return dalloc(nSize);
		#endif
	}

	//called to release a block of memory from the engine. This will properly handle NULL
	//values and should only be called on memory allocated with the Allocate function
	virtual void	Free(void* pData)
	{
		dfree(pData);
	}

private:

	//the reference count on the allocation information. This can only be set when it is 0
	uint32			m_nAllocInfoRefCount;

	//the line number that this allocation reference began at
	uint32			m_nAllocLine;

	//the filename that the allocation came from
	char			m_pszAllocFile[MAX_PATH + 1];

	//the type of memory for the current allocation
	uint32			m_nAllocType;
};

/*!
Exported function so that game modules hooking into the engine can access the memory library
in the global space
*/
extern "C" __declspec(dllexport) ILTMemory* LTGetILTMemory()
{
	static CLTMemory s_ILTMemory;
	return &s_ILTMemory;
}