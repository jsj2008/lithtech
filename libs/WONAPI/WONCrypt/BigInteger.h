#ifndef __WON_MINICRYPT_BIGINTEGER_H__
#define __WON_MINICRYPT_BIGINTEGER_H__
#include "WONShared.h"


#include <string>
#include <assert.h>
#include "WONCommon/WONTypes.h"
#include "Random.h"


namespace WONAPI
{
class BigInteger
{
public:
	typedef unsigned long word;
	typedef	unsigned __int64 dword;

protected:
	static const int   BITS; 
    static const dword RADIX;
    static const word  MASK; 

	enum { STACKN_SIZE = 4 };

	mutable word stackN[STACKN_SIZE];
	mutable word *n;
	mutable unsigned short nlen, len;
    bool negative;

	void NewBuffer(int theSize, const word *theOldBuf = NULL, int theOldBufSize = 0) const;    
	void Init();

public:
	virtual void debugInitStr() { }
	virtual void initStr() { }

	virtual ~BigInteger();

    int byteLength() const;
	int toBinary(RawBuffer &buffer) const;
	void fromBinary(const RawBuffer &buffer);

public:
	static int AddWords(word *r, const word *a, const word *b, int NA, int NB);
	static word SubWords(word *r, const word *a, const word *b, int NA, int NB);
	static void MultiplyWords(word *r, const word *a, const word *b, int NA, int NB);
	static void MultiplyWord(word *r, const word *a, const word b, int NA);
	static int CompareWords(const word *a, const word *b, int NA, int NB);
	static void KaratsubaMultiplyWords(word *r, word *workspace, const word *a, const word *b, int N, int aLen, int bLen);
	static int FindSize(const word *r, int size);


public:
    static const BigInteger ZERO;
    static const BigInteger ONE; 

	BigInteger();
	BigInteger(int from);
	BigInteger(const std::string& theString);
	BigInteger(const RawBuffer& buffer);
	BigInteger(int numBits, Random &rndSrc);
	BigInteger(const BigInteger&);
	const BigInteger& operator =(const BigInteger&);

	bool testBit(int theBit);
	BigInteger& setBit(int theBit);
	int bitLength() const;

	void fromString(const std::string& inHex);
	std::string toString() const;

	BigInteger gcd(const BigInteger &b) const;
	BigInteger modExp(const BigInteger &power, const BigInteger &modulo) const;
	BigInteger modExp(const BigInteger &power, const BigInteger &modulo, const BigInteger &recip, short nb) const;
	BigInteger inverseModN(const BigInteger &n) const;
	BigInteger mod(const BigInteger &b) const;
	BigInteger mul(const BigInteger &b) const;
	BigInteger div(const BigInteger &b) const;
	BigInteger divrem(const BigInteger &a, BigInteger &rem) const;
	BigInteger add(const BigInteger &b) const;
	BigInteger sub(const BigInteger &b) const;
	bool equals(const BigInteger &b) const;
	int cmp(const BigInteger &a) const;
	bool operator<(const BigInteger &a) const;

	BigInteger negate() const;

	BigInteger recip(int &nb) const;
    BigInteger modMulRecip(const BigInteger &y, const BigInteger &m, const BigInteger &i, short nb) const;

	BigInteger& shiftLeft(int n);
	BigInteger& shiftRight(int n);

public:
	static RawBuffer st_randomBits(int numBits, Random &rndSrc);

	static void st_CopyBuffer(word *dest, const word *src, size_t count);
    static void st_copy(BigInteger &dst, const BigInteger &src);

	static int st_bitLength(const BigInteger &n);
    static bool st_bit(const BigInteger &n, int i);
    static void st_setBit(BigInteger &n, int i);
	static void st_grow(const BigInteger &a, int i, bool copyOld = true);
    
    
    static void st_zero(BigInteger &a);
    static void st_one(BigInteger &a);

    static int st_cmp(const BigInteger &a, const BigInteger &b);
    static int st_ucmp(const BigInteger &a, const BigInteger &b);

    static void st_add(BigInteger &r ,const BigInteger &a, const BigInteger &b);
    static void st_add_unsigned(BigInteger &r, const BigInteger &a, const BigInteger &b);
    static void st_sub(BigInteger &r, const BigInteger &a, const BigInteger &b);
    static void st_sub_unsigned(BigInteger &r, const BigInteger &a, const BigInteger &b, bool determine_sign = true);

    static void st_shiftLeft(BigInteger &r, const BigInteger &a, int n);
    static void st_shiftRight(BigInteger &r, const BigInteger &a, int n);

    static void st_mul(BigInteger &r, const BigInteger &a, const BigInteger &b);
	static void st_kmul(BigInteger &r, const BigInteger &a, const BigInteger &b, word *tn = NULL);
    static void st_mod(BigInteger &r, const BigInteger &m, const BigInteger &d);
    static void st_div(BigInteger &dv, const BigInteger &m, const BigInteger &d);
    static void st_div(BigInteger *dv, BigInteger *rem, const BigInteger &m, const BigInteger &d);

    static int st_recip(BigInteger &r, const BigInteger &m);
    static void st_modMulRecip(BigInteger &r, const BigInteger &x, const BigInteger &y, const BigInteger &m, const BigInteger &i, short nb, word *t = NULL);
    static void st_exp(BigInteger &r, const BigInteger &a, const BigInteger &power);
    static void st_modExp(BigInteger &r, const BigInteger &a, const BigInteger &power, const BigInteger &modulo);
    static void st_modExp(BigInteger &r, const BigInteger &a, const BigInteger &power, const BigInteger &modulo, const BigInteger &recip, short nb);

    static void st_inverseModN(BigInteger &r, const BigInteger &a, const BigInteger &n);
    static void st_euclid(BigInteger &r, const BigInteger &x, const BigInteger &y);
    static void st_gcd(BigInteger &r, const BigInteger &a, const BigInteger &b);
/*
	static void st_sqrt(BigInteger &r, const BigInteger &x);

	static void st_MontgomeryReduce(BigInteger &r, BigInteger &workspace, const BigInteger &t, const BigInteger &n, int bBits, const BigInteger &nprime, word *work); 
	static int st_MontgomeryRepresentation(BigInteger &nprime, BigInteger &bsquared, const BigInteger &n);
	static void st_ModPow2(BigInteger &r, const BigInteger &t, int nBits);
	static void st_DivPow2(BigInteger &r, const BigInteger &t, int nBits);
	static void st_MulPow2(BigInteger &r, const BigInteger &t, int nBits);*/
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class ArithmeticException : public std::exception
{
public:
	std::string what;
	ArithmeticException() : exception() { }
	ArithmeticException(const std::string &theWhat) { what = theWhat; }
};

}; // namespace WONAPI

#endif
