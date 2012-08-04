//lithtech error header.
#include "lterror.h"


//****************************************************************************
//
//  ERROR_NOPRINT dependent code
//
//****************************************************************************

#if !defined(ERROR_NOPRINT)
    //holder for an implementation of IErrorPrint
    static IErrorPrint *error = NULL;
    define_holder(IErrorPrint, error);

    void error_PrintError(uint32 color, const char *category, const char *message) {
        //check our holder
        if (error == NULL) return;

        //call the IError function
        error->PrintError(color, category, message);
    }

    void error_PrintError(uint32 color, const char *category, const char *format, va_list args) {
        //check our holder
        if (error == NULL) return;

        //call the IErrorPrint function.
        error->PrintError(color, category, format, args);
    }

#else //#if !defined(ERROR_NOPRINT)
    void error_PrintError(uint32 color, const char *category, const char *message) {
        //do nothing when ERROR_NOPRINT is defined.
    }

    void error_PrintError(uint32 color, const char *category, const char *format, va_list args) {
        //do nothing when ERROR_NOPRINT is defined.
    }

#endif //#if !defined(ERROR_NOPRINT)


//****************************************************************************
//
//  ERROR_NOFILTER dependent code
//
//****************************************************************************

#if !defined(ERROR_NOFILTER) && !defined(ERROR_NOPRINT)
    //holder for an implementation of IErrorFilter
    static IErrorFilter *error_filter = NULL;
    define_holder(IErrorFilter, error_filter);

    //current filter version number.
    int32 error_g_CurrentFilterVersion = 0;

    void error_FiltersChanged() {
        //increment the filter version number
        error_g_CurrentFilterVersion++;
    }

    bool error_CheckCategory(const char *category) {
        //check our holder.
        if (error_filter == NULL) return true;

        //call IError function
        return error_filter->CheckCategory(category);
    }

#else //#if !defined(ERROR_NOFILTER) && !defined(ERROR_NOPRINT)
    void error_FiltersChanged() {
        //do nothing when ERROR_NOFILTER or ERROR_NOPRINT is defined
    }

#endif //#if !defined(ERROR_NOFILTER) && !defined(ERROR_NOPRINT)


//****************************************************************************
//
//  ERROR_NOSTACK dependent code
//
//****************************************************************************

#if !defined(ERROR_NOSTACK) && !defined(ERROR_NOPRINT)
    //holder for an implementation of ICallStack
    static ICallStack *callstack = NULL;
    define_holder(ICallStack, callstack);

    void error_CallStackPush(CCallStackTracker *tracker) {
        //check our holder.
        if (callstack == NULL) return;

        //call ICallStack function
        callstack->CallStackPush(tracker);
    }

    void error_CallStackPop(CCallStackTracker *tracker) {
        //check our holder.
        if (callstack == NULL) return;

        //call ICallStack function
        callstack->CallStackPop(tracker);
    }

    void error_CallStackConfirmTop(CCallStackTracker *tracker) {
        //check our holder.
        if (callstack == NULL) return;

        //call ICallStack function
        callstack->CallStackConfirmTop(tracker);
    }

    void error_CallStackPrint(uint32 color, const char *category) {
        //check holder
        if (callstack == NULL) return;

        //call ICallStack function
        callstack->CallStackPrint(color, category);
    }
#endif //#if !defined(ERROR_NOSTACK) && !defined(ERROR_NOPRINT)


