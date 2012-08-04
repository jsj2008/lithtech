#include "ElGamal.h"
#include "MD5Digest.h"
#include "WONCommon/LittleEndian.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ElGamal::ElGamal()
{
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ElGamal::ElGamal(const void *theKey, int theKeyLen, bool isPrivate)
{
	if(isPrivate)
		SetPrivateKey(theKey,theKeyLen);
	else
		SetPublicKey(theKey,theKeyLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ElGamal::Invalidate()
{
	mIsPrivate = false;
	mIsPublic = false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::IsPrivate() const
{
	return mIsPrivate;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::IsPublic() const
{
	return mIsPublic;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ByteBufferPtr ElGamal::GetRawPrivateKey() const
{
	if(!IsPrivate())
		return NULL;

	IntegerInserter anInserter;
	anInserter.Insert(p);
	anInserter.Insert(q);
	anInserter.Insert(g);
	anInserter.Insert(y);
	anInserter.Insert(x);

	return anInserter.Get();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	
ByteBufferPtr ElGamal::GetRawPublicKey() const
{
	if(!IsPublic())
		return NULL;

	IntegerInserter anInserter;
	anInserter.Insert(p);
	anInserter.Insert(q);
	anInserter.Insert(g);
	anInserter.Insert(y);

	return anInserter.Get();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ElGamal::SetPrivateKey(const void *theKey, int theKeyLen)
{		
	Invalidate();

	IntegerExtractor anExtractor(theKey,theKeyLen);
	bool aSuccess = true;
	aSuccess = anExtractor.Extract(p) && anExtractor.Extract(q) && anExtractor.Extract(g) 
			   && anExtractor.Extract(y) && anExtractor.Extract(x);	
	
	if(!aSuccess)
		return false;
	
	modulusLen = p.bitLength()/8;

	recip = p.recip(nb);
	

	mIsPrivate = true;
	mIsPublic = true;
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ElGamal::SetPublicKey(const void *theKey, int theKeyLen)
{		
	Invalidate();

	IntegerExtractor anExtractor(theKey,theKeyLen);
	bool aSuccess = true;
	aSuccess = anExtractor.Extract(p) && anExtractor.Extract(q) && anExtractor.Extract(g)
		&& anExtractor.Extract(y);
	
	if(!aSuccess)
		return false;

	modulusLen = p.bitLength()/8;

	recip = p.recip(nb);

	mIsPublic = true;
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr ElGamal::Encrypt(const void *thePlainText, int thePlainTextLen) const
{
	if(!IsPublic())
		return NULL;

	const unsigned char *aPlainText = (const unsigned char*)thePlainText;

	int aBlockLen = modulusLen - 3;
	int aNumBlock = thePlainTextLen / aBlockLen;
	if ((thePlainTextLen % aBlockLen) != 0) aNumBlock++;

	WriteBuffer anEncrypt;
	anEncrypt.Reserve(4+modulusLen*2*aNumBlock);

	int anOffset = 0;

	anEncrypt.AppendLong(aNumBlock);

	while(anOffset < thePlainTextLen)
	{	
		int thisBlockLen = aBlockLen;
		
		if(thePlainTextLen - anOffset < aBlockLen)
			thisBlockLen = thePlainTextLen - anOffset;
		
		RawBuffer anEncryptBlock(modulusLen-1,(unsigned char)0);

		for(int k=0,j=modulusLen-2-thisBlockLen; j<modulusLen-2; j++,k++)
			anEncryptBlock[j] = aPlainText[anOffset+k];

		anEncryptBlock[modulusLen - 2] = (unsigned char)thisBlockLen;


		BigInteger ab[2];
		
		if(!encrypt(BigInteger(anEncryptBlock),ab))
			return NULL;


		RawBuffer aa,bb;
		ab[0].toBinary(aa);
		ab[1].toBinary(bb);
		
		if(aa.length()==modulusLen)
			anEncrypt.AppendBytes(aa.data(),aa.length());
		else if(aa.length()>modulusLen)
			anEncrypt.AppendBytes(aa.data()+aa.length()-modulusLen,modulusLen);
		else
		{
			for(int i=aa.length(); i<modulusLen; i++)
				anEncrypt.AppendByte(0);
			
			anEncrypt.AppendBytes(aa.data(),aa.length());
		}
		

		if(bb.length()==modulusLen)
			anEncrypt.AppendBytes(bb.data(),bb.length());
		else if(bb.length()>modulusLen)
			anEncrypt.AppendBytes(bb.data()+bb.length()-modulusLen,modulusLen);
		else
		{
			for(int i=bb.length(); i<modulusLen; i++)
				anEncrypt.AppendByte(0);
			
			anEncrypt.AppendBytes(bb.data(),bb.length());
		}						
		anOffset+=thisBlockLen;		
	}		

	return anEncrypt.ToByteBuffer();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr ElGamal::Decrypt(const void *theCipherText, int theCipherTextLen) const
{
	if(!IsPrivate())
		return NULL;

	const unsigned char *in = (const unsigned char*)theCipherText;
	int inOffset = 0;

	if(theCipherTextLen-inOffset<4) return NULL;
	int aNumBlocks = LongFromLittleEndian(*(int*)in); inOffset+=4;

	if(theCipherTextLen-inOffset < aNumBlocks*modulusLen*2-inOffset)
		return NULL;

	RawBuffer aBuf(modulusLen,(unsigned char)0);
	RawBuffer bBuf(modulusLen,(unsigned char)0);

	WriteBuffer aDecrypt;

	BigInteger a;;
	BigInteger b;
		
	BigInteger aPlainText;

	for(int i=0; i<aNumBlocks; i++)
	{
		aBuf.assign(in+inOffset,modulusLen); inOffset+=modulusLen;
		bBuf.assign(in+inOffset,modulusLen); inOffset+=modulusLen;
		
		a.fromBinary(aBuf);
		b.fromBinary(bBuf);

		if(!decrypt(a,b,aPlainText))
			return NULL;
		
		RawBuffer aBigIntArray;
		aPlainText.toBinary(aBigIntArray);
		

		if(aBigIntArray.length()==0) return NULL;
		int aPlainLen = aBigIntArray[aBigIntArray.length() - 1];
		
		if(aPlainLen>modulusLen - 3)
			return NULL;
		
		if(aBigIntArray.length() - 1 - aPlainLen < 0)
		{
			int extra = aPlainLen - (aBigIntArray.length() - 1);
			for(int j=0; j<extra; j++)
				aDecrypt.AppendByte(0);
			
			aDecrypt.AppendBytes(aBigIntArray.data(),aBigIntArray.length());
		}
		else
			aDecrypt.AppendBytes(aBigIntArray.data()+aBigIntArray.length()-1-aPlainLen,aPlainLen);
	}

	return aDecrypt.ToByteBuffer();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ElGamal::EncodeDigest(const RawBuffer& digest, BigInteger &h) const
{
	IntegerExtractor aDecoder(digest.data(),digest.length(),false);
	if(digest.length()*8 < q.bitLength())
	{
		if(!aDecoder.Decode(digest.length(),h))
			return false;
	}
	else
	{
		if(!aDecoder.Decode(q.byteLength(),h))
			return false;

		h = h.shiftRight(q.byteLength()*8 - q.bitLength() + 1);
	}

	return true;
}	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ElGamal::Verify(const void *theMessage, int theMessageLen, const void *theSignature, int theSignatureLen) const
{
	if(!IsPublic())
		return false;

	if(theSignatureLen!=q.byteLength()*2)
		return false;
	
	
	MD5Digest anMD5;
	anMD5.update(theMessage,theMessageLen);
	RawBuffer aDigest = anMD5.digest();
			

	BigInteger M;
	if(!EncodeDigest(aDigest,M))
		return false;

	const unsigned char *in = (const unsigned char*)theSignature;

	RawBuffer aBuf(q.byteLength(),(unsigned char)0);
	RawBuffer bBuf(q.byteLength(),(unsigned char)0);

	aBuf.assign(in,aBuf.length());
	bBuf.assign(in+aBuf.length(),bBuf.length());
					
	BigInteger a(aBuf);
	BigInteger b(bBuf);
	
	return BogusVerify(M,a,b);	
}		

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr ElGamal::Sign(const void* theMessage, int theMessageLen) const
{		
	if(!IsPrivate())
		return NULL;

	MD5Digest anMD5;
	anMD5.update(theMessage, theMessageLen);
	RawBuffer aDigest = anMD5.digest();
	
	BigInteger M;
	if(!EncodeDigest(aDigest,M))
		return NULL;
	
	BigInteger ab[2];
	
	if(!BogusSign(M,ab))
		return NULL;

	WriteBuffer aSignature;
		
	int qLen = q.byteLength();
	RawBuffer a,b;
	ab[0].toBinary(a);
	ab[1].toBinary(b);

	if(a.length()==qLen)
		aSignature.AppendBytes(a.data(),a.length());
	else if(a.length()>qLen)
		aSignature.AppendBytes(a.data()+a.length()-qLen,qLen);
	else
	{
		for(int i=a.length(); i<qLen; i++)
			aSignature.AppendByte(0);
		
		aSignature.AppendBytes(a.data(),a.length());
	}
	

	if(b.length()==qLen)
		aSignature.AppendBytes(b.data(),b.length());
	else if(b.length()>qLen)
		aSignature.AppendBytes(b.data()+b.length()-qLen,qLen);
	else
	{
		for(int i=b.length(); i<qLen; i++)
			aSignature.AppendByte(0);
		
		aSignature.AppendBytes(b.data(),b.length());
	}						
	
	return aSignature.ToByteBuffer();
}		

	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ElGamal::encrypt(const BigInteger &M, BigInteger ab[]) const
{
	try
	{
		BigInteger p_minus_1 = p.sub(BigInteger::ONE);
		// choose a random k, relatively prime to p-1.
		BigInteger k;
		do {
			// values of k with the same number of bits as p won't be chosen, but
			// that shouldn't be a problem.
			k = BigInteger(p.bitLength()-1, mRandom);
			if (!(k.testBit(0)))
				k = k.setBit(0); // make sure k is odd
		} while (!(k.gcd(p_minus_1).equals(BigInteger::ONE)));

		ab[0] /* a */ = g.modExp(k, p, recip, nb);
		ab[1] /* b */ = y.modExp(k, p, recip, nb).modMulRecip(M,p,recip,nb);

		return true;
	}
	catch(ArithmeticException&)
	{
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::decrypt(const BigInteger &a, const BigInteger &b, BigInteger &out) const
{
    try 
	{
		out = b.modMulRecip(a.modExp(x, p, recip, nb).inverseModN(p), p, recip, nb);
		return true;
    } 
	catch (ArithmeticException &) 
	{
    }
	
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BigInteger ElGamal::GetRandBetween(Random &rng, const BigInteger &min, const BigInteger &max)
{
	BigInteger range = max.sub(min);
	int nBits = range.bitLength();

	BigInteger anInt;
	do
	{
		anInt = BigInteger(nBits,rng);
	}
	while (anInt.cmp(range)>0);

	anInt = anInt.add(min);
	return anInt;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::BogusSign(const BigInteger &M, BigInteger ab[]) const
{
	try 
	{
		BigInteger TWO(2);		
		BigInteger k;
		
		do
		{
			k = GetRandBetween(mRandom, TWO, q.sub(TWO));			
			ab[0] = g.modExp(k,p);
			
			
			ab[0] = ab[0].add(M);
			
			ab[0] = ab[0].mod(p);
			
			ab[0] = ab[0].mod(q);
			
			ab[1] = k.sub(x.mul(ab[0]));

			
			ab[1] = ab[1].mod(q);
			
			
			//r = (gpc.Exponentiate(k) + m) % q;
			//s = (k - x*r) % q;
		} while (ab[0].cmp(BigInteger::ZERO)==0);			// make sure r != 0

		return true;
	}
	catch(ArithmeticException &)
	{
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
public static void sign(BigInteger M, BigInteger[] ab,
                 BigInteger p, BigInteger g, BigInteger x,
                 Random rng) 
{
    BigInteger p_minus_1 = p.subtract(ONE);
    // choose a random k, relatively prime to p-1.
    BigInteger k;
    do 
	{
        // values of k with the same number of bits as p won't be chosen, but
        // that shouldn't be a problem.
        k = new BigInteger(p.bitLength()-1, rng);
        if (!(k.testBit(0)))
            k = k.setBit(0); // make sure k is odd
    } while (!(k.gcd(p_minus_1).equals(ONE)));

    BigInteger a = g.modPow(k, p);
    ab[0] = a;

    // solve for b in the equation:
    //     M = (x.a + k.b) mod (p-1)
    // i.e.
    //     b = (inverse of k mod p-1).(M - x.a) mod (p-1)
    try 
	{
        ab[1] = k.modInverse(p_minus_1)
                         .multiply(M.subtract(x.multiply(a)).mod(p_minus_1))
                         .mod(p_minus_1);
    } 
	catch (ArithmeticException e) 
	{
		ab[0] = null;
		ab[1] = null;
    }
}*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
public static boolean verify(BigInteger M, BigInteger a, BigInteger b,
                      BigInteger p, BigInteger g, BigInteger y) 
{
    BigInteger p_minus_1 = p.subtract(ONE);
    // sanity checks
    if (M.compareTo(ZERO) < 0 || M.compareTo(p_minus_1) >= 0 ||
        a.compareTo(ZERO) < 0 || a.compareTo(p_minus_1) >= 0 ||
        b.compareTo(ZERO) < 0 || b.compareTo(p_minus_1) >= 0)
        return false;

    //accept iff y^a.a^b = g^M (mod p)
	

    return y.modPow(a, p).multiply(a.modPow(b, p)).mod(p)
            .equals(g.modPow(M, p));
}*/

bool ElGamal::BogusVerify(const BigInteger &M, const BigInteger &a, const BigInteger &b) const 
{

// check a != 0 && a == (g^b * y^a + m) mod q
	
	try
	{
		if(a.cmp(BigInteger::ZERO)==0)
			return false;

//		BigInteger a1 = a.mod(q);
//		BigInteger b1 = b.mod(q);
		
		BigInteger gbq = g.modExp(b,p,recip,nb);
		BigInteger yaq = y.modExp(a,p,recip,nb);

	/*
		cout << "p = " << p << endl;
		cout << "g = mod(" << g << ",p)" << endl;
		cout << "a = " << a << endl;
		cout << "b = " << b << endl;
		cout << "y = " << y << endl;

		cout << "gbq = " << gbq << endl;
		cout << "yaq = " << yaq << endl;*/
		
		BigInteger result;

		result = gbq.modMulRecip(yaq,p,recip,nb);
//		result = gbq.mul(yaq);
		result = result.add(M);
//		result = result.mod(p);
		result = result.mod(q);
		
		if(a.cmp(result)==0)
			return true;
		else
			return false;
	}
	catch(ArithmeticException &)
	{
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Utility Classes for Streaming ElGamal Keys in Crypto++ Format //////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


IntegerExtractor::IntegerExtractor(const void *theInput, unsigned long theInputLen, bool isBufferedTransform)
	: input((const unsigned char*)theInput, theInputLen)
{	
	offset = 0;
	mIsValid = true;
	
	if(isBufferedTransform)
	{	
		if(GetByte() != (SEQUENCE | CONSTRUCTED))
			return;
		else if(!LengthDecode())
			return;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned char IntegerExtractor::GetByte()
{
	if(offset<input.length())
		return input[offset++];

	mIsValid = false;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned char IntegerExtractor::PeekByte()
{
	if(offset<input.length())
		return input[offset];

	mIsValid = false;
	return 0;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned char IntegerExtractor::Decrement(RawBuffer& A, int N)
{
	unsigned char t = A[0];
	A[0] = (unsigned char)(t-1);
	if (A[0] <= t)
		return 0;
	
	for (int i=1; i<N; i++)
		if (A[i]-- != 0)
			return 0;
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IntegerExtractor::TwosComplement(RawBuffer& A, int N)
{
	Decrement(A, N);
	for (int i=0; i<N; i++)
		A[i] = (unsigned char)~A[i];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool IntegerExtractor::LengthDecode()
{
	int length;
	unsigned char b = GetByte();

	if ((b & 0x80) == 0)
		length = b;
	else
	{
		int lengthBytes = b & 0x7f;
		if (input.length() - offset < lengthBytes)
			return false;

		b = GetByte();
		while (b==0 && lengthBytes>1)
		{
			b = GetByte();
			lengthBytes--;
		}

		switch (lengthBytes)
		{
			case 0:
				return false;   // indefinite length
			case 1:
				length = b;
				break;
			case 2:
				length = b << 8;
				length |= GetByte();
				break;
			default:
				return false;
		}
	}
	return mIsValid;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool IntegerExtractor::Decode(int inputLen, BigInteger &theInt)
{
	if(!mIsValid)
		return false;

	if(input.length()<=offset)
		return false;

	bool negative = ((PeekByte() & 0x80)!=0) ? true : false;

	while (inputLen>0 && PeekByte()==0)
	{
		offset++;
		inputLen--;
	}

	RawBuffer reg(inputLen,(unsigned char)0);
			
	for (int i=0; i<inputLen; i++)
	{
		reg[i] = GetByte();
	}

/*		
	if (negative)
	{
		for (int i=inputLen; i<reg.length; i++)
			reg[i] = (byte)0xff;
		
		TwosComplement(reg, reg.length);
	}*/

	theInt.fromBinary(reg);
	return mIsValid;
}	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool IntegerExtractor::Extract(BigInteger &theInt)
{	
	if(!mIsValid)
		return false;

	if (GetByte() != INTEGER)
		return false;


	int bc;
	if ((PeekByte() & 0x80)==0)
		bc = GetByte();
	else
	{
		int lengthBytes = GetByte() & 0x7f;
		if (lengthBytes > 2)
			return false;
	
		bc = GetByte();
		if (lengthBytes > 1)
		{
			bc = (bc << 8) | GetByte();
		}
	}


	return Decode(bc,theInt);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IntegerInserter::IntegerInserter()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IntegerInserter::AddLength(RawBuffer &theBuf, int theLen)
{
	if(theLen>0x7f)
	{
		if(theLen>0xff)
		{
			theBuf+=0x82; // 2 length bytes

			unsigned char aLowLen = ((unsigned short)theLen)>>8;
			unsigned char aHighLen = theLen & 0xff;
			theBuf+=aHighLen;
			theBuf+=aLowLen;
		}
		else
		{
			theBuf+=0x81; // 1 length byte
			theBuf+=(unsigned char)theLen;
		}
	}
	else
	{
		theBuf+=(unsigned char)theLen;
	}

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ByteBufferPtr IntegerInserter::Get()
{
	RawBuffer aHeader;
	aHeader += (unsigned char)(SEQUENCE | CONSTRUCTED);
	AddLength(aHeader,out.length());
	
	char *aBuf = new char[aHeader.length() + out.length()];
	memcpy(aBuf,aHeader.data(),aHeader.length());
	memcpy(aBuf+aHeader.length(),out.data(),out.length());
	return new ByteBuffer(aBuf,aHeader.length() + out.length(),true);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void IntegerInserter::Insert(const BigInteger &theInt)
{
	out+=(unsigned char)INTEGER;
	RawBuffer aBuf;
	theInt.toBinary(aBuf);
	AddLength(out,aBuf.length());
	out+=aBuf;	
}	

