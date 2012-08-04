//------------------------------------------------------------------
//
//  FILE      : AbstractIO.h
//
//  PURPOSE   : Defines the CAbstractIO class.
//
//  CREATED   : 1st May 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __ABSTRACTIO_H__
#define __ABSTRACTIO_H__

#ifndef __STDLITHDEFS_H__
#include "stdlithdefs.h"
#endif

#ifndef __LITHEXCEPTION_H__
#include "lithexception.h"
#endif

#ifndef __MEMORY_H__
#include "memory.h"
#endif


// The exception that will be thrown on a read or write fail.
typedef enum
{

    MoWriteError=0,
    MoReadError=1,
    MoSeekError=2

} LithIOExceptionType;


#define LITHIO_EXCEPTION        10

class CLithIOException : public CLithException
{
    public:

        CLithIOException()
        {
            SetExceptionType(LITHIO_EXCEPTION);
        }
        
        CLithIOException(LithIOExceptionType code)
        {
            SetExceptionType(LITHIO_EXCEPTION);
            m_Code=code;
        }

        LithIOExceptionType     m_Code;

};



class CAbstractIO
{
    public:

        // Member functions

        CAbstractIO();
        ~CAbstractIO();

        virtual LTBOOL          Open(const char *pFilename, const char *pAccess)  { return TRUE; }
        virtual void            Close()                                             {}


        void                    SetUserData1(uint32 data) { m_UserData1 = data; }
        uint32                  GetUserData1()              { return m_UserData1; }


        void                    EnableExceptions(LTBOOL bEnable);
        LTBOOL                  IsExceptionsEnabled();

        // Functions to toWrite data
        virtual LTBOOL          Write(const void *pBlock, uint32 blockSize)=0;
        CAbstractIO&    operator << (unsigned short toWrite)  { Write(&toWrite, sizeof(toWrite)); return *this; }
        CAbstractIO&    operator << (short toWrite)   { Write(&toWrite, sizeof(toWrite)); return *this; }
        CAbstractIO&    operator << (unsigned char toWrite)   { Write(&toWrite, sizeof(toWrite)); return *this; }
        CAbstractIO&    operator << (char toWrite)    { Write(&toWrite, sizeof(toWrite)); return *this; }
        CAbstractIO&    operator << (float toWrite)   { Write(&toWrite, sizeof(toWrite)); return *this; }
        CAbstractIO&    operator << (double toWrite)  { Write(&toWrite, sizeof(toWrite)); return *this; }
        CAbstractIO&    operator << (unsigned int toWrite)    { Write(&toWrite, sizeof(toWrite)); return *this; }
        CAbstractIO&    operator << (int toWrite)     { Write(&toWrite, sizeof(toWrite)); return *this; }

        // Functions to read data
        virtual LTBOOL          Read(void *pBlock, uint32 blockSize)=0;
        CAbstractIO&    operator >> (long &toRead)    { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (unsigned short &toRead)  { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (short &toRead)   { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (unsigned char &toRead)   { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (char &toRead)    { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (float &toRead)   { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (double &toRead)  { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (unsigned int &toRead)    { Read(&toRead, sizeof(toRead)); return *this; }
        CAbstractIO&    operator >> (int &toRead)     { Read(&toRead, sizeof(toRead)); return *this; }

        LTBOOL                  WriteString(const char *pStr);
        LTBOOL                  ReadString(char *pStr, uint32 maxLen);

        LTBOOL                  ReadTextString(char *pStr, uint32 maxLen);

        virtual uint32          GetCurPos()=0;
        virtual uint32          GetLen()=0;

        virtual LTBOOL          SeekTo(uint32 pos)=0;

    
    protected:

        // User data stuff.
        uint32                  m_UserData1;

        // Tells whether or not it should throw exceptions.
        LTBOOL                  m_bExceptionsEnabled;

        // Throws an exception if they're enabled.
        void                    MaybeThrowIOException(LithIOExceptionType code);

};


#endif


