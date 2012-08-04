//------------------------------------------------------------------
//
//  FILE      : ClientException.h
//
//  PURPOSE   : Defines the CClientException class and others.
//
//  CREATED   : January 15, 1997
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __CLIENTEXCEPTION_H__
#define __CLIENTEXCEPTION_H__


#define CLIENT_EXCEPTION    100

class CClientException : public CLithException
{
    public:

        CClientException(int code)
        {
            m_Code = code;
            SetExceptionType(CLIENT_EXCEPTION);
        }


        int         m_Code;

};


#endif  // __CLIENTEXCEPTION_H__



