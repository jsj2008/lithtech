#ifndef __WON_MINICRYPT_BLOWFISH_H__
#define __WON_MINICRYPT_BLOWFISH_H__
#include "WONShared.h"

#include <string>
#include "WONCommon/WONTypes.h"
#include "WONCommon/ByteBuffer.h"

namespace WONAPI
{

class Blowfish
{

public:
	enum
	{
		minKeyLen    = 5,    // min key length in bytes (40 bits)
		blockLength  = 8,   
		maxKeyLen    = 56    // max key length in bytes (448 bits)
	};

private:
	static const int Pinit[];
	static const int Sinit[];

	int S0[256];
    int S1[256];
    int S2[256];
    int S3[256];
    int P[18];
	
	mutable RawBuffer XOR_MASK;
	RawBuffer mRawKey;
								 
    int P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17;
	
	bool mIsValid;

    void encryptTwoIntegers(int in[], int inOff, int out[], int outOff) const;
    void encryptBlock(const unsigned char *in, unsigned char *out) const;
    void decryptBlock(const unsigned char *in, unsigned char *out) const;	
  	void initXORMask() const;

public:
	Blowfish();
	Blowfish(int theKeyLen);
	Blowfish(const void *theKey, int theKeyLen);

	bool Create(int theLen);
	bool SetKey(const void *theKey, int theKeyLen);

	const void* GetKey() const { return mRawKey.data(); }
	int GetKeyLen() const { return mRawKey.length(); }

	ByteBufferPtr Encrypt(const void *in, int inLen) const;
	ByteBufferPtr Decrypt(const void *in, int inLen) const;	
};

}; // namespace WONAPI

#endif
