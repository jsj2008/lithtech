// ------------------------------------------------------------------------- //
//
// FILE      : S T D L T E R R O R . H
//
// CREATED   : 10/22/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : 
//
// ------------------------------------------------------------------------- //

#ifndef __STDLTERROR_H__
#define __STDLTERROR_H__

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
// NONE, don't include things here, because many things include this file
// and we don't want to pull everything into each file

// ------------------------------------------------------------------------- //
// Externs
// ------------------------------------------------------------------------- //
extern class IBaseError *g_pSTDLTError;

// ------------------------------------------------------------------------- //
// Typedefs & Defines
// ------------------------------------------------------------------------- //
// short cuts so you don't have to use class reference
#define CRITICAL_ERROR(m,s) g_pSTDLTError->CriticalError(m,s)
#define RECOVERABLE_ERROR(m,s) g_pSTDLTError->RecoverableError(m,s)

enum ESTDLTResults {
	STDLT_OK = 0,
	STDLT_ERROR,
	STDLT_WARNING,

	STDLT_BEGIN,
	STDLT_END
};

// #define LTRESULT ESTDLTResults

// ------------------------------------------------------------------------- //
// Classes
// ------------------------------------------------------------------------- //

class IBaseError {
	public:
		virtual void  CriticalError (char *module, char *eStr) = 0;
		virtual void  RecoverableError (char *module, char *eStr) = 0;
};

class CNULLError : public IBaseError {
	public:
		void  CriticalError (char *module, char *eStr) {}
		void  RecoverableError (char *module, char *eStr) {}
};


#endif  // __STDLTERROR_H__

