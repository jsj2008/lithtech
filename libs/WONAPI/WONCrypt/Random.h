#ifndef __WON_MINICRYPT_RANDOM_H__
#define __WON_MINICRYPT_RANDOM_H__
#include "WONShared.h"

#include "WONCommon/WONTypes.h"
#include "MD5Digest.h"

namespace WONAPI
{

class Random 
{

private:
    RawBuffer state;
    MD5Digest digest;

	RawBuffer randomBytes;
	int randomBytesUsed;
	__int64 counter;

	static long usageCount;
	
public:

	Random();
	Random(const RawBuffer& seed); 
	void setSeed(const RawBuffer& seed);
	void setSeed(__int64 seed);

	void nextBytes(RawBuffer& bytes);
	void nextBytes(void* data, int theLen);

	unsigned int next(int numBits);
	
	static RawBuffer getSeed(int numBytes); 
	RawBuffer longToByteArray(__int64 l); 


};

}; // namespace WONAPI

#endif
