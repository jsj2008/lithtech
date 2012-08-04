// **************************************************************************** //
//
//	Project:	The Dark
//	File:		sonicintonehandlers.h
//
//	Purpose:	These are classes which handle commands from Sonics
//
//	Author:		Andy Mattingly
//	Date:		07/23/04
//
//	Copyright (C) 2004-2005 (Monolith Productions)
//
// **************************************************************************** //

#ifndef __SONICINTONEHANDLERS_H__
#define __SONICINTONEHANDLERS_H__

// **************************************************************************** //

#ifndef __SONICDATA_H__
#include "SonicData.h"
#endif//__SONICDATA_H__

// **************************************************************************** //

class SonicIntoneHandlerDefault : public SonicIntoneHandler
{
	public:

		// Singleton declaration
		NO_INLINE static SonicIntoneHandlerDefault* Instance() { static SonicIntoneHandlerDefault sInstance; return &sInstance; }

		// The general command handler...
		virtual bool HandleCommand( const char** sToks, uint32 nToks, HOBJECT hSrc, HOBJECT hDest );


	protected:

		// Command type handlers
		bool		HandleProjectile( const char** sToks, uint32 nToks, HOBJECT hSrc, HOBJECT hDest );
		bool		HandleHeal( const char** sToks, uint32 nToks, HOBJECT hSrc, HOBJECT hDest );

};

// **************************************************************************** //

#endif//__SONICINTONEHANDLERS_H__

