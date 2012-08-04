// ----------------------------------------------------------------------- //
//
// MODULE  : AICharacterTypeRestrictions.h
//
// PURPOSE : Class definition for an aggregate used to include restrictions
//           on character types to include or exclude.
//
// CREATED : 09/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_CHAR_TYPE_RESTRICTIONS_H_
#define _AI_CHAR_TYPE_RESTRICTIONS_H_

//
// Includes...
//

#include "iaggregate.h"


//
// Defines...
//

#define ADD_AI_CHAR_TYPE_RESTRICTIONS_AGGREGATE( Group ) \
	PROP_DEFINEGROUP( Restrictions, Group, "Restrictions on the types of Characters allowed to use this NavMesh." ) \
  		ADD_STRINGPROP_FLAG( MaskType, "Include", Group|PF_STATICLIST, "Include or exclude the specified character types.") \
		ADD_STRINGPROP_FLAG( CharacterType1, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType2, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType3, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType4, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType5, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType6, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType7, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType8, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType9, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType10, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType11, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType12, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType13, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType14, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType15, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \
		ADD_STRINGPROP_FLAG( CharacterType16, "None", Group|PF_STATICLIST, "Character type to include or exclude.") \



class CAICharacterTypeRestrictions : public IAggregate
{
	public :

		// Ctors/Dtors/etc

		CAICharacterTypeRestrictions();
		virtual ~CAICharacterTypeRestrictions();

		// Engine calls to the aggregate...

		void			ReadProp(const GenericPropList *pProps);

		// Save/Load

		void			Save(ILTMessage_Write *pMsg);
		void			Load(ILTMessage_Read *pMsg);
		
		// Data access

		uint32			GetCharTypeMask() const { return m_dwCharTypeMask; }

	protected :

		uint32			m_dwCharTypeMask;
};

class CAICharacterTypeRestrictionsPlugin : public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

private:

	static bool					sm_bInitted;
};

//-----------------------------------------------------------------

#endif // _AI_CHAR_TYPE_RESTRICTIONS_H_
