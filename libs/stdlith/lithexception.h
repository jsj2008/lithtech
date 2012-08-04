#ifndef __LITHEXCEPTION_H__
#define __LITHEXCEPTION_H__


#define LITH_EXCEPTION      0
#define LITH_MEMEXCEPTION   1


// Use these instead of regular try and catch.
#ifdef STDLITH_MFC
    #define LithTry         try { try
    #define LithCatch       catch(CMemoryException *__memory_exception) \
                            { __memory_exception->Delete(); throw CLithMemException(); } } catch
    #define LithExtraCatch  catch
#else
    #define LithTry         try
    #define LithCatch       catch
    #define LithExtraCatch  catch
#endif                                               



extern int g_LithExceptionType;


// This is the base exception class for all StdLith exceptions, and
// you can derive your own exception type from it if you want.
class CLithException
{
    public:

                    CLithException()                { SetExceptionType(LITH_EXCEPTION); }

        void        SetExceptionType(int type)    { g_LithExceptionType = type; }
        int         GetExceptionType()              { return g_LithExceptionType; }

};


// Use this for your memory allocation exceptions.
class CLithMemException : public CLithException
{
    public:
        
                    CLithMemException()     { SetExceptionType(LITH_MEMEXCEPTION); }

};


#endif  // __LITHEXCEPTION_H__

