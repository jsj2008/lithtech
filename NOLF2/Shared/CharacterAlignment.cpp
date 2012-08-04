//----------------------------------------------------------------------------
//              
//	MODULE:		CharacterAlignment.cpp
//              
//	PURPOSE:	= implementation
//              
//	CREATED:	20.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	=
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "CharacterAlignment.h"		
#include "AIUtils.h"
#include "AIAssert.h"

// Forward declarations

// Globals

// Statics

const char* RelationTraits::s_aszAITraitTypes[] =
{
	#define TRAIT_TYPE_AS_STRING 1
	#include "RelationTraitTypeEnums.h"
	#undef TRAIT_TYPE_AS_STRING
};

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::GetCrosshairRelativeToPlayer()
//              
//	PURPOSE:	Returns a selected RelationData based on our relation
//				with the player.  To summarize, this function converts two 
//				dynamic character classes into a static RelationData
//              
//----------------------------------------------------------------------------
CharacterClass GetRelativeAlignment(const RelationSet& r1, const RelationData& c2)
{
	CharacterClass CC = NEUTRAL;
	CharacterAlignment CA = GetAlignment(r1, c2);

	// If we our alignment with eachother was set, then simplify into a 
	// relations the crosshair system supports (ie RelationData)
	if ( CA != INVALID )
	{
		switch( CA )
		{
		case LIKE:
			CC = GOOD;
			break;
		case TOLERATE:
			CC = NEUTRAL;
			break;
		case HATE:
			CC = BAD;
			break;
		case UNDETERMINED:
			CC = UNKNOWN;
			break;
		default:
			UBER_ASSERT( 0, "Unknown relation" );
		};
	}

	return CC;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetAlignment
//
//	PURPOSE:	Determine how c1 feels about c2
//
// ----------------------------------------------------------------------- //
CharacterAlignment GetAlignment(const RelationSet& r1, const RelationData& c1)
{
	// First try the characterclasses' test function before dropping back to 
	// the old method.
	CharacterAlignment Alignment = r1.GetAlignment( c1 );
	if ( Alignment != INVALID )
	{
		return Alignment;
	}

	return TOLERATE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSide
//
//	PURPOSE:	Determine whether c1 is on the same side as c2
//
// ----------------------------------------------------------------------- //
CharacterSide GetSide( const RelationData& c1, const RelationData& c2)
{
	UBER_ASSERT( 0, "GetSide no longer maintained or supported" );
	return CS_NEUTRAL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	ConvertAlignmentNameToEnum()
//              
//	PURPOSE:	Converts the name of a Alignment to a alignment.
//              
//----------------------------------------------------------------------------
CharacterAlignment ConvertAlignmentNameToEnum(const char* const szName)
{
	if ( szName != NULL )
	{
		if ( 0 == strcmp(szName, "LIKE") )
		{
			return LIKE;
		}
		else if ( 0 == strcmp(szName, "HATE") )
		{
			return HATE;
		}
		else if ( 0 == strcmp(szName, "TOLERATE") )
		{
			return TOLERATE;
		}
		else if ( 0 == strcmp(szName, "UNDETERMINED") )
		{
			return UNDETERMINED;
		}
		else
		{
			AIASSERT1( 0, NULL, "Invalid Alignment Name: %s", szName );
		}
	}

	return INVALID;
}



//============================================================================



//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationTraits::RelationDescription()
//              
//	PURPOSE:	Constructor to clear the values of the relation description
//              
//----------------------------------------------------------------------------
RelationDescription::RelationDescription() : 
	eTrait( RelationTraits::kTrait_Invalid ),
	eAlignment( INVALID) 
{
	szValue[0] = '\0';
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationTraits::operator==()
//              
//	PURPOSE:	Comparison operator to handle comparing two
//				RelationDescription instances
//              
//----------------------------------------------------------------------------
bool RelationDescription::operator==(const RelationDescription& RD) const
{
	if( RD.eTrait != eTrait )
	{
		return false;
	}
	if( RD.eAlignment != eAlignment )
	{
		return false;
	}
	if ( strcmp( szValue, RD.szValue ) != 0 )
	{
		return false;
	}

	return true;
}



//============================================================================



RelationTraits::CIDValue::_mapValueToID RelationTraits::CIDValue::sm_mapValueToID;
int RelationTraits::CIDValue::sm_nID;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CIDValue::CIDValue()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
RelationTraits::CIDValue::CIDValue()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CIDValue::SetValue()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void RelationTraits::CIDValue::SetValue( const std::string& NewValue )
{
	m_Value = NewValue;
	m_ID = LookupID( m_Value );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CIDValue::CopyValueToString()
//              
//	PURPOSE:	Returns by arguement the contents of the Value in char string
//				form
//              
//----------------------------------------------------------------------------
void RelationTraits::CIDValue::CopyValueToString(char* pszOut,int nSize) const
{
	strncpy( pszOut, m_Value.c_str(), nSize-1 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationTraits::ResetValueEnumeration()
//              
//	PURPOSE:	Only safe to do when either all IDValues instances have
//				been destroyed, or when we aren't going to be using them
//				any more (level exit, etc)
//              
//----------------------------------------------------------------------------
void RelationTraits::CIDValue::ResetValueEnumeration()
{
	sm_mapValueToID.clear();
	sm_nID = 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CIDValue::GetValue()
//              
//	PURPOSE:	Returns the coorsponding ID to a value.
//              
//----------------------------------------------------------------------------
int RelationTraits::CIDValue::GetValue(void) const
{
	return m_ID;
}

void RelationTraits::CIDValue::Save(ILTMessage_Write *pMsg)
{
	char szValue[256];
	UBER_ASSERT( m_Value.length() < sizeof(szValue)-1, "Attempted to save CIDValue with a length greater than 255" );

	// Convert the std::string to an HSTRING, and save it
	strcpy( szValue, m_Value.c_str() );
	HSTRING hValue = g_pLTServer->CreateString( szValue );
	SAVE_HSTRING( hValue );
	FREE_HSTRING( hValue );
}

void RelationTraits::CIDValue::Load(ILTMessage_Read *pMsg)
{
	const char* szValue;
	HSTRING hValue;

	LOAD_HSTRING( hValue );
	szValue = g_pLTServer->GetStringData(hValue);
	SetValue( szValue );
	FREE_HSTRING( hValue );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CIDValue::GetID()
//              
//	PURPOSE:	Returns the ID corresponding to instance std::string.  If none 
//				exists, one is created such that a valid ID is always returned
//              
//----------------------------------------------------------------------------
int RelationTraits::CIDValue::LookupID( const std::string& Value)
{
	// Find Value in the map.  If it does not exist there, then
	// add it so that we always return a valid ID.
	_mapValueToID::iterator it = sm_mapValueToID.find( Value );
	if ( it == sm_mapValueToID.end() )
	{
		sm_nID++;
		sm_mapValueToID.insert( std::make_pair< std::string,int>( Value, sm_nID ) );
		return sm_nID;
	}
	else
	{
		return (*it).second;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CIDValue::CIDValue()
//              
//	PURPOSE:	Copy constructor for CIDValues
//              
//----------------------------------------------------------------------------
RelationTraits::CIDValue::CIDValue(const CIDValue& rhs)
{
	if ( this == &rhs )
		return;
	
	m_ID = rhs.m_ID;
	m_Value = rhs.m_Value;
	sm_nID = rhs.sm_nID;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CIDValue::operator=()
//              
//	PURPOSE:	Assignment operator for CIDValues
//              
//----------------------------------------------------------------------------
RelationTraits::CIDValue& RelationTraits::CIDValue::operator=(const CIDValue& rhs )
{
	if ( this == &rhs )
		return *this;

	this->m_ID =  rhs.m_ID;
	this->m_Value =  rhs.m_Value;

	return *this;
}

RelationSet::RelationSet(){}
RelationSet::~RelationSet(){}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::operator=()
//              
//	PURPOSE:	Clears and then copies the RelationSet passed in.
//              
//----------------------------------------------------------------------------
RelationSet& RelationSet::operator=(const RelationSet& rhs )
{
	int i;

	if( this == &rhs )
		return *this;

	// Clear out all the existing relationships
	for ( i=0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_RelationSet[i].clear();
	}

	// Copy all of the relationships
	for ( i=0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_RelationSet[i] = rhs.m_RelationSet[i];
	}

	return *this;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::CanAddRelation()
//              
//	PURPOSE:	Returns true if this relation can be safely added.  A relation
//				can be added if it is not already present.
//              
//----------------------------------------------------------------------------
bool RelationSet::CanAddRelation(const RelationDescription& RD) const
{
	// If we are are attempting to add a Hate relationships, see if we already
	// hate this character or not.
	return ( HasSpecificRelation(RD) == false );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::AddRelation()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void RelationSet::AddRelation(const RelationDescription& RD)
{
	// Make sure that this exact relationships is not already in the list
	// as we don't want to add a duplicate!
	UBER_ASSERT3( !HasSpecificRelation(RD), "Relation overwritting not supported for: %d, %s, %d", RD.eTrait, RD.szValue, RD.eAlignment );
	if ( !HasSpecificRelation(RD) )
	{
		GetRelationMap(RD.eTrait).insert(_mapEnumStringsToAlignment::value_type(MakeValue(RD.szValue), RD.eAlignment));
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::CanRemoveRelation()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
bool RelationSet::CanRemoveRelation(const RelationDescription& RD) const
{
	return ( HasSpecificRelation(RD) == true );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::RemoveARelation()
//              
//	PURPOSE:	Removes a relation from the RelationSet
//              
//----------------------------------------------------------------------------
void RelationSet::RemoveARelation(RelationTraits::eRelationTraits eTrait, const char* const pszValue )
{
	// Make sure that it is not already in the list
	_mapEnumStringsToAlignment::iterator it = GetRelationMap(eTrait).find( MakeValue(pszValue) );
	if ( it != GetRelationMap(eTrait).end() )
	{
		GetRelationMap(eTrait).erase( it );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::RemoveSpecificRelation()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void RelationSet::RemoveSpecificRelation(const RelationDescription& RD)
{
	// Make sure that it is not already in the list
	_mapEnumStringsToAlignment::iterator it = GetRelationMap(RD.eTrait).find( MakeValue(RD.szValue) );
	if ( it != GetRelationMap(RD.eTrait).end() )
	{
		UBER_ASSERT( (*it).second == RD.eAlignment, "RelationSet::RemoveSpecificRelation: Attempted to remove unfound relation" );
		if ( (*it).second == RD.eAlignment )
		{
			GetRelationMap(RD.eTrait).erase( it );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	_mapEnumStringsToAlignment::GetRelationMap()
//              
//	PURPOSE:	Converts a Trait to a RelationMap
//              
//----------------------------------------------------------------------------
const RelationSet::_mapEnumStringsToAlignment& RelationSet::GetRelationMap(RelationTraits::eRelationTraits eTrait) const
{
	UBER_ASSERT( eTrait >= 0 && eTrait < RelationTraits::kTrait_Count, "Enumerated Trait out of range on AddRelation" );
	return m_RelationSet[eTrait];
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	_mapEnumStringsToAlignment::GetRelationMap()
//              
//	PURPOSE:	Converts a Trait to a RelationMap
//              
//----------------------------------------------------------------------------
RelationSet::_mapEnumStringsToAlignment& RelationSet::GetRelationMap(RelationTraits::eRelationTraits eTrait)
{
	UBER_ASSERT( eTrait >= 0 && eTrait < RelationTraits::kTrait_Count, "Enumerated Trait out of range on AddRelation" );
	return m_RelationSet[eTrait];
}

const RelationTraits::_EnumString RelationSet::MakeValue(const char* const pszValue) const
{
	std::string szValue;
	szValue.assign( pszValue );
	RelationTraits::_EnumString Value;
	Value.SetValue( szValue );

	return Value;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::HasARelation()
//              
//	PURPOSE:	Function for testing to see if a relation already exists WITH
//				the same value as the relation to be deleted.
//              
//----------------------------------------------------------------------------
bool RelationSet::HasARelation(RelationTraits::eRelationTraits eTrait, const RelationTraits::_EnumString& Value ) const
{
	// Make sure that it is not already in the list
	_mapEnumStringsToAlignment::const_iterator it = GetRelationMap(eTrait).find( Value );

	// If there is no relation between this type, then there is no such relation.
	if ( it == GetRelationMap(eTrait).end() )
	{
		return false;
	}

	return true;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::HasSpecificRelation()
//              
//	PURPOSE:	Does this specific relation exist?
//              
//----------------------------------------------------------------------------
bool RelationSet::HasSpecificRelation(const RelationDescription& RD) const
{
	// Make sure that it is not already in the list
	_mapEnumStringsToAlignment::const_iterator it = GetRelationMap(RD.eTrait).find( MakeValue(RD.szValue));

	// If there is no relation between this type, then there is no such relation.
	if ( it == GetRelationMap(RD.eTrait).end() )
	{
		return false;
	}

	// If the Alignment is not the same, then it is not a match.
	if ( (*it).second != RD.eAlignment ) 
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::GetAlignment()
//              
//	PURPOSE:	Determine what RelationData 1 thinks about RelationData 2
//				in terms of CharacterAlignment (ie CC1 HATES CC2)
//              
//----------------------------------------------------------------------------
CharacterAlignment RelationSet::GetAlignment(const RelationData& Other) const
{
	//	Check the map to determine if a Relation present matches the ID
	//		Return the relation (map.second) if it does
	//		Otherwise move to the next map

	_mapEnumStringsToAlignment::const_iterator itMapToString;

	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		if ( !m_RelationSet[i].empty() )
		{
			itMapToString = m_RelationSet[i].find( Other.m_Value[i] );

			if ( itMapToString != m_RelationSet[i].end() )
			{
				// We found the first match!  Set the Result and exit
				return ( (*itMapToString).second );
			}
		}
	}

	return INVALID;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationSet::ClearRelations()
//              
//	PURPOSE:	Clears out all entries in a RelationSet.
//              
//----------------------------------------------------------------------------
void RelationSet::ClearRelations(void)
{
	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_RelationSet[i].clear();
	}
}

void RelationSet::Save(ILTMessage_Write *pMsg)
{
	// for each CharacterTrait array, save 
	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		// Save the size, so we know how many elements this RelationSet has when we load
		SAVE_INT( m_RelationSet[i].size() );

		// Save each of the _EnumString/int pairs
		_mapEnumStringsToAlignment::iterator itMapToString;
		itMapToString = m_RelationSet[i].begin();
		while( itMapToString != m_RelationSet[i].end() )
		{
			// Copy the Value, then save it
			RelationTraits::CIDValue inst = (*itMapToString).first;
			inst.Save(pMsg);

			// Save the associated relation
			SAVE_INT( (int)((*itMapToString).second) );

			itMapToString++;
		}
	}
}

void RelationSet::Load(ILTMessage_Read *pMsg)
{
	RelationTraits::_EnumString PairString;
	int PairInt;

	// for each CharacterTrait array, load 
	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		// Clear any existing relations before progressing
		m_RelationSet[i].clear();

		int iCount; 
		// Save the size, so we know how many elements this RelationSet has when we load
		LOAD_INT( iCount );

		for ( int x = 0; x < iCount; x++ )
		{
			PairString.Load(pMsg);
			LOAD_INT( PairInt );
			
			m_RelationSet[i].insert( std::make_pair( PairString, (CharacterAlignment)PairInt ) );
		}
	}
}


// Ctors/Dtors/etc
RelationData::RelationData()
{
	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_Value[i].SetValue( std::string("") );
	}
}

RelationData::~RelationData(){}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::RelationData()
//              
//	PURPOSE:	Copy constructor to handle.. copying a RelationData
//              
//----------------------------------------------------------------------------
RelationData::RelationData(const RelationData& rhs)
{
	*this = rhs;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::operator=
//              
//	PURPOSE:	Equality operator to handle assignment of one RelationData
//				to another
//              
//----------------------------------------------------------------------------
RelationData& RelationData::operator=( const RelationData& rhs  )
{
	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_Value[i] = rhs.m_Value[i];
	}

	return *this;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::Clear()
//              
//	PURPOSE:	Function to handle clearing all relations.
//              
//----------------------------------------------------------------------------
void RelationData::Clear( void )
{
	std::string szNULL("");
	for (int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_Value[i].SetValue(szNULL);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::SetValue()
//              
//	PURPOSE:	Accessor to set the a trait to a value
//              
//----------------------------------------------------------------------------
void RelationData::SetTraitValue(RelationTraits::eRelationTraits Trait, char const * const pszString )
{
	std::string szString;

	// Prevent the NULL string case == if a null char ptr is passed in,
	// then assign a null value.
	if ( pszString == NULL )
	{
		szString == "";
	}
	else
	{
		szString.assign( pszString );
	}

	m_Value[Trait].SetValue( szString );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::GetTraitValue()
//              
//	PURPOSE:	Returns the value of a relation in the form of a const string
//              
//----------------------------------------------------------------------------
void RelationData::GetTraitValue(RelationTraits::eRelationTraits eTrait, char* pszValue, int nSize) const
{
	m_Value[eTrait].CopyValueToString(pszValue, nSize);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::Save()
//              
//	PURPOSE:	Save a RelationData (by saving all of the m_Values)
//              
//----------------------------------------------------------------------------
void RelationData::Save(ILTMessage_Write *pMsg)
{
	// Save each of our _EnumStrings by calling their Save functions
	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_Value[i].Save(pMsg);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	RelationData::Load()
//              
//	PURPOSE:	Save a RelationData (by loading all of the m_Values)
//              
//----------------------------------------------------------------------------
void RelationData::Load(ILTMessage_Read *pMsg)
{
	// Restore each of our _EnumStrings by calling their Save functions
	for( int i = 0; i < RelationTraits::kTrait_Count; i++ )
	{
		m_Value[i].Load(pMsg);
	}
}


void RelationDescription::Save(ILTMessage_Write *pMsg)
{
	SAVE_BYTE( eTrait );
	SAVE_CHARSTRING( szValue );
	SAVE_BYTE( eAlignment );
}

void RelationDescription::Load(ILTMessage_Read *pMsg)
{
	LOAD_BYTE_CAST( eTrait, RelationTraits::eRelationTraits );
	LOAD_CHARSTRING( szValue, sizeof(szValue) );
	LOAD_BYTE_CAST( eAlignment, CharacterAlignment );
}

