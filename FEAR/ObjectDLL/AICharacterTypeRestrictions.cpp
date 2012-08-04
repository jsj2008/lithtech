// ----------------------------------------------------------------------- //
//
// MODULE  : AICharacterTypeRestrictions.cpp
//
// PURPOSE : Class implementation for an aggregate used to include restrictions
//           on character types to include or exclude.
//
// CREATED : 09/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AICharacterTypeRestrictions.h"

// ----------------------------------------------------------------------- //

CMDMGR_BEGIN_REGISTER_CLASS( CAICharacterTypeRestrictions )
CMDMGR_END_REGISTER_CLASS( CAICharacterTypeRestrictions, IAggregate )


#define MAX_CHAR_TYPE	16


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICharacterTypeRestrictions::CAICharacterTypeRestrictions()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CAICharacterTypeRestrictions::CAICharacterTypeRestrictions()
:	IAggregate			( "CAICharacterTypeRestrictions" ),
	m_dwCharTypeMask	( ALL_CHAR_TYPES )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICharacterTypeRestrictions::~CAICharacterTypeRestrictions()
//
//	PURPOSE:	Destructor - deallocate lists
//
// ----------------------------------------------------------------------- //

CAICharacterTypeRestrictions::~CAICharacterTypeRestrictions()
{
	m_dwCharTypeMask = ALL_CHAR_TYPES;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICharacterTypeRestrictions::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void CAICharacterTypeRestrictions::ReadProp(const GenericPropList *pProps)
{
	// Read the mask type.
	// Mask defines whether the specified types should be included or excluded.

	const char* pszPropString = pProps->GetString( "MaskType", "" );
	bool bMaskInclude = LTStrIEquals( pszPropString, "Include" );
	if( bMaskInclude )
	{
		m_dwCharTypeMask = 0;
	}
	else {
		m_dwCharTypeMask = ALL_CHAR_TYPES;
	}

	// Read the character types.

	char szLabel[32];
	bool bSetRequirements = false;
	ENUM_AIAttributesID eAttributesID;
	for( uint32 iType=1; iType <= MAX_CHAR_TYPE; ++iType )
	{
		sprintf( szLabel, "CharacterType%d", iType );
		pszPropString = pProps->GetString( szLabel, "" );

		// Skip "None".

		if( LTStrIEquals( pszPropString, "None" ) )
		{
			continue;
		}

		// Skip "".

		if( LTStrIEquals( pszPropString, "" ) )
		{
			continue;
		}

		// Convert the character type name to a bit flag,
		// and include or exclude it from the mask.

		bSetRequirements = true;
		eAttributesID = g_pAIDB->GetAIAttributesRecordID( pszPropString );
		if( bMaskInclude )
		{
			m_dwCharTypeMask |= ( 1 << eAttributesID );
		}
		else {
			m_dwCharTypeMask &= ~( 1 << eAttributesID );
		}
	}

	// No requirements were set, so allow everyone by default.

	if( !bSetRequirements )
	{
		m_dwCharTypeMask = ALL_CHAR_TYPES;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICharacterTypeRestrictions::Save()
//
//	PURPOSE:	Save lists
//
// ----------------------------------------------------------------------- //

void CAICharacterTypeRestrictions::Save( ILTMessage_Write *pMsg )
{
	// Only save requirements if they exist.

	bool bSetRequirements = ( m_dwCharTypeMask != ALL_CHAR_TYPES );
	SAVE_BOOL( bSetRequirements );
	if( !bSetRequirements )
	{
		return;
	}

	// Count character type bits set to true.

	uint32 cFlags = 0;
	uint32 iType;
	uint32 cCharTypes = g_pAIDB->GetNumAIAttributesRecords();
	for( iType=0; iType < cCharTypes; ++iType )
	{
		if( m_dwCharTypeMask & ( 1 << iType ) )
		{
			++cFlags;
		}
	}
	SAVE_INT( cFlags );

	// Save name for each flag.

	const char* pszName;
	AIDB_AttributesRecord* pRecord;
	for( iType=0; iType < cCharTypes; ++iType )
	{
		if( m_dwCharTypeMask & ( 1 << iType ) )
		{
			pRecord = g_pAIDB->GetAIAttributesRecord( iType );
			pszName = pRecord ? pRecord->strName.c_str() : "";
			SAVE_CHARSTRING( pszName );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICharacterTypeRestrictions::Load()
//
//	PURPOSE:	Load lists
//
// ----------------------------------------------------------------------- //

void CAICharacterTypeRestrictions::Load( ILTMessage_Read *pMsg )
{
	// Only load requirements if they exist.

	bool bSetRequirements;
	LOAD_BOOL( bSetRequirements );
	if( !bSetRequirements )
	{
		m_dwCharTypeMask = ALL_CHAR_TYPES;
	}

	// Load flags and convert from names.

	else
	{
		uint32 cFlags;
		LOAD_INT( cFlags );

		char szName[64];
		m_dwCharTypeMask = 0;
		ENUM_AIAttributesID eAttributesID;
		for( uint32 iFlag=0; iFlag < cFlags; ++iFlag )
		{
			LOAD_CHARSTRING( szName, ARRAY_LEN( szName ) );

			// Convert the character type name to a bit flag,
			// and include it in the mask.

			eAttributesID = g_pAIDB->GetAIAttributesRecordID( szName );
			m_dwCharTypeMask |= ( 1 << eAttributesID );
		}
	}
}

// Plugin statics

bool CAICharacterTypeRestrictionsPlugin::sm_bInitted = false;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICharacterTypeRestrictionsPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CAICharacterTypeRestrictionsPlugin::PreHook_EditStringList(const char* szRezPath,
												 const char* szPropName,
												 char** aszStrings,
												 uint32* pcStrings,
												 const uint32 cMaxStrings,
												 const uint32 cMaxStringLength)
{
	if ( !sm_bInitted )
	{
		sm_bInitted = true;
	}

	// Mask Type.

	if ( !LTStrICmp("MaskType", szPropName) )
	{
		strcpy( aszStrings[(*pcStrings)++], "Include" );
		strcpy( aszStrings[(*pcStrings)++], "Exclude" );

		return LT_OK;
	}

	// Character Type.

	char szBuffer[32];
	for( int iAttribute=1; iAttribute <= 16; ++iAttribute )
	{
		sprintf( szBuffer, "CharacterType%d", iAttribute );
		if ( !LTStrICmp( szBuffer, szPropName ) )
		{
			strcpy( aszStrings[(*pcStrings)++], "None" );

			AIDB_AttributesRecord* pRecord;
			int cCharTypes = g_pAIDB->GetNumAIAttributesRecords();
			for( int iType=0; iType < cCharTypes; ++iType )
			{
				pRecord = g_pAIDB->GetAIAttributesRecord( iType );
				if( pRecord )
				{
					strcpy( aszStrings[(*pcStrings)++], pRecord->strName.c_str() );
				}
			}

			return LT_OK;
		}
	}

	// Unsupported.

	return LT_UNSUPPORTED;
};
