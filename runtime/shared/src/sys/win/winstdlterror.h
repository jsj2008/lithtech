// ------------------------------------------------------------------------- //
//
// FILE      : W I N T D L T E R R O R . H
//
// CREATED   : 11/17/00
//
// AUTHOR    : Rick Lambright (based on Matt Scott's PS2 version)
//
// COPYRIGHT : LithTech Inc.  All Rights Reserved
//
// ORIGN     : lithtech 2.1 winstdlterror.h
//
// ------------------------------------------------------------------------- //

#ifndef __WINSTDLTERROR_H__
#define __WINSTDLTERROR_H__

#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif

#ifndef __ASSERT_H__
#include <assert.h>
#define __ASSERT_H__
#endif

// ------------------------------------------------------------------------- //
// Typedefs & Defines
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Classes
// ------------------------------------------------------------------------- //

class CWINError : public IBaseError 
{
	public:
		void  CriticalError (char *module, char *eStr) 
		{
			char msg [ 256 ];
			if (_snprintf(msg, sizeof(msg), "Critical Error! %s: %s\n",module, eStr) < 0)
				msg[ sizeof(msg) - 1 ] = '\0'; // force null
			printf(msg); // just in case
			_assert(msg, __FILE__, __LINE__);
			exit(-1);
		}
		void  RecoverableError (char *module, char *eStr) 
		{
			char msg [ 256 ];
			if (_snprintf(msg, sizeof(msg), "Recoverable Error. %s: %s\n",module, eStr) < 0)
				msg[ sizeof(msg) - 1 ] = '\0'; // force null
			printf(msg); // just in case
			_assert(msg, __FILE__, __LINE__);
		}
}; // CWINError

#endif  // __WINSTDLTERROR_H__

