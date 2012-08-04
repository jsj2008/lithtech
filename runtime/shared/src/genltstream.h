
// This file defines the CGenLTStream class which implements the basic
// abstract functions in ILTStream.  All Lithtech classes that implement
// ILTStream should implement it through this class.

// If a class wants to implement any of these functions, it should use the
// CGenLTStream implementations as a reference.

#ifndef __GENLTSTREAM_H__
#define __GENLTSTREAM_H__

#ifndef __ILTSTREAM_H__
#include "iltstream.h"
#endif


class CGenLTStream : public ILTStream
{
public:

    virtual LTRESULT    ReadString(char *pStr, uint32 maxBytes);
    virtual LTRESULT    WriteString(const char *pStr);
    virtual LTRESULT    WriteStream(ILTStream &dsSource, uint32 dwMin, uint32 dwMax);

};


#endif



