// ------------------------------------------------------------------------- //
//
// FILE      : L I N U X S T D L T E R R O R . H
//
// CREATED   : 11/08/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : 
//
// ------------------------------------------------------------------------- //

#ifndef __LINUXSTDLTERROR_H__
#define __LINUXSTDLTERROR_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
// NONE, don't include things here, because many things include this file
// and we don't want to pull everything into each file

#include <stdio.h>
#include <stdlib.h>

// ------------------------------------------------------------------------- //
// Typedefs & Defines
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Classes
// ------------------------------------------------------------------------- //

class CLINUXError : public IBaseError {
	public:
		void  CriticalError (char *module, char *eStr) {
			#ifdef __DEBUG
			printf("%s: %s\n",module, eStr);
			exit(-1);
			#endif
		}
		void  RecoverableError (char *module, char *eStr) {
			#ifdef __DEBUG
			printf("%s: %s\n",module, eStr);
			#endif
		}
};

#endif  // __LINUXSTDLTERROR_H__

