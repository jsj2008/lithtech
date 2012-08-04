#ifndef __LTERROR_H__
#define __LTERROR_H__

//****************************************************************************
//
//  Standard error handling and callstack tracking.
//
//  Complier Options:
//      ERROR_NOPRINT:  Disables everything in this module.
//      ERROR_NOFILTER: Disables category filter checking for error messages.
//      ERROR_NOSTACK:  Disables call stack tracking.
//
//  The only part of this component that are used in normal code are
//  these inlines/macros:
//      RGBERR
//
//      function_info
//      function_progress
//      PRINTCALLSTACK
//
//      error_Print
//      error_Printf
//
//      IFPRINTERROR
//      IFPRINTERRORRETURN
//      IFPRINTERRORNULL
//      IFPRINTERRORFALSE
//      IFPRINTERRORTRUE
//
//      break_Print
//      break_Printf
//
//      IFBREAKERROR
//      IFBREAKERRORRETURN
//      IFBREAKERRORNULL
//      IFBREAKERRORFALSE
//      IFBREAKERRORTRUE
//
//  The interfaces IError, IErrorFilter, and ICallStack are defined
//  here only for the sake of anyone wanting to create implementations
//  and should not be used directly.
//
//  The global functions error_PrintError and error_FiltersChanged is 
//  considered "public" for implementations of the interfaces, 
//  but "private" in other code.
//
//  Everything else is "private" and is only here for the sake of the 
//  "public" macros and should not be referenced directly in any manner.
//
//****************************************************************************

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

//for va_list
#ifndef __STDARG_H__
#include <stdarg.h>
#define __STDARG_H__
#endif


//****************************************************************************
//
//  Macro for making a color that is passed to print functions.
//  Takes 3 uint8's and packs them into a uint32 in the format 0x00RRGGBB
//
//****************************************************************************

//forms a uint32 out of 3 uint8's.
#define RGBERR(r, g, b) ((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))

//some predefined colors.
#define RGBERR_WHITE (RGBERR(255, 255, 255))
#define RGBERR_RED (RGBERR(255, 0, 0))
#define RGBERR_GREEN (RGBERR(0, 255, 0))
#define RGBERR_BLUE (RGBERR(0, 0, 255))
#define RGBERR_YELLOW (RGBERR(255, 255, 0))
#define RGBERR_GREY (RGBERR(128, 128, 128))


//****************************************************************************
//
//  ERROR_NOPRINT dependent code.  None of this is "public", dont peek!
//
//****************************************************************************

#if !defined(ERROR_NOPRINT)
    //
    //Macros
    //

    #define error_define_function_info(name)                                            \
        /* Define the static structure.  It always has the same progress */             \
        static SFunctionInfo __function_info__ = {name, "Start", 0};

    #define error_update_function_info_progress(string_id)                              \
        /* Store the string as the progress marker in the function info */              \
        __function_info__.progress = string_id;

    #define error_print_error(color, category, message)                                 \
        /* call the print function */                                                   \
        error_PrintError(color, category, message);

    #define error_printf_error(color, category, format, args)                           \
        /* call the print function */                                                   \
        error_PrintError(color, category, format, args);

    //a structure that gives information about a function, to be used
    //in call stack tracking and error reporting.
    struct SFunctionInfo {
        //the name of the function.
        const char *func_name;

        //current progress.
        const char *progress;

        //recursion depth.
        uint32 recursion_depth;
    };

#else //#if !defined(ERROR_NOPRINT)
    #define error_define_function_info(name)
    #define error_update_function_info_progress(string_id)
    #define error_print_error(color, category, message)
    #define error_printf_error(color, category, message, args)

#endif //#if !defined(ERROR_NOPRINT)


//****************************************************************************
//
//  ERROR_NOFILTER dependent code.  None of this is "public", dont peek!
//
//****************************************************************************

#if !defined(ERROR_NOFILTER) && !defined(ERROR_NOPRINT)
    //
    //Global variables.  We define as global so the macros can make use
    //of it in the most efficient manner.
    //

    //every time the category filters change, this variable is changed.
    extern int32 error_g_CurrentFilterVersion;

    //
    //Global functions used in category filtering.
    //

    //returns true if the given category passes the current filters.
    bool error_CheckCategory(const char *category);

    //
    //Macros
    //

    #define error_if_passed_filter(category)                                            \
        /* define cache structure to hold results of expensive filter check */          \
        static SCategoryCheckCache category_check = {-1, false};                        \
        /* Check if we need to redo the expensive filter check */                       \
        if (category_check.current_filter_version != error_g_CurrentFilterVersion) {    \
            /* do the expensive check on this category. */                              \
            category_check.category_passed = error_CheckCategory(category);             \
            /* save the filter version we just checked against */                       \
            category_check.current_filter_version = error_g_CurrentFilterVersion;       \
        }                                                                               \
        /* Print the message if the category passed */                                  \
        if (category_check.category_passed)

    //a structure that holds info about whether a category is filtered
    //by the global set of category filters.
    struct SCategoryCheckCache {
        //the filter set we compared to last.
        int32 current_filter_version;

        //true if we passed the filter check
        bool category_passed;
    };

#else //#if !defined(ERROR_NOFILTER) && !defined(ERROR_NOPRINT)
    #define error_if_passed_filter(category)

#endif //#if !defined(ERROR_NOFILTER) && !defined(ERROR_NOPRINT)


//****************************************************************************
//
//  ERROR_NOSTACK dependent code.  None of this is "public", dont peek!
//
//****************************************************************************

#if !defined(ERROR_NOSTACK) && !defined(ERROR_NOPRINT)
    //classes we use for error tracking.
    class CCallStackTracker;

    //
    //Global functions used in call stack tracking.
    //

    void error_CallStackPush(CCallStackTracker *tracker);
    void error_CallStackPop(CCallStackTracker *tracker);

    //whenever we can, we should make sure we call this and
    //make sure things we called that popped themselves off the stack are really gone.
    //We aren't sure if functions popped themselves off due to normal
    //execution, or if exception handling has caused the destructors
    //of our CCallStackTracker objects to be called.  Therefore, we dont trust
    //calls to error_CallStackPop until another function is pushed on, or
    //this confirm function is called.
    void error_CallStackConfirmTop(CCallStackTracker *tracker);

    //prints out the current callstack.
    void error_CallStackPrint(uint32 color, const char *category);

    //
    //Macros
    //

    #define error_define_callstack_tracker()                                            \
        /* Define local variable for call stack tracking */                             \
        CCallStackTracker __call_stack_tracker__(__function_info__);

    #define error_update_callstack_progress()                                           \
        /* Update the call stack tracking variable */                                   \
        __call_stack_tracker__.Progress();

    #define error_do_print_callstack(category)                                          \
        /* call the print function */                                                   \
        error_CallStackPrint(category);

    //
    //Class used to track the call stack.
    //

    class CCallStackTracker {
    public:
        inline CCallStackTracker(SFunctionInfo &function_info);
        inline ~CCallStackTracker();

        inline void Progress();

        //the info for the current function.
        SFunctionInfo &info;
    };

    //
    //CCallStackTracker inline functions
    //

    CCallStackTracker::CCallStackTracker(SFunctionInfo &function_info) : info(function_info) {
        //push this function onto the call stack
        error_CallStackPush(this);

        //increment recursion count
        info.recursion_depth++;
    }

    CCallStackTracker::~CCallStackTracker() {
        //decrement the recursion count
        info.recursion_depth--;

        //pop this function off the stack.
        error_CallStackPop(this);
    }

    void CCallStackTracker::Progress() {
        //confirm any pops that have taken place.
        error_CallStackConfirmTop(this);
    }

#else //#if !defined(ERROR_NOSTACK) && !defined(ERROR_NOPRINT)
    #define error_define_callstack_tracker()
    #define error_update_callstack_progress()
    #define error_do_print_callstack(category)

#endif //#if !defined(ERROR_NOSTACK) && !defined(ERROR_NOPRINT)


//****************************************************************************
//
//  Global functions.  Not for "public" use, dont peek!
//
//****************************************************************************

//increments the current filter version number.  This number is
//used so that any given category string needs to be compared to the
//current global filters
void error_FiltersChanged();

//prints error messages, with no filter checking.  These should be called
//only if it is certain that the given category has already been checked
//with the IErrorFilter interface.
void error_PrintError(uint32 color, const char *category, const char *message);
void error_PrintError(uint32 color, const char *category, const char *format, va_list args);


//****************************************************************************
//
//
//
//  Stuff defined below here is the "public" interface.
//
//
//
//****************************************************************************



//****************************************************************************
//
//  Function ID and callstack tracking macros.  These macros are used so that 
//  information is known about the current function when an error is generated.
//  They also do the work of keeping track of the 
//
//****************************************************************************


//macro that defines the static function info structure.
//Use it at the top of a function that is going to report error messages
//or be included in call stack tracking.
//
//Used like this:
//  void foo() {
//      function_info("foo");
//      //stuff
//  }
#define function_info(name)                                                             \
    /* Set up our function tracking variable. */                                        \
    error_define_function_info(name)                                                    \
    /* Call stack tracking. */                                                          \
    error_define_callstack_tracker();

//Macro to record the fact that a function has progressed to a certain point.
//Used like this:
//  void foo() {
//      function_info("foo");
//      //stuff
//      function_progress("got here");
//      //more stuff
//      function_progress("now here");
//      //
//  }
#define function_progress(string_id)                                                    \
    /* update our function info structure */                                            \
    error_update_function_info_progress(string_id);                                     \
    /* update the callstack */                                                          \
    error_update_callstack_progress();


//prints the current callstack, with the messages all filtered with the given category.
#define PRINTCALLSTACK(category)                                                        \
{                                                                                       \
    /* Check if our category passed the current filter set. */                          \
    error_if_passed_filter(category) {                                                  \
        /* print the error message */                                                   \
        error_do_print_callstack(category);                                             \
    }                                                                                   \
}




//****************************************************************************
//
//  Error reporting macros.  
//
//****************************************************************************


//macro that prints an error message if the category passes the current global filters
inline void error_Print(uint32 color, const char *category, const char *message) {
    // Check if our category passed the current filter set. 
    error_if_passed_filter(category) {
        // print the error message 
        error_print_error(color, category, message);
    }
}

//variation on error_Print that takes a variable number of arguments.
inline void error_Printf(uint32 color, const char *category, const char *format, ...) {
    //check if our category passed the current filter set.
    error_if_passed_filter(category) {
        //get our parameter list.
        va_list args;
        va_start(args, format);

        //print the error message
        error_printf_error(color, category, format, args);
    }
}

//macro that checks a condition and prints an error if it evaluates to true.
#define IFPRINTERROR(condition, color, category, message)                               \
    if (condition) {error_Print(color, category, message);}

//macro that checks a condition.  If true, it prints an error and returns the given value
#define IFPRINTERRORRETURN(condition, val, color, category, message)                    \
    if (condition) {error_Print(color, category, message); return val;}

//checks a condition, prints and error and returns NULL if true.
#define IFPRINTERRORNULL(condition, color, category, message)                           \
    IFPRINTERRORRETURN(condition, NULL, color, category, message)

//returns false.
#define IFPRINTERRORFALSE(condition, color, category, message)                          \
    IFPRINTERRORRETURN(condition, false, color, category, message)

//returns true.
#define IFPRINTERRORTRUE(condition, color, category, message)                           \
    IFPRINTERRORRETURN(condition, true, color, category, message)



//triggers a breakpoint and prints an error message if the category passes
//the current global filters.
inline void break_Print(uint32 color, const char *category, const char *message) {
    // Check if our category passed the current filter set.
    error_if_passed_filter(category) {
        // print the error message
        error_print_error(color, category, message);

        // trigger a breakpoint
        BREAK1();
    }
}

//variation on break_Print that takes a variable number of arguments.
inline void break_Printf(uint32 color, const char *category, const char *format, ...) {
    // Check if our category passed the current filter set.
    error_if_passed_filter(category) {
        //get our parameter list.
        va_list args;
        va_start(args, format);

        //print the error message
        error_printf_error(color, category, format, args);

        // trigger a breakpoint
        BREAK1();
    }
}

//macro that checks a condition and prints an error if it evaluates to true.
#define IFBREAKERROR(condition, color, category, message)                               \
    if (condition) {break_Print(color, category, message);}

//macro that checks a condition.  If true, it prints an error and returns the given value
#define IFBREAKERRORRETURN(condition, val, color, category, message)                    \
    if (condition) {break_Print(color, category, message); return val;}

//checks a condition, prints and error and returns NULL if true.
#define IFBREAKERRORNULL(condition, color, category, message)                           \
    IFBREAKERRORRETURN(condition, NULL, color, category, message)

//returns false.
#define IFBREAKERRORFALSE(condition, color, category, message)                          \
    IFBREAKERRORRETURN(condition, false, color, category, message)

//returns true.
#define IFBREAKERRORTRUE(condition, color, category, message)                           \
    IFBREAKERRORRETURN(condition, true, color, category, message)



//****************************************************************************
//
//  Interfaces.
//
//  All the error_* functions for error message printing, filter checking, and
//  call stack tracking call directly into these interfaces, which 
//  may be left unimplemented to enhance performance.
//
//****************************************************************************

class IErrorPrint : public IBase {
public:
    interface_version(IErrorPrint, 0);

    //error message function.
    virtual void PrintError(uint32 color, const char *category, const char *message) = 0;
    virtual void PrintError(uint32 color, const char *category, const char *format, va_list args) = 0;
};

class IErrorFilter : public IBase {
public:
    interface_version(IErrorFilter, 0);

    //checks the given category against the current filter set.
    virtual bool CheckCategory(const char *category) = 0;
};

class CCallStackTracker;

class ICallStack : public IBase {
public:
    interface_version(ICallStack, 0);

    //call stack tracking functions.  See comments for error_* functions.
    virtual void CallStackPush(CCallStackTracker *tracker) = 0;
    virtual void CallStackPop(CCallStackTracker *tracker) = 0;
    virtual void CallStackConfirmTop(CCallStackTracker *tracker) = 0;

    //Prints the current callstack, using the IError interface
    virtual void CallStackPrint(uint32 color, const char *category) = 0;
};




#endif



