//------------------------------------------------------------------
//
//  FILE      : ServerException.h
//
//  PURPOSE   : Defines the CServerException and derivatives.
//
//  CREATED   : January 12 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __SERVEREXCEPTION_H__
#define __SERVEREXCEPTION_H__


#define SERVER_EXCEPTION    3000


class CServerException : public CLithException {
public:

    CServerException(int code, LTBOOL bShutdown) {
        SetExceptionType(SERVER_EXCEPTION);
        m_Code = code;
        m_bShutdown = bShutdown;
    }

    int m_Code;
    LTBOOL m_bShutdown;
};  


#endif  // __SERVEREXCEPTION_H__

