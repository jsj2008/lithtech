

#ifndef __LIBLTINFO_H__
#define __LIBLTINFO_H__


// for the moment, Output Redirection is enabled only in a debug build
#ifdef _DEBUG
#define LIBLTINFO_OUTPUT_REDIRECTION
#endif // _DEBUG


// includes necessary for output redirection
#ifndef __ILTOUTPUTREDIR_H__
#include "libltinfo/iltoutputredir.h"
#endif


#endif // __LIBLTINFO_H__

