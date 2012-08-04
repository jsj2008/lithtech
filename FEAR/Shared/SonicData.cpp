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

#include "Stdafx.h"

#ifndef __SONICDATA_H__
#include "SonicData.h"
#endif//__SONICDATA_H__

#ifndef __MSGIDS_H__
#include "MsgIDs.h"
#endif//__MSGIDS_H__

// **************************************************************************** //

SonicData::SonicData()
{
	m_hClient = NULL;
	m_hBook = NULL;
	m_pIntoneHandler = NULL;

	m_fBreathRemaining = 1.0f;
	m_fBreathRecoveryScale = 1.0f;
	m_fBreathIntoneRecoveryDelay = 0.0f;
	m_fBreathAmountScale = 1.0f;

	memset( m_aSkillLevels, 0, sizeof( uint8 ) * SONICDATA_MAXSONICGROUPS );
}

// **************************************************************************** //

SonicData::~SonicData()
{

}

// **************************************************************************** //

void SonicData::Update( float fFrameTime )
{
	SERVER_CODE
	(
		// Don't bother doing anything if we're already full
		if( m_fBreathRemaining >= 1.0f )
		{
			return;
		}

		// Increase the remaining breath based on the recovery values...
		if( m_fBreathIntoneRecoveryDelay > 0.0f )
		{
			m_fBreathIntoneRecoveryDelay -= fFrameTime;
		}
		else
		{
			float fMinRecoveryRate = g_pSonicsDB->GetFloat( m_hBook, SDB_BOOK_BREATHMINRECOVERYRATE );
			float fMaxRecoveryRate = g_pSonicsDB->GetFloat( m_hBook, SDB_BOOK_BREATHMAXRECOVERYRATE );

			float fRecoveryAmount = ( fMinRecoveryRate + ( ( 1.0f - m_fBreathRemaining ) * ( fMaxRecoveryRate - fMinRecoveryRate ) ) );
			fRecoveryAmount *= m_fBreathRecoveryScale;

			SetBreathRemaining( m_fBreathRemaining + ( fRecoveryAmount * fFrameTime ) );
		}
	)
}

// **************************************************************************** //

void SonicData::SetClient( HCLIENT hClient )
{
	m_hClient = hClient;
}

// **************************************************************************** //

HCLIENT SonicData::GetClient() const
{
	return m_hClient;
}

// **************************************************************************** //

void SonicData::SetBook( HSONICBOOK hBook, bool bResetDefaults )
{
	// If we haven't been initialized yet, then set the starting skill values
	if( !m_hBook || bResetDefaults )
	{
		uint32 nGroups = g_pSonicsDB->GetNumGroups( hBook );
		memset( m_aSkillLevels, 0, sizeof( uint8 ) * SONICDATA_MAXSONICGROUPS );

		for( uint32 i = 0; i < nGroups; ++i )
		{
			m_aSkillLevels[ i ] = g_pSonicsDB->GetDefaultSkillLevel( hBook, i );
		}
	}

	m_hBook = hBook;
}

// **************************************************************************** //

void SonicData::SetBook( const char* sBook, bool bResetDefaults )
{
	SetBook( g_pSonicsDB->GetBookRecord( sBook ), bResetDefaults );
}

// **************************************************************************** //

HSONICBOOK SonicData::GetBook() const
{
	return m_hBook;
}

// **************************************************************************** //

void SonicData::SetIntoneHandler( SonicIntoneHandler* pHandler )
{
	m_pIntoneHandler = pHandler;
}

// **************************************************************************** //

SonicIntoneHandler* SonicData::GetIntoneHandler() const
{
	return m_pIntoneHandler;
}

// **************************************************************************** //

void SonicData::SetBreathRemaining( float fValue )
{
	m_fBreathRemaining = LTCLAMP( fValue, 0.0f, 1.0f );

	SERVER_CODE
	(
		if( m_hClient )
		{
			CAutoMessage msgBreath;

			msgBreath.Writeuint8( MID_SONIC );
			msgBreath.Writeuint8( kSonicUpdateBreath );
			msgBreath.Writeuint8( ( uint8 )( m_fBreathRemaining * 255 ) );

			g_pLTServer->SendToClient( msgBreath.Read(), m_hClient, MESSAGE_GUARANTEED );
		}
	)
}

// **************************************************************************** //

float SonicData::GetBreathRemaining() const
{
	return m_fBreathRemaining;
}

// **************************************************************************** //

void SonicData::SetBreathRecoveryScale( float fValue )
{
	m_fBreathRecoveryScale = fValue;
}

// **************************************************************************** //

float SonicData::GetBreathRecoveryScale() const
{
	return m_fBreathRecoveryScale;
}

// **************************************************************************** //

void SonicData::SetBreathAmountScale( float fValue )
{
	m_fBreathAmountScale = fValue;
}

// **************************************************************************** //

float SonicData::GetBreathAmountScale() const
{
	return m_fBreathAmountScale;
}

// **************************************************************************** //

void SonicData::SetSkillLevel( const char* sSonic, uint8 nValue )
{
	SetSkillLevel( g_pSonicsDB->GetGroupIndex( m_hBook, sSonic ), nValue );
}

// **************************************************************************** //

uint8 SonicData::GetSkillLevel( const char* sSonic ) const
{
	return GetSkillLevel( g_pSonicsDB->GetGroupIndex( m_hBook, sSonic ) );
}

// **************************************************************************** //

void SonicData::SetSkillLevel( uint32 nIndex, uint8 nValue )
{
	if( ( nIndex != 0xFFFFFFFF ) && ( nIndex < SONICDATA_MAXSONICGROUPS ) )
	{
		HSONICSKILLS hSkills = g_pSonicsDB->GetSkillsRecord( m_hBook, nIndex );
		uint8 nSkillLevels = ( uint8 )g_pSonicsDB->GetNumSkillLevels( hSkills );

		m_aSkillLevels[ nIndex ] = LTMIN( nValue, nSkillLevels );

		SERVER_CODE
		(
			// Transfer skill levels to the client if needed
			if( m_hClient )
			{
				CAutoMessage msgSkills;

				msgSkills.Writeuint8( MID_SONIC );
				msgSkills.Writeuint8( kSonicUpdateSkills );
				msgSkills.WriteData( m_aSkillLevels, sizeof( uint8 ) * SONICDATA_MAXSONICGROUPS );

				g_pLTServer->SendToClient( msgSkills.Read(), m_hClient, MESSAGE_GUARANTEED );
			}
		)
	}
}

// **************************************************************************** //

uint8 SonicData::GetSkillLevel( uint32 nIndex ) const
{
	if( nIndex != 0xFFFFFFFF )
	{
		return m_aSkillLevels[ nIndex ];
	}

	return 0;
}

// **************************************************************************** //

bool SonicData::HasEnoughBreath( const char* sSonic ) const
{
	uint32 nIndex = g_pSonicsDB->GetGroupIndex( m_hBook, sSonic );
	return ( nIndex != 0xFFFFFFFF ) && HasEnoughBreath( nIndex );
}

// **************************************************************************** //

bool SonicData::HasEnoughBreath( uint32 nIndex ) const
{
	// If the skill level isn't valid, we shouldn't be calling this!
	if( m_aSkillLevels[ nIndex ] < 1 )
	{
		LTERROR( "SonicData::HasEnoughBreath() -- Invalid skill level. A skill of zero represents 'not usable'." );
		return false;
	}

	HSONICSKILLS hSkills = g_pSonicsDB->GetSkillsRecord( m_hBook, nIndex );
	uint32 nSkillIndex = ( m_aSkillLevels[ nIndex ] - 1 );

	float fBreathUsed = g_pSonicsDB->GetBreathAmount( hSkills, nSkillIndex );
	return ( m_fBreathRemaining >= ( fBreathUsed * m_fBreathAmountScale ) );
}

// **************************************************************************** //

void SonicData::IntoneSonic( const char* sSonic, HOBJECT hSrc, HOBJECT hDest )
{
	uint32 nIndex = g_pSonicsDB->GetGroupIndex( m_hBook, sSonic );
	if( nIndex != 0xFFFFFFFF )
	{
		IntoneSonic( nIndex, hSrc, hDest );
	}
}

// **************************************************************************** //

void SonicData::IntoneSonic( uint32 nIndex, HOBJECT hSrc, HOBJECT hDest )
{
	// If the skill level isn't valid, we shouldn't be calling this!
	if( m_aSkillLevels[ nIndex ] < 1 )
	{
		LTERROR( "SonicData::IntoneSonic() -- Attempted to intone a sonic without a valid skill value. A skill of zero represents 'not usable'." );
		return;
	}

	CLIENT_CODE
	(
		// Send a message to the server, telling us to intone the sonic...
		CAutoMessage msgIntone;

		msgIntone.Writeuint8( MID_SONIC );
		msgIntone.Writeuint8( kSonicIntone );
		msgIntone.Writeuint8( ( uint8 )nIndex );
		msgIntone.WriteObject( hSrc );
		msgIntone.WriteObject( hDest );

		g_pLTClient->SendToServer( msgIntone.Read(), MESSAGE_GUARANTEED );
	)

	SERVER_CODE
	(
		// Reduce our remaining breath
		HSONICSKILLS hSkills = g_pSonicsDB->GetSkillsRecord( m_hBook, nIndex );
		uint32 nSkillIndex = ( m_aSkillLevels[ nIndex ] - 1 );

		float fBreathUsed = g_pSonicsDB->GetBreathAmount( hSkills, nSkillIndex );
		SetBreathRemaining( m_fBreathRemaining - ( fBreathUsed * m_fBreathAmountScale ) );

		// Reset the recovery delay
		m_fBreathIntoneRecoveryDelay = g_pSonicsDB->GetFloat( m_hBook, SDB_BOOK_BREATHINTONERECOVERYDELAY );

		// Fire off the commands for the Sonic
		LTASSERT( m_pIntoneHandler, "SonicData::IntoneSonic() -- Intone handler not set!" );

		if( m_pIntoneHandler )
		{
			// Fill in a buffer for parsing
			char sCommandLine[ 256 ];
			LTStrCpy( sCommandLine, g_pSonicsDB->GetIntoneCommand( hSkills, nSkillIndex ), 256 );

			char* sTracker = sCommandLine;
			char* sToks[ 64 ];
			uint32 nToks = 0;

			while( *sTracker != '\0' )
			{
				// Skip starting spaces
				while( isspace( *sTracker ) )
				{
					++sTracker;
				}

				// Verify we didn't get to the end already
				if( *sTracker == '\0' )
				{
					continue;
				}

				// Handle quote blocks being handled as one token
				if( *sTracker == '"' )
				{
					++sTracker;
					sToks[ nToks++ ] = sTracker;

					while( *sTracker != '\"' )
					{
						if( *sTracker == '\0' )
						{
							LTERROR( "SonicData::IntoneSonic() -- Command parsing resulted in an invalid quote block!" );
						}

						++sTracker;
					}

					*sTracker = '\0';
					++sTracker;

					continue;
				}

				// Check for odd semi colon
				if( *sTracker == ';' )
				{
					LTERROR( "SonicData::IntoneSonic() -- Command line has an empty command!" );

					++sTracker;
					continue;
				}

				// Find the token ending
				sToks[ nToks++ ] = sTracker;

				while( !isspace( *sTracker ) )
				{
					if( *sTracker == '\0' )
					{
						break;
					}

					if( *sTracker == ';' )
					{
						*sTracker = '\0';
						++sTracker;

						if( nToks > 0 )
						{
							if( !m_pIntoneHandler->HandleCommand( ( const char** )sToks, nToks, hSrc, hDest ) )
							{
								LTERROR( "SonicData::IntoneSonic() -- Command not handled!" );
							}

							nToks = 0;
						}

						continue;
					}

					++sTracker;
				}

				*sTracker = '\0';
				++sTracker;
			}

			// If we have any tokens left over... use 'em!
			if( nToks > 0 )
			{
				if( !m_pIntoneHandler->HandleCommand( ( const char** )sToks, nToks, hSrc, hDest ) )
				{
					LTERROR( "SonicData::IntoneSonic() -- Command not handled!" );
				}
			}
		}
	) // SERVER_CODE
}

// **************************************************************************** //

void SonicData::Save( ILTMessage_Write* pMsg ) const
{
	// Write the basic data
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hBook );
	pMsg->Writefloat( m_fBreathRemaining );
	pMsg->Writefloat( m_fBreathRecoveryScale );
	pMsg->Writefloat( m_fBreathIntoneRecoveryDelay );
	pMsg->Writefloat( m_fBreathAmountScale );
	pMsg->WriteData( m_aSkillLevels, sizeof( uint8 ) * SONICDATA_MAXSONICGROUPS );
}

// **************************************************************************** //

void SonicData::Load( ILTMessage_Read* pMsg )
{
	// Read the basic data
	m_hBook = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pSonicsDB->GetBookCategory() );
	m_fBreathRemaining = pMsg->Readfloat();
	m_fBreathRecoveryScale = pMsg->Readfloat();
	m_fBreathIntoneRecoveryDelay = pMsg->Readfloat();
	m_fBreathAmountScale = pMsg->Readfloat();
	pMsg->ReadData( m_aSkillLevels, sizeof( uint8 ) * SONICDATA_MAXSONICGROUPS );
}

// **************************************************************************** //

void SonicData::HandleMessage( ILTMessage_Read* pMsg )
{
	// Get the type of Sonic message that this represents
	SonicMessage eMessage = ( SonicMessage )pMsg->Readuint8();

	switch( eMessage )
	{
		case kSonicUpdateBreath:
		{
			SetBreathRemaining( pMsg->Readuint8() / 255.0f );
			break;
		}

		case kSonicUpdateSkills:
		{
			pMsg->ReadData( m_aSkillLevels, sizeof( uint8 ) * SONICDATA_MAXSONICGROUPS );
			break;
		}

		case kSonicIntone:
		{
			uint8 nIndex = pMsg->Readuint8();
			HOBJECT hSrc = pMsg->ReadObject();
			HOBJECT hDest = pMsg->ReadObject();

			IntoneSonic( nIndex, hSrc, hDest );
			break;
		}

		default:
		{
			LTERROR( "CPlayerMgr::HandleMsgSonic() -- Sonic message type not handled!" );
			break;
		}
	}
}

// **************************************************************************** //

void SonicData::HandleMessage( const CParsedMsg& crParsedMsg )
{
	// Must have some parameters
	if( crParsedMsg.GetArgCount() < 2 )
	{
		return;
	}

	// Figure out which command we want to handle
	const char* sCommand = crParsedMsg.GetArg( 1 ).c_str();

	if( !LTStrICmp( sCommand, "SkillInc" ) )
	{
		if( ( crParsedMsg.GetArgCount() >= 3 ) && ( crParsedMsg.GetArgCount() <= 4 ) )
		{
			const char* sGroup = crParsedMsg.GetArg( 2 ).c_str();
			uint32 nIndex = g_pSonicsDB->GetGroupIndex( m_hBook, sGroup );

			if( crParsedMsg.GetArgCount() == 4 )
			{
				SetSkillLevel( nIndex, GetSkillLevel( nIndex ) + atoi( crParsedMsg.GetArg( 3 ).c_str() ) );
			}
			else
			{
				SetSkillLevel( nIndex, GetSkillLevel( nIndex ) + 1 );
			}
		}
	}
	else if( !LTStrICmp( sCommand, "SkillDec" ) )
	{
		if( ( crParsedMsg.GetArgCount() >= 3 ) && ( crParsedMsg.GetArgCount() <= 4 ) )
		{
			const char* sGroup = crParsedMsg.GetArg( 2 ).c_str();
			uint32 nIndex = g_pSonicsDB->GetGroupIndex( m_hBook, sGroup );

			if( crParsedMsg.GetArgCount() == 4 )
			{
				SetSkillLevel( nIndex, GetSkillLevel( nIndex ) - atoi( crParsedMsg.GetArg( 3 ).c_str() ) );
			}
			else
			{
				SetSkillLevel( nIndex, GetSkillLevel( nIndex ) - 1 );
			}
		}
	}
	else if( !LTStrICmp( sCommand, "SkillSet" ) )
	{
		if( crParsedMsg.GetArgCount() == 4 )
		{
			const char* sGroup = crParsedMsg.GetArg( 2 ).c_str();
			uint32 nIndex = g_pSonicsDB->GetGroupIndex( m_hBook, sGroup );

			SetSkillLevel( nIndex, atoi( crParsedMsg.GetArg( 3 ).c_str() ) );
		}
	}
	else if( !LTStrICmp( sCommand, "BreathInc" ) )
	{
		if( crParsedMsg.GetArgCount() == 3 )
		{
			SetBreathRemaining( GetBreathRemaining() + ( float )atof( crParsedMsg.GetArg( 2 ).c_str() ) );
		}
	}
	else if( !LTStrICmp( sCommand, "BreathDec" ) )
	{
		if( crParsedMsg.GetArgCount() == 3 )
		{
			SetBreathRemaining( GetBreathRemaining() - ( float )atof( crParsedMsg.GetArg( 2 ).c_str() ) );
		}
	}
	else if( !LTStrICmp( sCommand, "BreathSet" ) )
	{
		if( crParsedMsg.GetArgCount() == 3 )
		{
			SetBreathRemaining( ( float )atof( crParsedMsg.GetArg( 2 ).c_str() ) );
		}
	}
}

