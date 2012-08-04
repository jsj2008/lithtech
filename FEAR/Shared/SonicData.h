// **************************************************************************** //
//
//	Project:	The Dark
//	File:		sonicdata.h
//
//	Purpose:	This is a class which manages the data associated with sonic
//				skills and properties.
//
//	Author:		Andy Mattingly
//	Date:		07/09/04
//
//	Copyright (C) 2004-2005 (Monolith Productions)
//
// **************************************************************************** //

#ifndef __SONICDATA_H__
#define __SONICDATA_H__

// **************************************************************************** //

#ifndef __SONICSDB_H__
#include "SonicsDB.h"
#endif//__SONICSDB_H__

// **************************************************************************** //

#define SONICDATA_MAXSONICGROUPS		16

// **************************************************************************** //

class SonicIntoneHandler
{
	public:

		virtual bool HandleCommand( const char** sToks, uint32 nToks, HOBJECT hSrc, HOBJECT hDest ) { return false; }
};

// **************************************************************************** //

class SonicData
{
	public:

		// ------------------------------------------------------------------------------------------- //
		// Construction and destruction

		SonicData();
		virtual ~SonicData();


		// ------------------------------------------------------------------------------------------- //
		// Updating

		void				Update( float fFrameTime );


		// ------------------------------------------------------------------------------------------- //
		// Property control

		// Sets the client for potential message passing
		void				SetClient( HCLIENT hClient );
		HCLIENT				GetClient() const;

		// Sets the record of from which to grab information from the database
		void				SetBook( HSONICBOOK hBook, bool bResetDefaults = false );
		void				SetBook( const char* sBook, bool bResetDefaults = false );
		HSONICBOOK			GetBook() const;

		// Sets the handler for intone commands
		void				SetIntoneHandler( SonicIntoneHandler* pHandler );
		SonicIntoneHandler*	GetIntoneHandler() const;

		// The breath remaining should be a zero to one value.
		void				SetBreathRemaining( float fValue );
		float				GetBreathRemaining() const;

		// A scale for the default breath recovery rates.
		void				SetBreathRecoveryScale( float fValue );
		float				GetBreathRecoveryScale() const;

		// The breath amounts should be zero to one values.
		void				SetBreathAmountScale( float fValue );
		float				GetBreathAmountScale() const;

		// Skill levels can be whatever you want.
		void				SetSkillLevel( const char* sSonic, uint8 nValue );
		uint8				GetSkillLevel( const char* sSonic ) const;
		void				SetSkillLevel( uint32 nIndex, uint8 nValue );
		uint8				GetSkillLevel( uint32 nIndex ) const;


		bool				HasEnoughBreath( const char* sSonic ) const;
		bool				HasEnoughBreath( uint32 nIndex ) const;

		// ------------------------------------------------------------------------------------------- //
		// Sonic usage interfaces

		// Returns true if the Sonic was intoned successfully.
		void				IntoneSonic( const char* sSonic, HOBJECT hSrc, HOBJECT hDest );
		void				IntoneSonic( uint32 nIndex, HOBJECT hSrc, HOBJECT hDest );


		// ------------------------------------------------------------------------------------------- //
		// Save / Load

		void				Save( ILTMessage_Write* pMsg ) const;
		void				Load( ILTMessage_Read* pMsg );


		// ------------------------------------------------------------------------------------------- //
		// Message handling

		void				HandleMessage( ILTMessage_Read* pMsg );
		void				HandleMessage( const CParsedMsg& crParsedMsg );


	protected:

		// Client data if it needs to be used
		HCLIENT				m_hClient;

		// Record from the database
		HSONICBOOK			m_hBook;

		// The command handling routines for intone commands
		SonicIntoneHandler*	m_pIntoneHandler;

		// 'Breath' values that determine whether a Sonic will be usable at any given time
		float				m_fBreathRemaining;
		float				m_fBreathRecoveryScale;
		float				m_fBreathIntoneRecoveryDelay;
		float				m_fBreathAmountScale;

		// Skill levels associated with the sonics
		uint8				m_aSkillLevels[ SONICDATA_MAXSONICGROUPS ];
};

// **************************************************************************** //

#endif//__SONICDATA_H__

