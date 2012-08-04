#ifndef __ICOMMANDLINEARGS_H__
#define __ICOMMANDLINEARGS_H__

//
//  Command line argument management module.
//

//lithtech module header
#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

//module interface class
class ICommandLineArgs : public IBase {
public:
    interface_version(ICommandLineArgs, 0);

    //Initializes the module.  Typically called from a program's
    //"main" function.  Pass in the system argc and argv sent to the program,
    //or anything else you feel like for that matter.  It can be called
    //as many times as you want.  Each subsequent call will cause values
    //passed in previous calls to be forgotten.
    virtual void Init(int32 argc, char **argv) = 0;

    //returns the number of arguments
    virtual uint32 Argc() = 0;

    //returns a specified argument.
    virtual const char *Argv(uint32 index) = 0;

    //searches for a "-" argument with the given name (dont include the '-')
    //and returns the arguement after it.  Returns NULL if there is 
    //no matching argument.
    virtual const char *FindArgDash(const char *arg_name) = 0;

    //same as FindArgDash, except no dash is added to the beginning of 
    //the given argument name.
    virtual const char *FindArg(const char *arg_name) = 0;
};






#endif //#ifndef __ICOMMANDLINEARGS_H__