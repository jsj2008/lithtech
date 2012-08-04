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


// Includes

#ifndef __VARTRACK_H__
#include "VarTrack.h"
#endif

// Forward declarations

// Globals
extern VarTrack g_vtMuteAIAssertsVar;

// Statics

#ifndef _FINAL

	void AIAssert(long nLine, char const* szFile, char const* szExp, HOBJECT hAI, char const* szDesc, ... );

	#define AIASSERT( exp, ai, desc ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) { \
				LTASSERT( exp, desc ); \
				AIAssert( __LINE__, __FILE__, #exp, ai, desc ); } \
		}    

	#define AIASSERT0	AIASSERT

	#define AIASSERT1( exp, ai, desc, d1 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) { \
				LTASSERT_PARAM1( exp, desc, d1 ); \
				AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1 ); } \
		}    
	#define AIASSERT2( exp, ai, desc, d1, d2 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) { \
				LTASSERT_PARAM2( exp, desc, d1, d2 ); \
				AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2 ); } \
		}    
	#define AIASSERT3( exp, ai, desc, d1, d2, d3 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) { \
				LTASSERT_PARAM3( exp, desc, d1, d2, d3 ); \
				AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2, d3 ); } \
		}    
	#define AIASSERT4( exp, ai, desc, d1, d2, d3, d4 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) { \
				LTASSERT_PARAM4( exp, desc, d1, d2, d3, d4 ); \
				AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2, d3, d4 ); } \
		}    
	#define AIASSERT5( exp, ai, desc, d1, d2, d3, d4, d5 ) \
		if ( !(exp) ) { \
			if( g_vtMuteAIAssertsVar.GetFloat() == 0.f ) { \
				LTASSERT_PARAM5( exp, desc, d1, d2, d3, d4, d5 ); \
				AIAssert( __LINE__, __FILE__, #exp, ai, desc, d1, d2, d3, d4, d5 ); } \
		}    

#else	// def _FINAL

	#define AIASSERT( exp, ai, desc ) (void)0;
	#define AIASSERT1( exp, ai, desc, d1 ) (void)0;
	#define AIASSERT2( exp, ai, desc, d1, d2 ) (void)0;
	#define AIASSERT3( exp, ai, desc, d1, d2, d3 ) (void)0;
	#define AIASSERT4( exp, ai, desc, d1, d2, d3, d4 ) (void)0;
	#define AIASSERT5( exp, ai, desc, d1, d2, d3, d4, d5 ) (void)0;

#endif // _FINAL

#endif // __AIASSERT_H__

