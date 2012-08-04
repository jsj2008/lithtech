#ifndef RezUtils_H
#define RezUtils_H

#include "stdafx.h"
#include "rezmgr.h"

// Takes a command string that can be passed into the RezCompiler, and determines
// if a specified flag is set
BOOL IsCommandSet(char cFlag, const char* pszCommnad);

// Takes exact same parameters as the ZMGR utility
// Returns the number of resources added, updated, or extracted
//
// sCmdLine refers to the commmand line options to control the compiler
// These can consist of 
// c - Create
// v - View,
// x - Extract
// f - freshen
// s - sort
// i - information
// v - Verbose
// z - Warn zero len
// l - Lower case ok
// 
// so strings can look like cl, cv, c, etc.
//
// sRezFile is the filename of the Rez file that will be created
// sTargetDir is the name of the root directory of the resource heiarchy
//
int RezCompiler(const char* sCmdLine, const char* sRezFile, const char* sTargetDir = NULL, BOOL bLithRez = FALSE, const char * sFilespec = "*.*" );

#endif