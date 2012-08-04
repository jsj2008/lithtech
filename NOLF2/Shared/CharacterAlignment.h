// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAlignment.h
//
// PURPOSE : Character alignment
//
// CREATED : 12/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_ALIGNMENT_H__
#define __CHARACTER_ALIGNMENT_H__

#pragma warning (disable : 4786)
#include <string>
#include <map>
#include <vector>



// TODO: May want to convert CharacterAlignment to Jeffs macro expansion?
enum CharacterAlignment { LIKE=0, TOLERATE, HATE, UNDETERMINED, INVALID };
enum CharacterSide { CS_ENEMY = 0, CS_FRIEND, CS_NEUTRAL };
enum CharacterClass   	{ UNKNOWN=0, GOOD, BAD, NEUTRAL };

class RelationData;
class RelationSet;
class RelationTraits;

const int RELATION_VALUE_LENGTH =	128;

//----------------------------------------------------------------------------
//              
//	CLASS:		RelationTraits
//              
//	PURPOSE:	Used by different classes to list the types of Relations and 
//				Traits currently supported
//              
//----------------------------------------------------------------------------
class RelationTraits
{
public:
	//----------------------------------------------------------------------------
	//              
	//	CLASS:		CIDValue
	//              
	//	PURPOSE:	Provides cheap equality testing for std::string by keeping an 
	//				enumeration of all instances of std::string. 
	//
	//				Example where T = inst std::string
	//				All instances with T == "some string" have an equivelant ID
	//				All instances with T != "some stirng" have a different ID
	//              
	//----------------------------------------------------------------------------
	class CIDValue
	{
		public:
			// Public members

			// Ctors/Dtors/etc
			CIDValue();
			~CIDValue() {}

			int		GetValue(void) const;
			void	CopyValueToString( char* pszOut, int nSize ) const;
			void	SetValue( const std::string& tNewValue );

			// Only safe to do when either all IDValues instances have
			// been destroyed, or when we aren't going to be using them
			// any more (level exit, etc)
			static	void ResetValueEnumeration();

			void Save(ILTMessage_Write *pMsg);
			void Load(ILTMessage_Read *pMsg);

			CIDValue(const CIDValue& rhs);
			CIDValue& operator=(const CIDValue& rhs );

			typedef std::map< std::string, int > _mapValueToID;

			struct Less_EnumValue
			{
				bool operator()(const CIDValue& lhs, const CIDValue& rhs) const
				{
					return lhs.GetValue() < rhs.GetValue();
				}
			};

		protected:
			// Protected members

		private:
			// Private members

			int LookupID( const std::string& Value );

			// Actual value of the instance, currently used only for 
			// debugging information after the ID is generated.
			std::string	m_Value;
			
			// ID value which represents the Value and can be used for 
			// cheap equality testing
			int		m_ID;

			// Map to track all instances of T to relate them to an ID.
			// Stores T/ID pairs such that if an there is an instance of 
			// T in the map, the corresponding ID can be found
			static _mapValueToID sm_mapValueToID;
		
			// Incremental tracking var to garentee unique IDs are
			// generated for all different instances of type _Value
			static int sm_nID;
	};

	
	// typedefs shared by the RelationData and the 
	// RelationSet class
	typedef CIDValue _EnumString;


	// Publicly enumerated Relation types so that the Relations and 
	// RelationData can be properly initialized.

	//
	// ENUM: Types of AI Traits
	//
	
	enum eRelationTraits
	{
		kTrait_Invalid = -1,

		#define TRAIT_TYPE_AS_ENUM 1
		#include "RelationTraitTypeEnums.h"
		#undef TRAIT_TYPE_AS_ENUM
	};

	//
	// STRINGS: const strings for AI sound types.
	//
	static const char* s_aszAITraitTypes[];
};


//----------------------------------------------------------------------------
//              
//	CLASS:		RelationDescription
//              
//	PURPOSE:	Helper struct consisting of a trait, value, alignment triple.
//				Added too late to become the core of the relationset, but
//				still a valuable helper for initializations and references.
//              
//----------------------------------------------------------------------------
struct RelationDescription
{
	RelationDescription();
	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);
	bool operator==( const RelationDescription& RD ) const;

	RelationTraits::eRelationTraits eTrait;
	char							szValue[RELATION_VALUE_LENGTH];
	CharacterAlignment				eAlignment;
};

//----------------------------------------------------------------------------
//              
//	CLASS:		RelationSet
//              
//	PURPOSE:	Set of Relations stored to determine how X feels about Y.  
//				RelationSets have lists of relationships categorized into
//				Traits used to match against Relation Data
//              
//----------------------------------------------------------------------------
class RelationSet
{
public:
	RelationSet();
	~RelationSet();
	RelationSet& operator=(const RelationSet& rhs );

	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

	// Modifying methods:
	void AddRelation( const RelationDescription& RD );
	void RemoveSpecificRelation( const RelationDescription& RD );
	void RemoveARelation( RelationTraits::eRelationTraits eTrait,  const char* const pszValue );

	void ClearRelations(void);

	// Non Modifying methods:
	CharacterAlignment GetAlignment( const RelationData& Other ) const;
	bool CanAddRelation( const RelationDescription& RD ) const;
	bool CanRemoveRelation( const RelationDescription& RD ) const;
	
private:
	typedef std::map<RelationTraits::_EnumString, CharacterAlignment, RelationTraits::CIDValue::Less_EnumValue> _mapEnumStringsToAlignment;

	// Methods:

	// Modifying:
	_mapEnumStringsToAlignment& GetRelationMap(RelationTraits::eRelationTraits eTrait);

	// Non Modifying methods:
	bool HasARelation(RelationTraits::eRelationTraits eTrait,
		const RelationTraits::_EnumString& Value ) const;

	bool HasSpecificRelation(const RelationDescription& RD ) const;
	
	const _mapEnumStringsToAlignment& GetRelationMap(RelationTraits::eRelationTraits eTrait) const;
	const RelationTraits::_EnumString MakeValue(const char* const pszValue) const;

	// Data:

	// List of all relationsets (local values likst Name/Alliance + local
	// relations (this HATES/LOVES <ID#>).
	_mapEnumStringsToAlignment m_RelationSet[RelationTraits::kTrait_Count];
};

//----------------------------------------------------------------------------
//              
//	CLASS:		RelationData
//              
//	PURPOSE:	Set of Data stored to define what X is.  RelationSets use   
//				this to determine relationships.
//              
//----------------------------------------------------------------------------
class RelationData
{
	public:
		// Public members
		friend class RelationSet;

		// Ctors/Dtors/etc
		RelationData();
		~RelationData();
		
		RelationData(const RelationData& rhs);
		RelationData& operator=(const RelationData& rhs );
		
		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		void Clear();
		void SetTraitValue( RelationTraits::eRelationTraits, const char* const pszString );
		void GetTraitValue( RelationTraits::eRelationTraits, char* pszOut, int nSize ) const;

	protected:
		// Protected members

	private:
		// Private members
		
		// Array of Values corresponding to the Traits enumeration.  Must
		// be listed in the enumerations order or they will be out of sync
		// with the RelationSet look up.
		RelationTraits::_EnumString m_Value[RelationTraits::kTrait_Count];
};

CharacterAlignment ConvertAlignmentNameToEnum(const char* const szName);
CharacterAlignment GetAlignment(const RelationSet& r1, const RelationData& c2);
CharacterSide GetSide(const RelationData& c1, const RelationData& c2);
CharacterClass GetRelativeAlignment( const RelationSet& r1, const RelationData& c2 );


#endif // __CHARACTER_ALIGNMENT_H__
