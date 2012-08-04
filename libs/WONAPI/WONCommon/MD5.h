#ifndef __WON_MD52_H__
#define __WON_MD52_H__
#include "WONShared.h"

namespace WONAPI
{



class MD5
{
public:
	typedef unsigned long uint32;

private:
	enum
	{
		MD5_HASH_SIZE = 16
	};

	uint32 m_buf[4];
	uint32 m_bits[2];
	unsigned char m_in[64];

	void Transform(uint32 buf[4], uint32 in[16]);

public:
	MD5();
	void Reset();
	void Update(const void *theBuf, unsigned long theLen);
	void Digest(unsigned char digest[16]);
};


//----------------------------------------------------------------------------
// Raw MD5 calls.
//----------------------------------------------------------------------------


};

#endif

