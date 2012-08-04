#include <assert.h>
#include <memory>
#include <algorithm>
#include <math.h>

#include "BigInteger.h"

using namespace std;
using namespace WONAPI;

const int BigInteger::BITS = 32;

#define LOW_WORD(x)  (word)(x)
#define HIGH_WORD(x) ((word)(x>>32))
//#define HIGH_WORD(x) (*(((word *)&(x))+1))

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::byteLength() const
{
	return ((st_bitLength(*this) + 7) / 8);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::st_bitLength(const BigInteger &n)
{
    int len = n.len;

    if (len == 0)
        return 0;

    int r = (len - 1) * BITS;

    unsigned int i = n.n[len-1];

    // Could probably speed this up with a binary search
    while (i != 0)
    {
        i >>= 1;
        ++r;
    }

    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

RawBuffer BigInteger::st_randomBits(int numBits, Random &rndSrc)
{		
	int numBytes = (numBits+7)/8;
	RawBuffer randomBits(numBytes,(unsigned char)0);

	// Generate random bytes and mask out any excess bits 
	if (numBytes > 0) 
	{
		rndSrc.nextBytes(randomBits);
		int excessBits = 8*numBytes - numBits;
		randomBits[0] &= (1 << (8-excessBits)) - 1;
	}
	return randomBits;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool BigInteger::st_bit(const BigInteger &n, int i)
{
    int bit = i % BITS;
    i /= BITS;

    if (i >= n.len || ((n.n[i] & (1L << bit)) == 0))
        return false;
	
    return true;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_setBit(BigInteger &n, int i)
{
    int bit = i % BITS;
    i /= BITS;

	
    if (i >= n.len)
		return;
	
	n.n[i] |= (1L << bit);				
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_CopyBuffer(word *dest, const word *src, size_t count)
{
	if(dest!=src)
		memcpy(dest,src,sizeof(word)*count);
	else
		memmove(dest,src,sizeof(word)*count);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::NewBuffer(int theSize, const word *theOldBuf, int theOldBufSize) const
{
	word *oldN = n;
	
	if(theSize>STACKN_SIZE) 
	{
		n = new word[theSize];
		assert(n);
	}
	else 
		n = stackN;

	if(theOldBuf!=n)
		memset(n,0,theSize*sizeof(word));

	nlen = theSize;
	len = theOldBufSize;


	if(theOldBuf!=NULL && theOldBuf!=n)
	{
		memcpy(n,theOldBuf,sizeof(word)*theOldBufSize);
	}

	if(oldN!=stackN)
		delete[] oldN;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::Init()
{
	n = NULL;

	NewBuffer(STACKN_SIZE);

    negative = false;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::~BigInteger()
{
	if(n!=stackN)
		delete[] n;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_copy(BigInteger &dst, const BigInteger &src)
{
    if (&dst == &src)
        return;

	dst.NewBuffer(src.nlen, src.n, src.len);
    dst.negative = src.negative;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_grow(const BigInteger &a, int i, bool copyOld)
{
    if (i <= a.nlen)
        return;

	i+=16; // Leave extra space

	if(copyOld)
		a.NewBuffer(i,a.n,a.len);
	else
		a.NewBuffer(i);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::toBinary(RawBuffer &buffer) const
{
    int len = (st_bitLength(*this)+7)/8;

	buffer.erase();
	buffer.reserve(len+1);

    int pos = 0;
    int bitpos = 0;

   	if(len==0)
		buffer += (unsigned char)0;
	
	for (int i=0; i<len; i++)
    {
        int b = (((unsigned int)n[pos] >> bitpos) & 0xFFL);
        bitpos += 8;
        if (bitpos >= BITS)
        {
            bitpos -= BITS;
            pos++;
            if (bitpos > 0)
                b |= (n[pos] << (8-bitpos)) & 0xFFL;
        }
        buffer += (unsigned char)b;
    }

	if(!negative && (char)buffer[buffer.length()-1] < 0)
		buffer += (unsigned char)0;

	reverse(buffer.begin(),buffer.end());
    return len;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::fromBinary(const RawBuffer &buffer)
{
    negative = false;    // Can't init negatives yet
    int alen = (buffer.length()*8 + BITS-1) / BITS;
    st_grow(*this, alen, false);
	len = alen;

    int pos = 0;
    n[pos] = 0;
    int bitpos = 0;
    // Index in reverse to get LSB first
    for (int i = buffer.length()-1; i >= 0; --i)
    {
        word b = buffer[i] & 0xFF;

        n[pos] |= (b << bitpos);
        bitpos += 8;
        if (bitpos >= BITS && i>0)
        {
            pos++;
            n[pos] = 0;
            bitpos -= BITS;
            if (bitpos > 0)
                n[pos] = b >> (8-bitpos);
        }
    }
    while (len > 0 && n[len-1] == 0)
        len--;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_zero(BigInteger &a)
{
    a.n[0] = 0;
    a.negative = false;
    a.len = 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_one(BigInteger &a)
{
    a.n[0] = 1;
    a.negative = false;
    a.len = 1;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::AddWords(word *r, const word *a, const word *b, int NA, int NB)
{
	if(NA<NB) 
	{
		return AddWords(r,b,a,NB,NA);
	}

	word carry = 0;
	int i;

	for(i=0; i<NB; i++)
	{
		dword result = (dword)a[i] + b[i] + carry;
		r[i] = LOW_WORD(result);
		carry = HIGH_WORD(result);

//		NumAdds+=2;
	}

	for(; i<NA; i++)
	{
		dword result = (dword)a[i] + carry;
		r[i] = LOW_WORD(result);
		carry = HIGH_WORD(result);

//		NumAdds++;
	}

	if(carry)
	{
		r[i] = carry;
		return i+1;
	}

	return i;
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::word BigInteger::SubWords(word *r, const word *a, const word *b, int NA, int NB)
{
	if(NA<NB) return SubWords(r,b,a,NB,NA);

	word borrow = 0;
	int i;
	for(i=0; i<NB; i++)
	{
		dword result = (dword)a[i] - b[i] - borrow;
		r[i] = LOW_WORD(result);
		borrow = 0 - HIGH_WORD(result);

//		NumAdds+=2;
	}

	for(; i<NA; i++)
	{
		dword result = (dword)a[i] - borrow;
		r[i] = LOW_WORD(result);
		borrow = 0 - HIGH_WORD(result);

//		NumAdds++;
	}

	return borrow;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::MultiplyWords(word *r, const word *a, const word *b, int NA, int NB)
{
	memset(r,0,(NA+NB)*sizeof(word));
    for (int i = 0; i < NA; ++i)
    {
        word carry = 0;
        dword m1 = a[i];
        int ri = i;

        for (int j = 0; j < NB; ++j)
        {
            dword m2 = r[ri];
            m2 += b[j] * m1 + carry;
            carry = HIGH_WORD(m2);
            r[ri++] = LOW_WORD(m2);

//			NumMults++;
//			NumAdds++;
        }
        r[ri] = carry;
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::MultiplyWord(word *r, const word *a, const word b, int NA)
{
	word carry = 0;
	dword result;
	int i;

	for(i=0; i < NA; ++i)
	{
		result = (dword)b*a[i] + carry;
		r[i] = LOW_WORD(result);
		carry = HIGH_WORD(result);

//		NumMults++;
//		NumAdds++;
	}

	r[i] = carry;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::CompareWords(const word *a, const word *b, int NA, int NB)
{
	if(NA!=NB)
		return NA>NB ? 1 : -1;

	for(int i=NA-1; i>=0; i--)
	{
		if(a[i]!=b[i])
			return a[i]>b[i] ? 1 : -1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::KaratsubaMultiplyWords(word *r, word *workspace, const word *a, const word *b, int N, int /*aLen*/, int bLen)
{

	if(N<=16)
	{
		MultiplyWords(r,a,b,N,N);
		return;
	}

	int N2 = N>>1;
	int TwoN = N<<1;
	int TwoN2 = N2<<1;
	
	const word *a0 = a;
	const word *b0 = b;
	const word *a1 = a + N2;
	const word *b1 = b + N2;

	word *c0 = r;
	word *c1 = workspace;
	word *c2 = r+2*N2;
	word *t = workspace+N;

	if(bLen<=N2)
	{
		memset(r,0,sizeof(word)*TwoN);
		KaratsubaMultiplyWords(r+N2,t,a1,b0,N2,N2,bLen);
		KaratsubaMultiplyWords(c1,t,a0,b0,N2,N2,bLen);
		AddWords(r,r,c1,N+N2,N);
	}
	else
	{
		bool needSubtract = false;
		if(CompareWords(a1,a0,N2,N2)>0)
		{
			SubWords(c0,a1,a0,N2,N2);
		}
		else
		{
			SubWords(c0,a0,a1,N2,N2);
			needSubtract = !needSubtract;
		}

		if(CompareWords(b1,b0,N2,N2)>0)
		{
			SubWords(c2,b1,b0,N2,N2);
			needSubtract = !needSubtract;
		}
		else
		{
			SubWords(c2,b0,b1,N2,N2);
		}

		KaratsubaMultiplyWords(c1,t,c0,c2,N2,N2,N2);
		KaratsubaMultiplyWords(c0,t,a0,b0,N2,N2,N2);		
		KaratsubaMultiplyWords(c2,t,a1,b1,N2,N2,N2);

		int tSize = AddWords(t,c0,c2,TwoN2,TwoN2);

		if(needSubtract)
		{
			SubWords(t,t,c1,tSize,TwoN2);
		}
		else
		{
			tSize = AddWords(t,t,c1,tSize,TwoN2);
		}

		AddWords(r+N2,r+N2,t,N+N2,tSize);
	}

	if(N&1)
	{	
		if(b[N-1]!=0)
		{
			MultiplyWord(c1,a,b[N-1],N-1);
			AddWords(r+N-1,r+N-1,c1,N,N);
		}

		MultiplyWord(t,b,a[N-1],N-1);
		AddWords(r+N-1,r+N-1,t,N,N);

		dword result = a[N-1]*b[N-1] + r[TwoN-2];
		r[TwoN-2] = LOW_WORD(result);
		r[TwoN-1] = HIGH_WORD(result);
	}
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::FindSize(const word *r, int size)
{
	while(size>0 && r[size-1]==0)
		size--;

	return size;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_add(BigInteger &r, const BigInteger &a, const BigInteger &b)
{
    if (a.len == 0)
    {
        st_copy(r, b);
        return;
    }
    if (b.len == 0)
    {
        st_copy(r, a);
        return;
    }

    if (a.negative)
    {
        if (b.negative)
        {
            st_add_unsigned(r, a, b);
            r.negative = true;
        }
        else
        {
            st_sub_unsigned(r, b, a);
        }
    }
    else
    {
        if (b.negative)
        {
            st_sub_unsigned(r, a, b);
        }
        else
        {
            st_add_unsigned(r, a, b);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_sub(BigInteger &r, const BigInteger &a, const BigInteger &b)
{
    if (a.len == 0)
    {
        st_copy(r, b);
        if (b.len > 0)
            r.negative = true ^ b.negative;
        return;
    }
    if (b.len == 0)
    {
        st_copy(r, a);
        return;
    }

    if (a.negative)
    {
        if (b.negative)
        {
            st_sub_unsigned(r, b, a);
        }
        else
        {
            st_add_unsigned(r, b, a);
            r.negative = true;
        }
    }
    else
    {
        if (b.negative)
        {
            st_add_unsigned(r, a, b);
        }
        else
        {
            st_sub_unsigned(r, a, b);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::st_cmp(const BigInteger &a, const BigInteger &b)
{
    if (a.len == 0 && b.len == 0) return 0;

    if (a.negative)
    {
        if (b.negative)
        {
            return st_ucmp(b, a);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (b.negative)
        {
            return 1;
        }
        else
        {
            return st_ucmp(a, b);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::st_ucmp(const BigInteger &a, const BigInteger &b)
{
    return CompareWords(a.n,b.n,a.len,b.len);
/*	int alen = a.len;
    int blen = b.len;

    if (alen < blen) return -1;
    if (alen > blen) return 1;

    word *an = a.n;
    word *bn = b.n;

    for (int i = alen-1; i >= 0; --i)
    {
        if (an[i] < bn[i]) return -1;
        if (an[i] > bn[i]) return 1;
    }
    return 0;*/
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_shiftLeft(BigInteger &r, const BigInteger &a, int n)
{
    if (a.len == 0)
    {
        st_zero(r);
        return;
    }
    int rem = n % BITS;
    int blocks = n / BITS;
    int len = a.len;

    st_grow(r, len + blocks);
    r.len = len + blocks;

    word *rn = r.n;

	st_CopyBuffer(rn+blocks,a.n,len);

    if (blocks > 0)
        for (int i = blocks-1; i >= 0 ; --i) { rn[i] = 0; }

    if (rem != 0)
    {
        word carry = 0;

        int rlen = r.len;
        for (int i = blocks; i < rlen; ++i)
        {
            word l = rn[i];

            rn[i] = ((l << rem) | carry);
            carry = (unsigned int)l >> (BITS-rem);
        }
        if (carry != 0)
        {
            rlen += 1;
            st_grow(r, rlen);
            r.n[rlen-1] = carry;
            r.len = rlen;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_shiftRight(BigInteger &r, const BigInteger &a, int n)
{
    int rem = n % BITS;
    int blocks = n / BITS;

    if (blocks >= a.len)
    {
        st_zero(r);
        return;
    }

    st_grow(r, a.len - blocks);
    r.len = a.len - blocks;

	st_CopyBuffer(r.n,a.n+blocks,r.len);

    if (rem != 0)
    {
        word carry = 0;
        word *rn = r.n;

        int rlen = r.len;
        for (int i = rlen-1; i > 0; --i)
        {
            word l = rn[i];

            rn[i] = ((unsigned int)l >> rem) | carry;
            carry = (l << (BITS-rem));
        }
        int l = rn[0];

        rn[0] = ((unsigned int)l >> rem) | carry;
        if (rlen > 0 && rn[rlen-1] == 0)
            r.len--;
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_add_unsigned(BigInteger &r, const BigInteger &a, const BigInteger &b)
{
    // Ensure a is the longest
    if (a.len < b.len)
    {
		st_add_unsigned(r,b,a);
		return;
    }

    // Needed in case the result is same object as r
    int alen = a.len;
    int blen = b.len;

    st_grow(r, alen + 1);
   
	r.negative = false;

	r.len = AddWords(r.n,a.n,b.n,alen,blen);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_sub_unsigned(BigInteger &r, const BigInteger &a, const BigInteger &b, bool determine_sign)
{
    switch (st_ucmp(a, b))
    {
    case 0:
        st_zero(r);
        return;
    case -1:
		if(determine_sign)
			r.negative = true;

		st_sub_unsigned(r,b,a,false);
		return;
    case 1:
        if(determine_sign)
			r.negative = false;
    }

    // Now a is the largest

    st_grow(r, a.len);

    word borrow = SubWords(r.n,a.n,b.n,a.len,b.len);
	r.len = FindSize(r.n,a.len);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_mul(BigInteger &r, const BigInteger &a, const BigInteger &b)
{
    if (&r == &a || &r == &b)
		throw ArithmeticException("Result must not be either Parameter (a or b)");

    if (a.len == 0 || b.len == 0)
    {
        st_zero(r);
        return;
    }

    r.negative = a.negative ^ b.negative;


    st_grow(r, a.len + b.len,false);
    r.len = a.len + b.len;

	MultiplyWords(r.n,a.n,b.n,a.len,b.len);
	r.len = FindSize(r.n,r.len);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_kmul(BigInteger &r, const BigInteger &a, const BigInteger &b, word *workspace)
{
	word *tn = workspace;

    if (&r == &a || &r == &b)
		throw ArithmeticException("Result must not be either Parameter (a or b)");

    if (a.len == 0 || b.len == 0)
    {
        st_mul(r,a,b);
        return;
    }

	if(a.len<b.len)
	{
		st_kmul(r,b,a,tn);
		return;
 	}

    r.negative = a.negative ^ b.negative;
	int aSize = a.len;

	if(tn==NULL)
		tn = new word[aSize*3];

	st_grow(a,aSize,true);
	st_grow(b,aSize,true);


	st_grow(r,aSize<<1,false);
	r.len = a.len+b.len;

	word *an = a.n;
	word *bn = b.n;

//	memset(an+a.len,0,(aSize-a.len)*sizeof(word));
//	memset(bn+b.len,0,(aSize-b.len)*sizeof(word));
//	memset(tn,0,aSize*3*sizeof(word));

//	memcpy(an,a.n,a.len*sizeof(word));
//	memcpy(bn,b.n,b.len*sizeof(word));


	KaratsubaMultiplyWords(r.n,tn,an,bn,a.len,a.len,b.len);
	r.len = FindSize(r.n,r.len);
		
	if(workspace==NULL)
		delete[] tn;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_mod(BigInteger &r, const BigInteger &m, const BigInteger &d)
{
    if (&r == &m)
		throw(ArithmeticException("Result must not be the same object as m"));

    st_copy(r, m);
    if (st_ucmp(m, d) < 0)
	{
		if(m.negative)
		{
			r.negative = true;
			st_add(r,d,r);
		}
        return;
	}

	r.negative = false;
	
    int i = st_bitLength(m) - st_bitLength(d);

    BigInteger ds;
    st_shiftLeft(ds, d, i);

    for (; i>= 0; --i)
    {
        if (st_cmp(r, ds) >= 0)
            st_sub(r, r, ds);
        st_shiftRight(ds, ds, (short)1);
    }
	
	if(m.negative)
	{
		r.negative = true;
		st_add(r,d,r);
	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_div(BigInteger &dv, const BigInteger &m, const BigInteger &d)
{
	st_div(&dv, NULL, m, d);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_div(BigInteger *dvOrig, BigInteger *remOrig, const BigInteger &m, const BigInteger &d)
{
    if (d.len == 0)
		throw(ArithmeticException("div: Divide by zero"));

    if (st_cmp(m, d) < 0)
    {
        if (remOrig != NULL)
            st_copy(*remOrig, m);
        if (dvOrig != NULL)
            st_zero(*dvOrig);

        return;
    }

	BigInteger *dvPtr = dvOrig, *remPtr = remOrig;

    if (dvPtr == NULL)
        dvPtr = new BigInteger();
    if (remPtr == NULL)
        remPtr = new BigInteger();

	auto_ptr<BigInteger> aDelDv(dvPtr), aDelRem(remPtr);
	if(dvOrig!=NULL)
		aDelDv.release();
	if(remOrig!=NULL)
		aDelRem.release();


	BigInteger &dv = *dvPtr;
	BigInteger &rem = *remPtr;

    BigInteger ds;
    st_copy(rem, m);
    st_zero(dv);

    int i = st_bitLength(m) - st_bitLength(d);
    st_shiftLeft(ds, d, i);

    for (; i >= 0; --i)
    {
        if (dv.len == 0)
        {
            if (st_cmp(rem, ds) >= 0)
            {
                st_one(dv);
                st_sub(rem, rem, ds);
            }
        }
        else
        {
            st_shiftLeft(dv, dv, 1);
            if (st_cmp(rem, ds) >= 0)
            {
                dv.n[0] |= 1;
                st_sub(rem, rem, ds);
            }
        }
        st_shiftRight(ds, ds, 1);
    }
    dv.negative = m.negative ^ d.negative;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_exp(BigInteger &r, const BigInteger &a, const BigInteger &power)
{
    BigInteger v1Orig;
	BigInteger v2Orig;

	st_copy(v1Orig,a);

	BigInteger *v1 = &v1Orig;
	BigInteger *v2 = &v2Orig;
	BigInteger *temp;
    
	int bits = st_bitLength(power);

    if ((power.n[0] & 1) != 0)
        st_copy(r,a);
    else
        st_one(r);


    for (int i = 1; i < bits; i++)
    {
        st_mul(*v2, *v1, *v1);
		temp = v1; v1 = v2; v2 = temp;

        if (st_bit(power, i))
		{
			st_copy(*v2,r);
			st_mul(r, *v1, *v2);
		}
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_modExp(BigInteger &r, const BigInteger &a, const BigInteger &power, const BigInteger &modulo)
{
    BigInteger d;
	int nb = st_recip(d, modulo);

	st_modExp(r,a,power,modulo,d,nb);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_modExp(BigInteger &r, const BigInteger &a, const BigInteger &power, const BigInteger &modulo, const BigInteger &d, short nb)
{
    BigInteger v;

    st_mod(v, a, modulo);
    int bits = st_bitLength(power);

    if ((power.n[0] & 1) != 0)
        st_mod(r, a, modulo);
    else
        st_one(r);

	int aSize = modulo.len;
	word *workspace = new word[aSize*6];

/*
	BigInteger nprime, bsquared;

	int nbits = MontgomeryRepresentation(nprime,bsquared,modulo);
	BigInteger vMontgomery;
	BigInteger rMontgomery;
	BigInteger temp1, temp2;

	mul(temp1,v,bsquared);
	MontgomeryReduce(vMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);

	mul(temp1,r,bsquared);
	MontgomeryReduce(rMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);
*/

     for (int i = 1; i < bits; i++)
    {

        st_modMulRecip(v, v, v, modulo, d, nb,workspace);

//		kmul(temp1,vMontgomery,vMontgomery,workspace);
//		MontgomeryReduce(vMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);

	
        if (st_bit(power, i))
		{
			st_modMulRecip(r, r, v, modulo, d, (short)nb,workspace);
			//kmul(temp1,rMontgomery,vMontgomery,workspace);
			//MontgomeryReduce(rMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);

		}
    }

//	MontgomeryReduce(r,temp2,rMontgomery,modulo,nbits,nprime,workspace);
	delete[] workspace;

	if(power.negative)
		st_inverseModN(r,r,modulo);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::st_recip(BigInteger &r, const BigInteger &m)
{
    BigInteger t;
    st_one(t);

    int mbits = st_bitLength(m);

    st_shiftLeft(t, t, 2*mbits);
    st_div(&r, NULL, t, m);
    return mbits+1;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_euclid(BigInteger &r, const BigInteger &x, const BigInteger &y)
{
    BigInteger aOrig;
    BigInteger bOrig;

    st_copy(aOrig, x);
    st_copy(bOrig, y);

	BigInteger *a = &aOrig;
	BigInteger *b = &bOrig;

    int shifts = 0;

    while (b->len != 0)
    {
        if ((a->n[0] & 1) != 0)    // a odd
            if ((b->n[0] & 1) != 0)    // b odd
            {
                st_sub(*a, *a, *b);
                st_shiftRight(*a, *a, 1);
                if (st_cmp(*a, *b) < 0)
                {
                    BigInteger *t = a;
                    a = b;
                    b = t;
                }
            }
            else
            {
                st_shiftRight(*b, *b, 1);
                if (st_cmp(*a, *b) < 0)
                {
                    BigInteger *t = a;
                    a = b;
                    b = t;
                }
            }
        else
            if ((b->n[0] & 1) != 0)    // b odd
            {
                st_shiftRight(*a, *a, 1);
                if (st_cmp(*a, *b) < 0)
                {
                    BigInteger *t = a;
                    a = b;
                    b = t;
                }
            }
            else
            {
                st_shiftRight(*a, *a, 1);
                st_shiftRight(*b, *b, 1);
                shifts++;
            }
    }
    if (shifts > 0)
        st_shiftLeft(r, *a, shifts);
    else
        st_copy(r, *a);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_gcd(BigInteger &r, const BigInteger &a, const BigInteger &b)
{
    if (st_cmp(a, b) > 0)
        st_euclid(r, a, b);
    else
        st_euclid(r, b, a);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_modMulRecip(BigInteger &r, const BigInteger &x, const BigInteger &y, const BigInteger &m, const BigInteger &i,short nb, word *t)
{
    BigInteger a;
    BigInteger b;
    BigInteger c;
    BigInteger d;

    st_kmul(a, x, y,t);			
	st_shiftRight(d, a, nb-1);		
    st_kmul(b, d, i,t);			
    st_shiftRight(c, b, nb-1);	
    st_kmul(b, m, c,t);		
    st_sub(r, a, b);		

    int j = 0;
    while (st_cmp(r, m) >= 0)
    {
        if (j++ > 2)
		{
			throw ArithmeticException("modMulRecip");
		}

        st_sub(r, r, m);
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::st_inverseModN(BigInteger &r, const BigInteger &a, const BigInteger &n)
{
    if (a.negative || n.negative)
		throw ArithmeticException("inverseModN");

	BigInteger x1orig;
	BigInteger x2orig;
	BigInteger x3orig; st_copy(x3orig,a);

	BigInteger y1orig;
	BigInteger y2orig;
	BigInteger y3orig; st_copy(y3orig,n);

    BigInteger *x1 = &x1orig;
    BigInteger *x2 = &x2orig;
    BigInteger *x3 = &x3orig;
    BigInteger *y1 = &y1orig;
    BigInteger *y2 = &y2orig;
	BigInteger *y3 = &y3orig;

    st_one(*x1); st_one(*y2);
    st_zero(*x2); st_zero(*y1);

	BigInteger t1orig;
	BigInteger t2orig;
	BigInteger t3orig;
	BigInteger qorig; 
	BigInteger porig;

	BigInteger *t1 = &t1orig;
	BigInteger *t2 = &t2orig;
	BigInteger *t3 = &t3orig;
	BigInteger *q = &qorig;
	BigInteger *p = &porig;


    while (y3->len != 0)
    {
        st_div(q, t3, *x3, *y3);
        st_mul(*t1, *q, *y2);		
        st_sub(*t2, *x2, *t1);
        st_mul(*p, *q, *y1);
        st_sub(*t1, *x1, *p);

		BigInteger *tt1 = x1;
		BigInteger *tt2 = x2;
		BigInteger *tt3 = x3;

        x1 = y1; x2 = y2; x3 = y3;
        y1 = t1; y2 = t2; y3 = t3;
		t1 = tt1; t2 = tt2; t3 = tt3;
    }

    if (x1->negative)
        st_add(*x1, *x1, n);
	
    st_copy(r, *x1);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/*
void BigInteger::sqrt(BigInteger &r, const BigInteger &x)
{
	int maxSqrtBits = ceil((bitLength(x) + 1)/2.0);

	BigInteger aMax;
	BigInteger aMin;
	BigInteger aSqr;
	BigInteger aTwo;
	BigInteger aTemp;

	assign(aMax,1);
	assign(aMin,0);
	assign(aTwo,2);

	shiftLeft(aMax,aMax,maxSqrtBits);
	div(r,aMax,aTwo);

	while(cmp(r,aMin)!=0)
	{
		mul(aSqr,r,r);

		int aCmp = cmp(aSqr,x);
		if(aCmp==0)
			return;
		else if(aCmp>0)
			copy(aMax,r);
		else 
			copy(aMin,r);
			
		add(aTemp,aMax,aMin);
		div(r,aTemp,aTwo);
	}

	
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::MontgomeryRepresentation(BigInteger &nprime, BigInteger &bsquared, const BigInteger &n)
{
	int nBits = bitLength(n);
	if(nBits%BITS!=0)
		nBits+= BITS - nBits%BITS;

	BigInteger b, binv, temp;
	
	one(b); 
	one(temp);


	shiftLeft(b,b,nBits);
	shiftLeft(temp,temp,nBits*2);

	DWORD aTick = GetTickCount();
	mod(bsquared,temp,n);
	printf("%d\n",GetTickCount()-aTick);

	inverseModN(binv,b,n);

	mul(temp,b,binv);
	div(nprime,temp,n);

	return nBits;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::MontgomeryReduce(BigInteger &r, BigInteger &workspace, const BigInteger &t, const BigInteger &n, int nBits, const BigInteger &nprime, word *work)
{	
	ModPow2(r,t,nBits);				// r = t (mod B)
	kmul(workspace,r,nprime,work);		// workspace = [t (mod B)]*N' 
	ModPow2(r,workspace,nBits);		// r = tN' (mod B)
	kmul(workspace,r,n,work);			// workspace = tN'n (mod B)
	add(r,t,workspace);				// r = t + tN'n (mod B)
	DivPow2(r,r,nBits);				// r = (t + tN'n)/r

	if(cmp(r,n)>=0) 
		sub(r,r,n);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::ModPow2(BigInteger &r, const BigInteger &t, int nBits)
{
	int firstWord = nBits/BITS;

	copy(r,t);

	if(r.len<=firstWord)
		return;

	int anOffset = nBits%BITS;
	int aMask = 0xffffffff << anOffset;
	aMask^=0xffffffff;
	
	r.n[firstWord]&=aMask;

	memset(r.n + firstWord + 1, 0, r.len - firstWord - 1);
	r.len = firstWord + 1;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::DivPow2(BigInteger &r, const BigInteger &t, int nBits)
{
	shiftRight(r,t,nBits);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::MulPow2(BigInteger &r, const BigInteger &t, int nBits)
{
	shiftLeft(r,t,nBits);
}


*/

const BigInteger BigInteger::ZERO = BigInteger(0);
const BigInteger BigInteger::ONE = BigInteger(1);
      
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::BigInteger()
{
	Init();
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::BigInteger(const BigInteger &theVal)
{
	Init();
	st_copy(*this,theVal);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::BigInteger(int theFrom)
{
	Init();

	negative = theFrom<0;
	if(negative)
		n[0] = theFrom*-1;
	else
		n[0] = theFrom;

	len = 1;
	if(theFrom==0)
		len = 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::BigInteger(const string &theString)
{
	Init();
	fromString(theString);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::BigInteger(const RawBuffer &buffer)
{
	Init();
    fromBinary(buffer);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger::BigInteger(int numBits, Random &rndSrc)
{
	Init();
	fromBinary(st_randomBits(numBits,rndSrc));
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

const BigInteger& BigInteger::operator =(const BigInteger& theVal)
{
	st_copy(*this,theVal);
	return *this;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool BigInteger::testBit(int theBit)
{
	return st_bit(*this,theBit);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger& BigInteger::setBit(int theBit)
{
	st_setBit(*this,theBit);
	return *this;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::gcd(const BigInteger &b) const
{
	BigInteger r;
	st_gcd(r,*this,b);

	return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::bitLength() const
{ 
	return st_bitLength(*this); 
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigInteger::fromString(const std::string& theString)
{

	st_zero(*this);

	int aPos = 0;
	while(aPos<theString.length() && isspace(theString[aPos]))
		aPos++;

	int lastPos = aPos;
	while(lastPos<theString.length() && isdigit(theString[lastPos]))
		lastPos++;

	string aStr = theString.substr(aPos,lastPos-aPos);


	BigInteger thousand(1000);
	BigInteger r1(0), r2(0);

	if(aStr.size()%3!=0)
	{
		string aSubStr = aStr.substr(0,aStr.size()%3);
		r1 = r2 = atoi(aSubStr.c_str());
		aPos+=aStr.size()%3;
	}
	
	while(aPos<aStr.length())
	{
		string aSubStr = theString.substr(aPos,3);

		
		BigInteger anInt = atoi(aSubStr.c_str());

		st_mul(r1,thousand,r2);
		st_add(r1,r1,anInt);

		r2 = r1;
		aPos+=3;
	}

	st_copy(*this,r1);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static char IntToHex(int theDigit)
{
	if(theDigit<=9)
		return '0' + theDigit;
	else
		return 'A' + (theDigit - 10);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

std::string BigInteger::toString() const
{
	string s;

	BigInteger N(1000);
	BigInteger r;
	BigInteger tmp = *this;

	char aBuf[20];

	while(!tmp.equals(BigInteger::ZERO))
	{
		tmp = tmp.divrem(N,r);
		sprintf(aBuf,"%03d",r.n[0]);
		s=aBuf+s;
	}


/*
    RawBuffer buffer = toByteArray();

	for(int i=buffer.length()-1; i>=0; i--)
	{
		int low = buffer[i] & 0x0f;
		int high = (buffer[i] & 0xf0) >> 4;
	
		s+=IntToHex(high);
		s+=IntToHex(low);
	}*/

    // remove leading 0's.
    int pos = 0, len = s.length();
    while ((pos < len) && (s[pos] == '0'))
        pos++;

    if (pos == len) 
	{
        return "0";
    }
	else 
	{	
        return s.substr(pos);
    }		
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::modExp(const BigInteger &power, const BigInteger &modulo) const
{
	BigInteger r;
	st_modExp(r, *this, power, modulo);

    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::modExp(const BigInteger &power, const BigInteger &modulo, const BigInteger &recip, short nb) const
{
	BigInteger r;
	st_modExp(r, *this, power, modulo,recip,nb);

    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::inverseModN(const BigInteger &n) const
{
	BigInteger r;

	st_inverseModN(r,*this, n);

    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::mod(const BigInteger &b) const
{
	BigInteger r;

	st_mod(r, *this, b);
   
	return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::mul(const BigInteger &b) const
{
	BigInteger r;
	st_mul(r, *this, b);

    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::div(const BigInteger &b) const
{
	BigInteger r;
	
	st_div(r, *this, b);

	return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::divrem(const BigInteger &a, BigInteger &rem) const
{
	BigInteger div;
	st_div(&div,&rem,*this,a);

	return div;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::add(const BigInteger &b) const
{
	BigInteger r;
	
	st_add(r, *this, b);
    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::sub(const BigInteger &b) const
{
	BigInteger r;
	st_sub(r, *this, b);

    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigInteger::cmp(const BigInteger &a) const
{
    return st_cmp(*this, a);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger& BigInteger::shiftLeft(int n)
{
	st_shiftLeft(*this, *this, (short) n);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger& BigInteger::shiftRight(int n)
{
	st_shiftRight(*this, *this, (short) n);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool BigInteger::equals(const BigInteger &b) const
{
    return (cmp(b) == 0);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::recip(int &nb) const
{
	BigInteger r;

	nb = st_recip(r,*this);
	return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::modMulRecip(const BigInteger &y, const BigInteger &m, const BigInteger &i, short nb) const
{
	BigInteger r;

	st_modMulRecip(r,*this,y,m,i,nb);
	
	return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigInteger BigInteger::negate() const
{
	BigInteger r(*this);

	r.negative = !(this->negative);
	return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool BigInteger::operator<(const BigInteger &a) const
{
	return cmp(a)<0;
}
