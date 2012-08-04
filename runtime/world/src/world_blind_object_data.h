//////////////////////////////////////////////////////////////////////////////
// Generic data stored in the .dat by preprocessor to be accessed by index

#ifndef __WORLD_BLIND_OBJECT_DATA_H__
#define __WORLD_BLIND_OBJECT_DATA_H__

#include "ltmodule.h"
#include "loadstatus.h"

class IWorldBlindObjectData : public IBase
{
public:
    interface_version(IWorldBlindObjectData, 0);

	virtual ~IWorldBlindObjectData() {};

	virtual void Term() = 0;
	
    virtual ELoadWorldStatus Load(ILTStream *pStream) = 0;

	virtual bool GetBlindObjectData( uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize ) = 0;
	virtual bool FreeBlindObjectData( uint32 nNum, uint32 nId ) = 0;
};

#endif //__WORLD_BLIND_OBJECT_DATA_H__
