#include "bdefs.h"
#include "world_blind_object_data.h"
#include <vector>


class CWorldBlindObjectData : public IWorldBlindObjectData
{
public:
    declare_interface(CWorldBlindObjectData);

	CWorldBlindObjectData();
	virtual ~CWorldBlindObjectData();

	virtual void Term();
	
    virtual ELoadWorldStatus Load(ILTStream *pStream);

	virtual bool GetBlindObjectData( uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize );
	virtual bool FreeBlindObjectData( uint32 nNum, uint32 nId );

private:
	uint32 numInUse;				// number of chunks still in use
	std::vector<int32> dataSize;	// size of each chunk (-1 means it has been freed)
	std::vector<uint32> dataID;		// id for each chunk (for debug purposes)
	std::vector<uint8*> dataPtr;	// pointer to each chunks data
};

define_interface(CWorldBlindObjectData, IWorldBlindObjectData);


//////////////////////////////////////////////////////////////////////////////
// CWorldBlindObjectData - Blind object data implementation

CWorldBlindObjectData::CWorldBlindObjectData()
{
	numInUse = 0;
}


CWorldBlindObjectData::~CWorldBlindObjectData()
{
	Term();
}


void CWorldBlindObjectData::Term()
{
	// clear the memory for each chunk
	if( numInUse )
	{
		for( std::vector<uint8*>::iterator it = dataPtr.begin(); it != dataPtr.end(); it++ )
		{
			delete [] *it;
		}
	}

	// clear and collapse the vectors
	std::vector<int32>().swap( dataSize );
	std::vector<uint32>().swap( dataID );
	std::vector<uint8*>().swap( dataPtr );

	numInUse = 0;
}


ELoadWorldStatus CWorldBlindObjectData::Load(ILTStream *pStream)
{
	// kill any left over data
	Term();

	// read in the number of blind data chunks
	uint32 numChunks;
	*pStream >> numChunks;

	// ensure we don't reallocate as we load
	if( numChunks )
	{
		LT_MEM_TRACK_ALLOC(dataSize.reserve( numChunks ), LT_MEM_TYPE_MISC);
		LT_MEM_TRACK_ALLOC(dataID.reserve( numChunks ), LT_MEM_TYPE_MISC);
		LT_MEM_TRACK_ALLOC(dataPtr.reserve( numChunks ), LT_MEM_TYPE_MISC);
	}

	// add each chunk
	for( uint32 curChunk = 0; curChunk < numChunks; curChunk++ )
	{
		// read in the size of the data
		uint32 curDataSize;
		*pStream >> curDataSize;

		// read in the data id
		uint32 curDataID;
		*pStream >> curDataID;

		// read in the actual data
		uint8* curDataPtr = NULL;
		if( curDataSize )
		{
			LT_MEM_TRACK_ALLOC(curDataPtr = new uint8[curDataSize],LT_MEM_TYPE_WORLD);
			pStream->Read( curDataPtr, curDataSize );
		}

		// add the chunk
		dataSize.push_back( curDataSize );
		dataID.push_back( curDataID );
		dataPtr.push_back( curDataPtr );

		numInUse++;
	}

	return LoadWorld_Ok;
}


bool CWorldBlindObjectData::GetBlindObjectData( uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize )
{
	// make sure the chunk in question is in range
	if( nNum >= dataSize.size() )
	{
		ASSERT(0);
		return false;
	}

	// make sure the chunk hasn't been freed yet
	if( dataSize[nNum] == -1 )
	{
		ASSERT(0);
		return false;
	}

	// make sure the caller is actually requesting the correct chunk
	if( nId != dataID[nNum] )
	{
		ASSERT(0);
		return false;
	}

	nSize = dataSize[nNum];
	pData = dataPtr[nNum];

	return true;
}


bool CWorldBlindObjectData::FreeBlindObjectData( uint32 nNum, uint32 nId )
{
	// make sure the chunk in question is in range
	if( nNum >= dataSize.size() )
	{
		ASSERT(0);
		return false;
	}

	// make sure the chunk hasn't been freed yet
	if( dataSize[nNum] == -1 )
	{
		ASSERT(0);
		return false;
	}

	// make sure the caller is actually requesting the correct chunk
	if( nId != dataID[nNum] )
	{
		ASSERT(0);
		return false;
	}

	// free the memory
	delete [] dataPtr[nNum];
	dataPtr[nNum] = NULL;

	// mark this chunk as freed
	dataSize[nNum] = -1;
	numInUse--;

	// all the chunks have been freed, so minimize our memory usage
	if( !numInUse )
		Term();

	return false;
}
