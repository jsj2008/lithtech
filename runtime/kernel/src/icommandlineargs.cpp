//our interface header.
#include "icommandlineargs.h"

//for NULL, etc
#include <stdlib.h>

//for strlen, etc
#include <string.h>

//buffer template
#include "ltt_buffer.h"

//for string helper functions
#include "helpers.h"

#include "ltmem.h"

//****************************************************************************************
//
//macros that will eventually be moved to some shared header file.
//
//****************************************************************************************

    //delete macros.
    #define delca(var) if (var != NULL) {delete [] var; var = NULL;}
    #define delc(var) if (var != NULL) {delete var; var = NULL;}


//****************************************************************************************
//
//  Our implementation class
//
//****************************************************************************************

class ICommandLineArgsCommonImp : public ICommandLineArgs {
public:
    declare_interface(ICommandLineArgsCommonImp);

    //constructor/destructor
    ICommandLineArgsCommonImp();
    ~ICommandLineArgsCommonImp();

    //the interface functions.
    void Init(int32 argc, char **argv);
    uint32 Argc();
    const char *Argv(uint32 index);
    const char *FindArgDash(const char *arg_name);
    const char *FindArg(const char *name);

private:
    //buffer that contains all arguments separated by '\0' characters
    char *all_arguments;

    //the number of arguments
    uint32 argc;

    //array of pointers to the arguments.
    char **argv;
};

//register our implementation
define_interface(ICommandLineArgsCommonImp, ICommandLineArgs);


//****************************************************************************************
//
//Implementation member definitions.
//
//****************************************************************************************

ICommandLineArgsCommonImp::ICommandLineArgsCommonImp() {
    //initialize our members
    all_arguments = NULL;
    argc = 0;
    argv = NULL;
}

ICommandLineArgsCommonImp::~ICommandLineArgsCommonImp() {
    //delete our buffers and arrays
    delc(all_arguments);
    delca(argv);

    //we have no arguments.
    argc = 0;
}

void ICommandLineArgsCommonImp::Init(int32 argc, char **argv) {
    //delete old data
    if (all_arguments) delc(all_arguments);
    if (this->argv)    delca(this->argv);
    this->argc = 0;

    //check parameters
    if (argv == NULL) return;
    
    //check if there are any arguments
    if (argc < 1) return;

    //get the combined length of all the arguments
    int32 total_length = 0;
	int32 i;
	for (i = 0; i < argc; i++) {
        //add in the length of this argument
        total_length += strlen(argv[i]) + 1;
    }

    //allocate the buffer to store all arguments
    LT_MEM_TRACK_ALLOC(all_arguments = new char[total_length],LT_MEM_TYPE_MISC);

    //allocate our argv
    LT_MEM_TRACK_ALLOC(this->argv = new char *[argc],LT_MEM_TYPE_MISC);

    //copy the arguments in and set our pointers.
    char *cur_pos = &all_arguments[0];
    for (i = 0; i < argc; i++) {
        //set the argv pointer
        this->argv[i] = cur_pos;

        //copy the argument.
        strcpy(this->argv[i], argv[i]);

        //advance the cur_pos pointer
        cur_pos += strlen(argv[i]) + 1;
    }

    //save the number of arguments.
    this->argc = argc;
}

uint32 ICommandLineArgsCommonImp::Argc() {
    //return the number of arguments we have
    return argc;
}

const char *ICommandLineArgsCommonImp::Argv(uint32 index) {
    //check index
    if (index >= argc) return NULL;

    //return the argument
    return argv[index];
}

const char *ICommandLineArgsCommonImp::FindArgDash(const char *name) {
    IFBREAKNULL(name == NULL);

    //form the new argument name with a dash in front.
    char with_dash[256];
    LTSNPrintF(with_dash, sizeof(with_dash), "-%s", name); 

    //call findarg
    return FindArg(with_dash);
}

const char *ICommandLineArgsCommonImp::FindArg(const char *name) {
    IFBREAKNULL(name == NULL);

    //search through the arguments we have
    for (uint32 i = 0; i < argc; i++) {
        //check if this is the argument we are looking for.
        if (CHelpers::UpperStrcmp(name, argv[i])) {
            //this is the argument we are looking for.
            //check if there is an argument after this
            if (i + 1 >= argc) {
                //no more arguments, return empty string.
                return "";
            }

            //return the next argument.
            return argv[i + 1];
        }
    }

    //could not find the argument.
    return NULL;
}




