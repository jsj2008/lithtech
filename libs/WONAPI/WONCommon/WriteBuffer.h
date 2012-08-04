#ifndef __WON_WRITEBUFFER_H__
#define __WON_WRITEBUFFER_H__
#include "WONShared.h"

#include "ByteBuffer.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class WriteBuffer
{
private:
	unsigned char mLengthFieldSize;
	bool mLengthIncludesLengthFieldSize;

	unsigned long mCapacity;
	unsigned long mDataLen;
	char *mData;

private:
	void CheckWrite(unsigned long theWriteLen);

	WriteBuffer(const WriteBuffer&);
	const WriteBuffer& operator=(WriteBuffer&);
	void AppendLength(unsigned long theLen, unsigned char theLengthFieldSize);

	void SlowAppendWString(const std::wstring &theWStr, unsigned char theLengthFieldSize = 2);

public:
	WriteBuffer(unsigned char theLengthFieldSize=0, bool lengthIncludesLengthFieldSize = true);
	virtual ~WriteBuffer();

	char* data() { return mData; }
	unsigned long length() const { return mDataLen; }
	unsigned long capacity() const { return mCapacity; }

	void Reset();
	void Release();
	void Reserve(unsigned long theNumBytes);
	void SetSize(unsigned long theNumBytes);

	void SkipBytes(unsigned long theLen);
	void AppendBytes(const void *theBytes, unsigned long theLen);
	void AppendByte(char theByte);
	void AppendBool(bool theBool);
	void AppendShort(short theShort);
	void AppendLong(long theLong);
	void AppendString(const std::string &theStr, unsigned char theLengthFieldSize = 2);
	void AppendWString(const std::wstring &theWStr, unsigned char theLengthFieldSize = 2);
	void AppendBuffer(const ByteBuffer* theBuffer, unsigned char theLengthFieldSize = 0);
	void AppendRawString(const std::string &theStr) { AppendString(theStr,0); }


	void SetBytes(unsigned long thePos, const void *theBytes, unsigned long theLen);
	void SetByte(unsigned long thePos, char theByte);
	void SetShort(unsigned long thePos, short theShort);
	void SetLong(unsigned long thePos, long theLong);


	ByteBufferPtr ToByteBuffer(bool release = true); // Calls Release if release is true

};

}; // namespace WONAPI

#endif
