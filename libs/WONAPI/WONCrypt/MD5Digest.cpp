#include "MD5Digest.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

MD5Digest::MD5Digest()
{
	reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MD5Digest::reset() 
{
	count = 0;
    mDigest[0] = 0x67452301;
    mDigest[1] = 0xEFCDAB89;
    mDigest[2] = 0x98BADCFE;
    mDigest[3] = 0x10325476;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

RawBuffer MD5Digest::digest()
{
	RawBuffer in = out;
	out.erase();

	return digest(in,in.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

RawBuffer MD5Digest::digest(const RawBuffer& in)
{
	update(in);
	return digest();	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

RawBuffer MD5Digest::digest(const RawBuffer& in, int pos)
{
    if (pos != 0) 
		memcpy(tmp,in.data(),pos);

    tmp[pos++] = 0x80;

    if (pos > DATA_LENGTH - 8)
    {
        while (pos < DATA_LENGTH)
            tmp[pos++] = 0;

        byte2int(tmp, 0, data, 0, DATA_LENGTH/4);
        transform(data);
        pos = 0;
    }

    while (pos < DATA_LENGTH - 8)
        tmp[pos++] = 0;

    byte2int(tmp, 0, data, 0, (DATA_LENGTH/4)-2);

    data[14] = (int) count;
    data[15] = (int) (count>>32);

    transform(data);

    RawBuffer buf(HASH_LENGTH,0);

    // Little endian
    int off = 0;
    for (int i = 0; i < HASH_LENGTH/4; ++i) {
        int d = mDigest[i];
        buf[off++] = (unsigned char) d;
        buf[off++] = (unsigned char) (d>>8);
        buf[off++] = (unsigned char) (d>>16);
        buf[off++] = (unsigned char) (d>>24);
    }
	
	reset();
    return buf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MD5Digest::update(const RawBuffer& input)
{
	update(input.data(),input.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MD5Digest::update(const void *theData, unsigned long theLen)
{
	__int64 tmp = count & 0xFFFFFFFFL;
	int offset = 0;
	const unsigned char *data = (const unsigned char*)theData;
	int len = theLen;
	
	count+=len*8;

	int num = (int)(tmp >> 3) & (DATA_LENGTH-1);

	if (num != 0)
	{
		if ((num+len) >= DATA_LENGTH)
		{
			out.append(data + offset,DATA_LENGTH - num);
			transform();
			offset += (DATA_LENGTH-num);
			len-=(DATA_LENGTH-num);
			num=0;
			// drop through and do the rest
		}
		else
		{
			out.append(data + offset,len);
			return;
		}
	}

	// we now can process the input data in blocks of blockSize
	// chars and save the leftovers to this->data.
	if (len >= DATA_LENGTH)
	{
		do
		{
			out.append(data+offset,DATA_LENGTH);
			transform();
			offset+=DATA_LENGTH;
			len-=DATA_LENGTH;
		} while (len >= DATA_LENGTH);
	}

	out.append(data+offset,len);
}	


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int MD5Digest::FF(int a,int b,int c,int d,int k,int s,int t)
{
    a += k+t+F(b,c,d);
    a = (a << s | (unsigned int)a >> -s);
    return a+b;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int MD5Digest::GG(int a,int b,int c,int d,int k,int s,int t)
{
    a += k+t+G(b,c,d);
    a = (a << s | (unsigned int)a >> -s);
    return a+b;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int MD5Digest::HH(int a,int b,int c,int d,int k,int s,int t)
{
    a += k+t+H(b,c,d);
    a = (a << s | (unsigned int)a >> -s);
    return a+b;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int MD5Digest::II(int a,int b,int c,int d,int k,int s,int t)
{
    a += k+t+I(b,c,d);
    a = (a << s | (unsigned int)a >> -s);
    return a+b;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MD5Digest::transform()
{
	ReadData();
	transform(data);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MD5Digest::transform (int M[])
{
    int a,b,c,d;

    a = mDigest[0];
    b = mDigest[1];
    c = mDigest[2];
    d = mDigest[3];

    a = FF(a,b,c,d,M[ 0], 7,0xd76aa478);
    d = FF(d,a,b,c,M[ 1],12,0xe8c7b756);
    c = FF(c,d,a,b,M[ 2],17,0x242070db);
    b = FF(b,c,d,a,M[ 3],22,0xc1bdceee);
    a = FF(a,b,c,d,M[ 4], 7,0xf57c0faf);
    d = FF(d,a,b,c,M[ 5],12,0x4787c62a);
    c = FF(c,d,a,b,M[ 6],17,0xa8304613);
    b = FF(b,c,d,a,M[ 7],22,0xfd469501);
    a = FF(a,b,c,d,M[ 8], 7,0x698098d8);
    d = FF(d,a,b,c,M[ 9],12,0x8b44f7af);
    c = FF(c,d,a,b,M[10],17,0xffff5bb1);
    b = FF(b,c,d,a,M[11],22,0x895cd7be);
    a = FF(a,b,c,d,M[12], 7,0x6b901122);
    d = FF(d,a,b,c,M[13],12,0xfd987193);
    c = FF(c,d,a,b,M[14],17,0xa679438e);
    b = FF(b,c,d,a,M[15],22,0x49b40821);

    a = GG(a,b,c,d,M[ 1], 5,0xf61e2562);
    d = GG(d,a,b,c,M[ 6], 9,0xc040b340);
    c = GG(c,d,a,b,M[11],14,0x265e5a51);
    b = GG(b,c,d,a,M[ 0],20,0xe9b6c7aa);
    a = GG(a,b,c,d,M[ 5], 5,0xd62f105d);
    d = GG(d,a,b,c,M[10], 9,0x02441453);
    c = GG(c,d,a,b,M[15],14,0xd8a1e681);
    b = GG(b,c,d,a,M[ 4],20,0xe7d3fbc8);
    a = GG(a,b,c,d,M[ 9], 5,0x21e1cde6);
    d = GG(d,a,b,c,M[14], 9,0xc33707d6);
    c = GG(c,d,a,b,M[ 3],14,0xf4d50d87);
    b = GG(b,c,d,a,M[ 8],20,0x455a14ed);
    a = GG(a,b,c,d,M[13], 5,0xa9e3e905);
    d = GG(d,a,b,c,M[ 2], 9,0xfcefa3f8);
    c = GG(c,d,a,b,M[ 7],14,0x676f02d9);
    b = GG(b,c,d,a,M[12],20,0x8d2a4c8a);

    a = HH(a,b,c,d,M[ 5], 4,0xfffa3942);
    d = HH(d,a,b,c,M[ 8],11,0x8771f681);
    c = HH(c,d,a,b,M[11],16,0x6d9d6122);
    b = HH(b,c,d,a,M[14],23,0xfde5380c);
    a = HH(a,b,c,d,M[ 1], 4,0xa4beea44);
    d = HH(d,a,b,c,M[ 4],11,0x4bdecfa9);
    c = HH(c,d,a,b,M[ 7],16,0xf6bb4b60);
    b = HH(b,c,d,a,M[10],23,0xbebfbc70);
    a = HH(a,b,c,d,M[13], 4,0x289b7ec6);
    d = HH(d,a,b,c,M[ 0],11,0xeaa127fa);
    c = HH(c,d,a,b,M[ 3],16,0xd4ef3085);
    b = HH(b,c,d,a,M[ 6],23,0x04881d05);
    a = HH(a,b,c,d,M[ 9], 4,0xd9d4d039);
    d = HH(d,a,b,c,M[12],11,0xe6db99e5);
    c = HH(c,d,a,b,M[15],16,0x1fa27cf8);
    b = HH(b,c,d,a,M[ 2],23,0xc4ac5665);

    a = II(a,b,c,d,M[ 0], 6,0xf4292244);
    d = II(d,a,b,c,M[ 7],10,0x432aff97);
    c = II(c,d,a,b,M[14],15,0xab9423a7);
    b = II(b,c,d,a,M[ 5],21,0xfc93a039);
    a = II(a,b,c,d,M[12], 6,0x655b59c3);
    d = II(d,a,b,c,M[ 3],10,0x8f0ccc92);
    c = II(c,d,a,b,M[10],15,0xffeff47d);
    b = II(b,c,d,a,M[ 1],21,0x85845dd1);
    a = II(a,b,c,d,M[ 8], 6,0x6fa87e4f);
    d = II(d,a,b,c,M[15],10,0xfe2ce6e0);
    c = II(c,d,a,b,M[ 6],15,0xa3014314);
    b = II(b,c,d,a,M[13],21,0x4e0811a1);
    a = II(a,b,c,d,M[ 4], 6,0xf7537e82);
    d = II(d,a,b,c,M[11],10,0xbd3af235);
    c = II(c,d,a,b,M[ 2],15,0x2ad7d2bb);
    b = II(b,c,d,a,M[ 9],21,0xeb86d391);

    mDigest[0] += a;
    mDigest[1] += b;
    mDigest[2] += c;
    mDigest[3] += d;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MD5Digest::ReadData()
{

	byte2int(out.data(),0,data,0,DATA_LENGTH/4);

//	for(int i=0,j=0; i<DATA_LENGTH; i+=4,j++)
//		data[j] = *(int*)(out.data() + i);

	out.erase();
}
	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void MD5Digest::byte2int(const unsigned char src[], int srcOffset,int dst[], int dstOffset, int length)
{
    while (length-- > 0)
    {
        // Little endian
        dst[dstOffset] =  (src[srcOffset++] & 0xFF);
        dst[dstOffset] |= ((int)(src[srcOffset++] & 0xFF) <<  8);
		dst[dstOffset] |= ((int)(src[srcOffset++] & 0xFF) << 16);
		dst[dstOffset] |= ((int)(src[srcOffset++] & 0xFF) << 24);
		dstOffset++;
    }
}

