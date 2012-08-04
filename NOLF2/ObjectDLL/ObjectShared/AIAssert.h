//----------------------------------------------------------------------------
//              
//	MODULE:		AIAssert.h
//              
//	PURPOSE:	CAIAssert declaration
//              
//	CREATED:	25.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIASSERT_H__
#define __AIASSERT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#ifndef __UBER_ASSERT_H__
#include "UberAssert.h"
#endif

#ifndef __CVARTRACK_H__
#include "CVarTrack.h"
#endif

// Forward declarations

// Globals
extern CVarTrack g_vtMuteAIAssertsVar;

// Statics

#ifdef _DEBUG

	#define CALL_UBER_ASSERT( exp, description ) \
		if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) { \
			if( UberAssert( #exp, description, __LINE__, __FILE__ ) ) { \
				_asm { int 3 } \
		} }  

#else	// ndef _DEBUG

		#define CALL_UBER_ASSERT( exp, description )

#endif


#ifndef _FINAL

	void AIAssert(long nLine, char const* szFile, char const* szExp, HOBJECT hAI, char const* szDesc, ... );

	#define AIASSERT( exp, ai, desc ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) \
				UBER_ASSERT( exp, desc ); \
			AIAssert( __LINE__, __FILE__, #exp, ai, desc ); \
		}    

	#define AIASSERT0	AIASSERT

	#define AIASSERT1( exp, ai, desc, d1 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) \
				UBER_ASSERT1( exp, desc, d1 ); \
			AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1 ); \
		}    
	#define AIASSERT2( exp, ai, desc, d1, d2 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) \
				UBER_ASSERT2( exp, desc, d1, d2 ); \
			AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2 ); \
		}    
	#define AIASSERT3( exp, ai, desc, d1, d2, d3 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) \
				UBER_ASSERT3( exp, desc, d1, d2, d3 ); \
			AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2, d3 ); \
		}    
	#define AIASSERT4( exp, ai, desc, d1, d2, d3, d4 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) \
				UBER_ASSERT4( exp, desc, d1, d2, d3, d4 ); \
			AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2, d3, d4 ); \
		}    
	#define AIASSERT5( exp, ai, desc, d1, d2, d3, d4, d5 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) \
				UBER_ASSERT5( exp, desc, d1, d2, d3, d4, d5 ); \
			AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2, d3, d4, d5 ); \
		}    

#else	// def _FINAL

	#define AIASSERT( exp, ai, desc ) (void)0
	#define AIASSERT1( exp, ai, desc, d1 ) (void)0
	#define AIASSERT2( exp, ai, desc, d1, d2 ) (void)0
	#define AIASSERT3( exp, ai, desc, d1, d2, d3 ) (void)0
	#define AIASSERT4( exp, ai, desc, d1, d2, d3, d4 ) (void)0
	#define AIASSERT5( exp, ai, desc, d1, d2, d3, d4, d5 ) (void)0

#endif // _FINAL

#endif // __AIASSERT_H__

