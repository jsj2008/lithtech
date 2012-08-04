#ifndef __WON_MINICRYPT_ELGAMAL_H__
#define __WON_MINICRYPT_ELGAMAL_H__
#include "WONShared.h"

#include "BigInteger.h"
#include "Random.h"
#include "WONCommon/WONTypes.h"
#include "WONCommon/ByteBuffer.h"
#include "WONCommon/WriteBuffer.h"

namespace WONAPI
{

class ElGamal
{

private:
	mutable Random mRandom;

	BigInteger p,q,g,y,x;
	BigInteger recip;
	int nb;

	int modulusLen;


	bool mIsPrivate;
	bool mIsPublic;

	bool EncodeDigest(const RawBuffer& digest, BigInteger &h) const;
	static BigInteger GetRandBetween(Random &rng, const BigInteger &min, const BigInteger &max);

	bool encrypt(const BigInteger &M, BigInteger ab[]) const;
	bool decrypt(const BigInteger &a, const BigInteger &b, BigInteger &out) const; 
	bool BogusSign(const BigInteger &M, BigInteger ab[]) const; 
	bool BogusVerify(const BigInteger &M, const BigInteger &a, const BigInteger &b) const;

public:
	ElGamal();
	ElGamal(const void *theKey, int theKeyLen, bool isPrivate);
	void Invalidate();
	bool IsPrivate() const;
	bool IsPublic() const;

	bool SetPrivateKey(const void *theKey, int theKeyLen);
	bool SetPublicKey(const void *theKey, int theKeyLen);

	ByteBufferPtr Encrypt(const void *thePlainText, int thePlainTextLen) const;
	ByteBufferPtr Decrypt(const void *theCipherText, int theCipherTextLen) const;

	bool Verify(const void *theMessage, int theMessageLen, const void *theSignature, int theSignatureLen) const;
	ByteBufferPtr Sign(const void *theMessage, int theMessageLen) const;

	ByteBufferPtr GetRawPrivateKey() const;
	ByteBufferPtr GetRawPublicKey() const;

	const BigInteger& GetG() { return g; }
	const BigInteger& GetP() { return p; }
	const BigInteger& GetY() { return y; }
	const BigInteger& GetX() { return x; }

	void SetX(const BigInteger &theX) { x = theX; }
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Utility Classes for Streaming ElGamal Keys in Crypto++ Format //////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class IntegerExtractor
{

private:
	RawBuffer input;
	int offset;
	bool mIsValid;

	bool LengthDecode();

	static unsigned char Decrement(RawBuffer& A, int N);
	static void TwosComplement(RawBuffer& A, int N);

	unsigned char GetByte();
	unsigned char PeekByte();

public:
	enum 
	{
		INTEGER=0x02,
		BIT_STRING=0x03,
		SEQUENCE=0x10,
		CONSTRUCTED = 0x20
	};

public:

	IntegerExtractor(const void *input, unsigned long inputLen, bool isBufferedTransform = true);

	bool Decode(int inputLen, BigInteger &theInt);
	bool Extract(BigInteger &theInt);
};

class IntegerInserter
{

private:
	RawBuffer out;

public:
	enum 
	{
		INTEGER=0x02,
		BIT_STRING=0x03,
		SEQUENCE=0x10,
		CONSTRUCTED = 0x20
	};

	void AddLength(RawBuffer& theBuf, int theLen);
public:
	IntegerInserter();

	void Insert(const BigInteger &theInt);
	ByteBufferPtr Get();
};

}; // namespace WONAPI

#endif
