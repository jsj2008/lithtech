// ----------------------------------------------------------------------- //
//
// MODULE  : UberAssert.h
//
// PURPOSE : More powerful assert declaration:
//           - breaks at the line of code that called assert.
//           - gives a plain english explaination, along with the expression.
//           - shows a stack trace.
//           - copies output to windows clipboard.
//
// CREATED : 6/27/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __UBER_ASSERT_H__
#define __UBER_ASSERT_H__

#ifdef _DEBUG

	extern bool UberAssert( long nLine, char const* szFile, char const* szExp, char const* szDesc, ... );

	#define UBER_ASSERT( exp, description ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, description )) \
			{ \
				__debugbreak(); \
			} \
		}   

	#define UBER_ASSERT0	UBER_ASSERT

	#define UBER_ASSERT1( exp, desc, d1 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1 )) \
			{ \
				__debugbreak(); \
			} \
		}   
	#define UBER_ASSERT2( exp, desc, d1, d2 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2 )) \
			{ \
				__debugbreak(); \
			} \
		}   
	#define UBER_ASSERT3( exp, desc, d1, d2, d3 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2, d3 )) \
			{ \
				__debugbreak(); \
			} \
		}   
	#define UBER_ASSERT4( exp, desc, d1, d2, d3, d4 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2, d3, d4 )) \
			{ \
				__debugbreak(); \
			} \
		}   
	#define UBER_ASSERT5( exp, desc, d1, d2, d3, d4, d5 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2, d3, d4, d5 )) \
			{ \
				__debugbreak(); \
			} \
		}   

#else	// ndef _DEBUG

	#define UBER_ASSERT( exp, desc )		(void)0
	#define UBER_ASSERT1( exp, desc, d1 )		(void)0
	#define UBER_ASSERT2( exp, desc, d1, d2 )		(void)0
	#define UBER_ASSERT3( exp, desc, d1, d2, d3 )		(void)0
	#define UBER_ASSERT4( exp, desc, d1, d2, d3, d4 )		(void)0
	#define UBER_ASSERT5( exp, desc, d1, d2, d3, d4, d5 )		(void)0

#endif


#endif // _UBER_ASSERT_H_

