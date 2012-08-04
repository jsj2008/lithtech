#ifndef __ILTSTREAM_H__
#define __ILTSTREAM_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif



/*!
Helper macro to read/write data if your stream pointer is called
\b pStream.
*/

#define STREAM_READ(_x_) pStream->Read(&(_x_), sizeof(_x_));
#define STREAM_WRITE(_x_) pStream->Write(&(_x_), sizeof(_x_));


#define WRITESTREAM_DEFAULT 0xFFFFFFFF


class ILTStream {
protected:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    virtual ~ILTStream() {}
    #endif
    
public:
/*!
Call this when you're done with it (it's the same as deleting it).

Used for: Misc.
*/
    virtual void Release() = 0;

/*!
\param pData Address of buffer to read into.
\param size Length of the pData buffer.
\return \b LT_ERROR when you read past the end. 

Read in data. \em Always fills in \b pData with 0s if it returns an error so 
it can safely read everything in without checking the return value every time 
and checking ErrorStatus() at the end.

Used for: Misc.
*/
    virtual LTRESULT Read(void *pData, uint32 size) = 0;

/*!
\param pStr Address of buffer string to read into.
\param maxBytes Length of string buffer.
\return \b LT_ERROR if you read past the end. 

Read in data. \em Always fills in \b pData with 0s if it returns an error so 
it can safely read everything in without checking the return value every time 
and checking ErrorStatus() at the end.

Used for: Misc.
*/
    virtual LTRESULT ReadString(char *pStr, uint32 maxBytes) = 0;

/*!
\return \b LT_OK if there are no errors; otherwise, an error occured.

Used for: Misc.
*/
    virtual LTRESULT ErrorStatus() = 0;

/*!
\param offset Number of bytes.
\return LT_OK on success.

Seek the file pointer to the given offset.

Used for: Misc.
*/
    virtual LTRESULT SeekTo(uint32 offset) = 0;

/*!
\param offset Address of variable to be filled in.
\return LT_OK on success.

Get the current offset into the file.

Used for: Misc.
*/
    virtual LTRESULT GetPos(uint32 *offset) = 0;

/*!
\param len Address of variable to be filled in.
\return LT_OK on success.

Get the length of the file.

Used for: Misc.
*/
    virtual LTRESULT GetLen(uint32 *len) = 0;

/*!
\param dsSource Stream to read from.
\param dwMin Start position in \b dsSource.
\param dwMax End position in \b dsSource.
\return \b LT_ERROR if either stream has an ErrorStatus() != \b LT_OK, or
        returns an error from Read or Write.

Read the data in \b dsSource and write it to this stream.
You can specify the range of \b dsSource to read, or leave it at 
\b WRITESTREAM_DEFAULT and it will write \em all of \b dsSource.

Used for: Misc.
*/
    virtual LTRESULT WriteStream(ILTStream &dsSource, 
        uint32 dwMin = WRITESTREAM_DEFAULT,
        uint32 dwMax = WRITESTREAM_DEFAULT) = 0;

/*!
\return the current offset into the file.

Get the current offset into the file.

Used for: Misc.
*/
    uint32 GetPos() {
        uint32 pos;
        GetPos(&pos);
        return pos;
    }

/*!
\return the length of the file.

Get the length of the file.

Used for: Misc.
*/
    uint32 GetLen() {
        uint32 len;
        GetLen(&len);
        return len;
    }

/*!
\return A reference to the \b ILTStream instance.
\param toRead Reference to the item that is the destination for the read.

A stream operator for reading in data.

Used for:   Misc.
*/
    template<class T>
    ILTStream &operator>>(T &toRead) {
        Read(&toRead, sizeof(toRead));
        return *this;
    }

public:
/*!
\param pData Address of data buffer to write.
\param size Number of bytes to write.
\return \b LT_OK on success.


Write data of the given length to the stream.
Only enabled in writeable streams.

Used for: Misc.
*/
    virtual LTRESULT Write(const void *pData, uint32 size) = 0;

/*!
\param pStr Address of null terminated string to write.
\return \b LT_OK on success.

Write a null-terminated string to the stream.
Only enabled in writeable streams.

Used for: Misc.
*/
    virtual LTRESULT WriteString(const char *pStr) = 0;

/*!
\param toWrite Primitive to write.
\return \b LT_OK on success.

Write data to the stream. This function will only work on
non-pointer data types. It writes out the data using sizeof() to
determine how much to write.  Only enabled in writeable streams.

Used for: Misc.
*/
    template<class T>
    ILTStream &WriteVal(const T toWrite) {
        Write(&toWrite, sizeof(toWrite));
        return *this;
    }

/*!
\param toWrite Primitive to write.
\return \b LT_OK on success.

Write data to the stream. The stream operation will only work on
non-pointer data types. It writes out the data using sizeof() to
determine how much to write.  Only enabled in writeable streams.

Used for: Misc.
*/
    template<class T>
    ILTStream &operator<<(T &toWrite) {
        LTStream_Write(this, toWrite);
        return *this;
    }
};

/*!
Functions for all the default types.
*/

#define READWRITE_FN(type)                                              \
    inline void LTStream_Read(ILTStream *pStream, type &val) {          \
        pStream->Read(&val, sizeof(val));                               \
    }                                                                   \
    inline void LTStream_Write(ILTStream *pStream, const type &val) {   \
        pStream->Write(&val, sizeof(val));                              \
    }


READWRITE_FN(char);
READWRITE_FN(unsigned char);
READWRITE_FN(short);
READWRITE_FN(unsigned short);
READWRITE_FN(int);
READWRITE_FN(uint32);
READWRITE_FN(float);
READWRITE_FN(double);


#endif  //! __ILTSTREAM_H__






